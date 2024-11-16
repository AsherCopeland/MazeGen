#ifndef MAZE_H
#define MAZE_H

#define MAZE_WIDTH 30
#define MAZE_HEIGHT 20


typedef struct {
    _Bool vert[MAZE_HEIGHT][MAZE_WIDTH + 1];
    _Bool horiz[MAZE_HEIGHT + 1][MAZE_WIDTH];
} Maze;


#define MAZE_INIT { .vert[0][0] = 0 }

void maze_print(const Maze *maze);
void maze_randomize(Maze *maze);

#endif
