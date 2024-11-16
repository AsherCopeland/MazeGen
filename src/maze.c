#include "maze.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CELL_WIDTH 2
#define CELL_HEIGHT 1

enum direction {
    RIGHT = 1,
    UP    = 2,
    LEFT  = 4,
    DOWN  = 8
};

/* const char *walls[] = { */
/*     [0] = " ", */

/*     [                   RIGHT] = "\u2576", */
/*     [              UP        ] = "\u2575", */
/*     [              UP | RIGHT] = "\u2514", */
/*     [       LEFT             ] = "\u2574", */
/*     [       LEFT      | RIGHT] = "\u2500", */
/*     [       LEFT | UP        ] = "\u2518", */
/*     [       LEFT | UP | RIGHT] = "\u2534", */
/*     [DOWN                    ] = "\u2577", */
/*     [DOWN             | RIGHT] = "\u250c", */
/*     [DOWN        | UP        ] = "\u2502", */
/*     [DOWN        | UP | RIGHT] = "\u251c", */
/*     [DOWN | LEFT             ] = "\u2510", */
/*     [DOWN | LEFT      | RIGHT] = "\u252c", */
/*     [DOWN | LEFT | UP        ] = "\u2524", */
/*     [DOWN | LEFT | UP | RIGHT] = "\u253c" */
/* }; */

const char *walls[] = {
    [0] = " ",

    [                   RIGHT] = "\u2576",
    [              UP        ] = "\u2575",
    [              UP | RIGHT] = "\u2570",
    [       LEFT             ] = "\u2574",
    [       LEFT      | RIGHT] = "\u2500",
    [       LEFT | UP        ] = "\u256f",
    [       LEFT | UP | RIGHT] = "\u2534",
    [DOWN                    ] = "\u2577",
    [DOWN             | RIGHT] = "\u256d",
    [DOWN        | UP        ] = "\u2502",
    [DOWN        | UP | RIGHT] = "\u251c",
    [DOWN | LEFT             ] = "\u256e",
    [DOWN | LEFT      | RIGHT] = "\u252c",
    [DOWN | LEFT | UP        ] = "\u2524",
    [DOWN | LEFT | UP | RIGHT] = "\u253c"
};

/* const char *walls[] = { */
/*     [0] = " ", */

/*     [                   RIGHT] = "-", */
/*     [              UP        ] = "|", */
/*     [              UP | RIGHT] = "+", */
/*     [       LEFT             ] = "-", */
/*     [       LEFT      | RIGHT] = "-", */
/*     [       LEFT | UP        ] = "+", */
/*     [       LEFT | UP | RIGHT] = "+", */
/*     [DOWN                    ] = "|", */
/*     [DOWN             | RIGHT] = "+", */
/*     [DOWN        | UP        ] = "|", */
/*     [DOWN        | UP | RIGHT] = "+", */
/*     [DOWN | LEFT             ] = "+", */
/*     [DOWN | LEFT      | RIGHT] = "+", */
/*     [DOWN | LEFT | UP        ] = "+", */
/*     [DOWN | LEFT | UP | RIGHT] = "+" */
/* }; */

// Helpful macros only exist later in the file
static char rp_cache_chars[] = {
    [0] = ' ',
    [1] = '^',
    [3] = '<',
    [5] = 'v',
    [7] = '>',

    [2] = 'V',
    [4] = '.',
    [6] = '*',
    [8] = 'B'
};

static const char *intersection(const Maze *maze,
                                size_t col, size_t row) {
    int direction = 0;
    if (col > 0 && maze->horiz[row][col - 1]) {
        direction |= LEFT;
    }

    if (row > 0 && maze->vert[row - 1][col]) {
        direction |= UP;
    }

    if (col < MAZE_WIDTH && maze->horiz[row][col]) {
        direction |= RIGHT;
    }

    if (row < MAZE_HEIGHT && maze->vert[row][col]) {
        direction |= DOWN;
    }

    return walls[direction];
}

typedef unsigned char RPCache[MAZE_HEIGHT][MAZE_WIDTH];

// Printing with randpath cache is used for debugging purposes
static void print_row(RPCache *rp_cache,
                      const Maze *maze, size_t row) {
#if CELL_HEIGHT > 0
    for (size_t i = 0; i < CELL_HEIGHT; ++i) {
        for (size_t col = 0; col < MAZE_WIDTH; ++col) {
            printf("%s", walls[maze->vert[row][col] ? DOWN | UP : 0]);

#if CELL_WIDTH > 0
            unsigned char cache_entry = 0;
            if (rp_cache) cache_entry = (*rp_cache)[row][col];

            size_t spaces_left = CELL_WIDTH - 1;
            char cache_char = rp_cache_chars[cache_entry];

            if (cache_char) {
                printf("%c", cache_char);
            } else {
                if (CELL_WIDTH > 1) {
                    printf("%.2X", cache_entry);
                    --spaces_left;
                } else {
                    printf("?");
                }
            }

            for (size_t j = 0; j < spaces_left; ++j) {
                printf("%s", walls[0]);
            }
#endif
        }

        printf("%s\n",
               walls[maze->vert[row][MAZE_WIDTH] ? DOWN | UP : 0]);
    }
#else
    (void)maze,(void)row;
#endif
}

static void maze_print_rp(RPCache *rp_cache, const Maze *maze) {
    for (size_t i = 0; i < MAZE_WIDTH; ++i) {
        printf("%s", intersection(maze, i, 0));

#if CELL_WIDTH > 0
        for (size_t j = 0; j < CELL_WIDTH; ++j) {
            printf("%s", walls[maze->horiz[0][i] ? LEFT | RIGHT : 0]);
        }
#endif
    }

    printf("%s\n", intersection(maze, MAZE_WIDTH, 0));


    for (size_t row = 0; row < MAZE_HEIGHT; ++row) {
        print_row(rp_cache, maze, row);

        for (size_t col = 0; col < MAZE_WIDTH; ++col) {
            printf("%s", intersection(maze, col, row + 1));

#if CELL_WIDTH > 0
            bool is_present = maze->horiz[row + 1][col];
            for (size_t i = 0; i < CELL_WIDTH; ++i) {
                printf("%s", walls[is_present ? LEFT | RIGHT : 0]);
            }
#endif
        }

        printf("%s\n", intersection(maze, MAZE_WIDTH, row + 1));
    }
}

void maze_print(const Maze *maze) {
    maze_print_rp(NULL, maze);
}

struct vec {
    size_t x;
    size_t y;
};


static struct vec unitvec[] = {
    { 0, (size_t)-1 },  // Up
    { (size_t)-1, 0 },  // Left
    { 0, 1 },           // Down
    { 1, 0 }            // Right
};

#define DIR_NEGATE(dir) ((dir) ^ 2)

static inline bool randpath_is_valid(const Maze *maze,
                                     struct vec x0,
                                     bool horiz /* of movement */,
                                     bool dir /* true for + */) {
    if (horiz) {
        if ((!dir && x0.x == 0) || (dir && x0.x == MAZE_WIDTH - 1)) {
            return false;
        }

        return maze->vert[x0.y][x0.x + dir];
    }

    if ((!dir && x0.y == 0) || (dir && x0.y == MAZE_HEIGHT - 1)) {
        return false;
    }

    return maze->horiz[x0.y + dir][x0.x];
}

static inline size_t randpath_select(size_t *open,
                                     size_t *num_open) {
    if (*num_open > 0) {
        size_t idx = (size_t)rand() % *num_open;
        size_t dir = open[idx];

        memmove(&open[idx], &open[idx + 1],
                sizeof *open * (*num_open - idx - 1));

        --*num_open;

        return dir;
    }

    return (size_t)-1;
}


/* states:
 * unset = 0,
 * visited = 2,
 * path(dir) = 1 + (dir << 1),
 * nopath = 4,
 * target = 6
 * bfs_visited = 8
 */

#define RP_VISITEDP(entry) ((entry) == 2)
#define RP_VISITED 2

#define RP_PATHP(entry) ((_Bool)((entry) & 1))
#define RP_PATH(dir) ((unsigned char)(((dir) << 1) + 1))
#define RP_DIR(entry) ((entry) >> 1)

#define RP_NOPATHP(entry) ((entry) == 4)
#define RP_NOPATH 4

#define RP_TARGETP(entry) ((entry) == 6)
#define RP_TARGET 6

#define RP_BFS_VISITEDP(entry) ((entry) == 8)
#define RP_BFS_VISITED 8

#define RP_UNSETP(entry) ((entry) == 0)
/* Can be assumed to equal 0 */
#define RP_UNSET 0


static void randpath_cache_update(RPCache *cache, struct vec start) {
    if (!RP_UNSETP((*cache)[start.y][start.x])) return;
    (*cache)[start.y][start.x] = RP_BFS_VISITED;

    struct vec visited[MAZE_WIDTH * MAZE_HEIGHT] = { [0] = start };
    size_t visited_count = 1;
    size_t next_visit = 0;
    struct vec found;
    bool has_found = false;

    while (!has_found && next_visit < visited_count) {
        struct vec pos = visited[next_visit++];

        for (size_t dir = 0; dir < 4; ++dir) {
            struct vec dx = unitvec[dir];
            if ((dx.x == (size_t)-1 && pos.x == 0)
                || pos.x + dx.x >= MAZE_WIDTH
                || (dx.y == (size_t)-1 && pos.y == 0)
                || pos.y + dx.y >= MAZE_HEIGHT) continue;

            struct vec adj = { .x = pos.x + dx.x,
                               .y = pos.y + dx.y  };

            unsigned char *adj_entry = &(*cache)[adj.y][adj.x];

            if (RP_NOPATHP(*adj_entry)
                || RP_PATHP(*adj_entry)
                || RP_TARGETP(*adj_entry)) {
                found = adj;
                has_found = true;
                break;
            }

            if (!RP_UNSETP(*adj_entry)) continue;

            *adj_entry = RP_BFS_VISITED;
            visited[visited_count++] = adj;
        }
    }

    unsigned char found_entry = (*cache)[found.y][found.x];
    bool path = has_found
        && (RP_PATHP(found_entry) || RP_TARGETP(found_entry));

    visited[0] = has_found ? found : start;
    visited_count = 1;
    next_visit = 0;

    if (!has_found) (*cache)[start.y][start.x] = RP_NOPATH;

    while (next_visit < visited_count) {
        struct vec pos = visited[next_visit++];

        for (size_t dir = 0; dir < 4; ++dir) {
            struct vec dx = unitvec[dir];
            if ((dx.x == (size_t)-1 && pos.x == 0)
                || pos.x + dx.x >= MAZE_WIDTH
                || (dx.y == (size_t)-1 && pos.y == 0)
                || pos.y + dx.y >= MAZE_HEIGHT) continue;

            struct vec adj = { .x = pos.x + dx.x,
                               .y = pos.y + dx.y  };

            unsigned char *adj_entry = &(*cache)[adj.y][adj.x];

            if (!RP_BFS_VISITEDP(*adj_entry)) continue;

            *adj_entry = path ? RP_PATH(DIR_NEGATE(dir)) : RP_NOPATH;
            visited[visited_count++] = adj;
        }
    }
}

static void randpath_cache_invalidate_path(RPCache *cache,
                                           struct vec pos) {
    unsigned char *start_entry = &(*cache)[pos.y][pos.x];
    if (!RP_PATHP(*start_entry)) return;
    *start_entry = RP_UNSET;

    for (size_t i = 0; i < 4; ++i) {
        struct vec dx = unitvec[i];
        if ((dx.x == (size_t)-1 && pos.x == 0)
            || pos.x + dx.x >= MAZE_WIDTH
            || (dx.y == (size_t)-1 && pos.y == 0)
            || pos.y + dx.y >= MAZE_HEIGHT) continue;

        struct vec adj_pos = { .x = pos.x + dx.x,
                               .y = pos.y + dx.y  };

        unsigned char adj_entry = (*cache)[adj_pos.y][adj_pos.x];

        if (!RP_PATHP(adj_entry) ||
            RP_DIR(adj_entry) != DIR_NEGATE(i)) continue;

        randpath_cache_invalidate_path(cache, adj_pos);
    }
}

static void randpath_cache_invalidate_nopath(RPCache *cache,
                                             struct vec pos) {
    unsigned char *start_entry = &(*cache)[pos.y][pos.x];
    if (!RP_NOPATHP(*start_entry) && !RP_UNSETP(*start_entry)) return;
    *start_entry = RP_UNSET;

    for (size_t i = 0; i < 4; ++i) {
        struct vec dx = unitvec[i];
        if ((dx.x == (size_t)-1 && pos.x == 0)
            || pos.x + dx.x >= MAZE_WIDTH
            || (dx.y == (size_t)-1 && pos.y == 0)
            || pos.y + dx.y >= MAZE_HEIGHT) continue;

        struct vec adj_pos = { .x = pos.x + dx.x,
                               .y = pos.y + dx.y  };

        unsigned char adj_entry = (*cache)[adj_pos.y][adj_pos.x];

        if (!RP_NOPATHP(adj_entry)) continue;

        randpath_cache_invalidate_nopath(cache, adj_pos);
    }
}


static bool maze_randpath(RPCache *cache, Maze *maze, struct vec x0) {
    if (RP_TARGETP((*cache)[x0.y][x0.x])) return true;
    if (RP_VISITEDP((*cache)[x0.y][x0.x])) return false;
    randpath_cache_invalidate_path(cache, x0);
    (*cache)[x0.y][x0.x] = RP_VISITED;

    size_t opendirs[4];
    size_t num_open = 0;

    for (size_t i = 0; i < 4; ++i) {
        if (!randpath_is_valid(maze, x0, i & 1, (i >> 1) & 1)) {
            continue;
        }

        opendirs[num_open++] = i;
    }

    size_t diridx;
    while ((diridx = randpath_select(opendirs, &num_open))
           != (size_t)-1) {
        struct vec dirvec = unitvec[diridx];
        struct vec next = { .x = x0.x + dirvec.x,
                            .y = x0.y + dirvec.y  };

        randpath_cache_update(cache, next);
        if (RP_NOPATHP((*cache)[next.y][next.x])) {
            continue;
        }

        bool horiz = diridx & 1;
        bool dir = (diridx >> 1) & 1;

        bool *entry;
        bool *forward;
        bool *walla;
        bool *wallb;

        if (horiz) {
            entry = &maze->vert[x0.y][x0.x + dir];
            forward = &maze->vert[x0.y][next.x + dir];
            walla = &maze->horiz[x0.y][next.x];
            wallb = &maze->horiz[x0.y + 1][next.x];
        } else {
            entry = &maze->horiz[x0.y + dir][x0.x];
            forward = &maze->horiz[next.y + dir][x0.x];
            walla = &maze->vert[next.y][x0.x];
            wallb = &maze->vert[next.y][x0.x + 1];
        }

        bool entry0 = *entry;
        bool forward0 = *forward;
        bool walla0 = *walla;
        bool wallb0 = *wallb;

        *entry = false;
        *forward = true;
        *walla = true;
        *wallb = true;

        if (maze_randpath(cache, maze, next)) {
            (*cache)[x0.y][x0.x] = RP_TARGET;
            return true;
        }

        *entry = entry0;
        *forward = forward0;
        *walla = walla0;
        *wallb = wallb0;
    }

    (*cache)[x0.y][x0.x] = RP_UNSET;
    randpath_cache_invalidate_nopath(cache, x0);    maze_print_rp(cache, maze);

    return false;
}

void maze_randomize(Maze *maze) {
    for (size_t i = 0; i < MAZE_HEIGHT; ++i) {
        for (size_t j = 0; j < MAZE_WIDTH + 1; ++j) {
            maze->vert[i][j] = j == 0 || j == MAZE_WIDTH;
        }
    }

    for (size_t i = 0; i < MAZE_HEIGHT + 1; ++i) {
        for (size_t j = 0; j < MAZE_WIDTH; ++j) {
            maze->horiz[i][j] = i == 0 || i == MAZE_HEIGHT;
        }
    }

    maze->vert[0][0] = false;
    maze->vert[0][1] = true;
    maze->horiz[1][0] = true;

    RPCache cache = {
        [MAZE_HEIGHT - 1][MAZE_WIDTH - 1] = RP_TARGET
    };

    maze_randpath(&cache, maze, (struct vec){ 0, 0 });

    maze->vert[MAZE_HEIGHT - 1][MAZE_WIDTH] = false;
}
