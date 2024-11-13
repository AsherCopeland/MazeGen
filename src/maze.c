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

const char *walls[] = {
    [0] = " ",

    [                   RIGHT] = "\u2576",
    [              UP        ] = "\u2575",
    [              UP | RIGHT] = "\u2514",
    [       LEFT             ] = "\u2574",
    [       LEFT      | RIGHT] = "\u2500",
    [       LEFT | UP        ] = "\u2518",
    [       LEFT | UP | RIGHT] = "\u2534",
    [DOWN                    ] = "\u2577",
    [DOWN             | RIGHT] = "\u250c",
    [DOWN        | UP        ] = "\u2502",
    [DOWN        | UP | RIGHT] = "\u251c",
    [DOWN | LEFT             ] = "\u2510",
    [DOWN | LEFT      | RIGHT] = "\u252c",
    [DOWN | LEFT | UP        ] = "\u2524",
    [DOWN | LEFT | UP | RIGHT] = "\u253c"
};

/* const char *walls[] = { */
/*     [0] = " ", */

/*     [                   RIGHT] = "\u2576", */
/*     [              UP        ] = "\u2575", */
/*     [              UP | RIGHT] = "\u2570", */
/*     [       LEFT             ] = "\u2574", */
/*     [       LEFT      | RIGHT] = "\u2500", */
/*     [       LEFT | UP        ] = "\u256f", */
/*     [       LEFT | UP | RIGHT] = "\u2534", */
/*     [DOWN                    ] = "\u2577", */
/*     [DOWN             | RIGHT] = "\u256d", */
/*     [DOWN        | UP        ] = "\u2502", */
/*     [DOWN        | UP | RIGHT] = "\u251c", */
/*     [DOWN | LEFT             ] = "\u256e", */
/*     [DOWN | LEFT      | RIGHT] = "\u252c", */
/*     [DOWN | LEFT | UP        ] = "\u2524", */
/*     [DOWN | LEFT | UP | RIGHT] = "\u253c" */
/* }; */

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

static void print_row(const Maze *maze, size_t row) {
#if CELL_HEIGHT > 0
    for (size_t i = 0; i < CELL_HEIGHT; ++i) {
        for (size_t col = 0; col < MAZE_WIDTH; ++col) {
            printf("%s", walls[maze->vert[row][col] ? DOWN | UP : 0]);

#if CELL_WIDTH > 0
            for (size_t j = 0; j < CELL_WIDTH; ++j) {
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

void maze_print(const Maze *maze) {
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
        print_row(maze, row);

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

struct vec {
    size_t x;
    size_t y;
};


static struct vec unitvec[] = {
    { 0, (size_t)-1 },
    { (size_t)-1, 0 },
    { 0, 1 },
    { 1, 0 }
};

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

static bool maze_randpath(bool (*visited)[MAZE_HEIGHT][MAZE_WIDTH],
                          Maze *maze, struct vec x0, struct vec xf) {
    if (x0.x == xf.x && x0.y == xf.y) return true;
    if ((*visited)[x0.y][x0.x]) return false;
    (*visited)[x0.y][x0.x] = true;

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

        if (maze_randpath(visited, maze, next, xf)) {
            (*visited)[x0.y][x0.x] = false;
            return true;
        }

        *entry = entry0;
        *forward = forward0;
        *walla = walla0;
        *wallb = wallb0;
    }

    (*visited)[x0.y][x0.x] = false;
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

    bool visited[MAZE_HEIGHT][MAZE_WIDTH] = { [0][0] = false };
    maze_randpath(&visited, maze,
                  (struct vec){ 0, 0 },
                  (struct vec){ MAZE_WIDTH - 1, MAZE_HEIGHT - 1 });

    maze->vert[MAZE_HEIGHT - 1][MAZE_WIDTH] = false;
}
