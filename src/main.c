#include "main.h"

typedef unsigned char byte;

typedef struct Maze {
    size_t width, height;
    byte *in, *out;
    byte cells[];
} Maze;

/*
    Cell bits:
    1 - N - North
    2 - E - East
    3 - S - South
    4 - W - West

    Additional bitmasks:
    V - Visited (checks if any direction bits are set)
*/

enum cell_bitmasks{N, E, S, W, V};
const byte BITMASKS[5] = {[N]=0x01, [E]=0x02, [S]=0x04, [W]=0x08, [V]=0x0F};
const size_t INV_DIRECTIONS[4] = {[N]=S, [E]=W, [S]=N, [W]=E};
const wchar_t SYMBOLS[] = {' ', 0x2579, 0x257a, 0x2517, 0x257b, 0x2503, 0x250f, 0x2523, 0x2578, 0x251b, 0x2501, 0x253b, 0x2513, 0x252b, 0x2533, 0x254b};

unsigned rand_int(unsigned max)
{
    return rand() % max;
}

byte *maze_get_cell(Maze *maze, size_t x, size_t y)
{
    return &maze->cells[y * maze->width + x];
}

Maze *maze_new(size_t width, size_t height)
{
    Maze *maze = calloc(1, sizeof(Maze) + (width * height));
    maze->width = width;
    maze->height = height;
    return maze;
}

void maze_delete(Maze *maze)
{
    free(maze);
}

size_t maze_get_x(Maze *maze, byte *cell)
{
    return (cell - maze->cells) % maze->width;
}

size_t maze_maze_get_y(Maze *maze, byte *cell)
{
    return (cell - maze->cells) / maze->width;
}

void maze_cell_remove_edge(Maze *maze, byte *cell)
{
    size_t in_x = maze_get_x(maze, cell);
    if(in_x == 0)
        *cell |= BITMASKS[W];
    else if(in_x == maze->width - 1)
        *cell |= BITMASKS[E];
    else {
        size_t in_y = maze_maze_get_y(maze, cell);
        if(in_y == 0)
            *cell |= BITMASKS[N];
        else
            *cell |= BITMASKS[S];
    }
}

byte *maze_get_cell_in_direction(Maze *maze, byte *cell, size_t direction)
{
    switch(direction) {
        case N:
        return maze_maze_get_y(maze, cell) ? cell - maze->width : NULL;
        case E:
        return maze_get_x(maze, cell) != maze->width - 1 ? cell + 1 : NULL;
        case S:
        return maze_maze_get_y(maze, cell) != maze->height - 1 ? cell + maze->width : NULL;
        case W:
        return maze_get_x(maze, cell) ? cell - 1 : NULL;
    }
    return NULL;
}

//generate maze using Randomized Depth-First Search
void maze_generate_rdfs(Maze *maze)
{
     maze_cell_remove_edge(maze, maze->in);
     byte *stack[maze->width * maze->height];
     ptrdiff_t stack_loc = 0;
     stack[0] = maze->in;
     while(stack_loc >= 0) {
         size_t direction, num_neighbors = 0, neighbor_directions[3];
         byte *neighbors[3];
         for(direction = 0; direction < 4; direction++) {
             byte *cell = maze_get_cell_in_direction(maze, stack[stack_loc], direction);
             if(cell)
                if(! (*cell && BITMASKS[V])) {
                    neighbor_directions[num_neighbors] = direction;
                    neighbors[num_neighbors++] = cell;
                }
         }
         if(num_neighbors) {
             size_t index = rand_int(num_neighbors);
             *stack[stack_loc] |= BITMASKS[neighbor_directions[index]];
             *neighbors[index] |= BITMASKS[INV_DIRECTIONS[neighbor_directions[index]]];
             stack[++stack_loc] = neighbors[index];
         } else
            stack_loc--;
     }
     maze_cell_remove_edge(maze, maze->out);
}

void maze_print(Maze *maze)
{
    size_t x, y, i = 0;
    for(y = 0; y < maze->height; y++) {
        for(x = 0; x < maze->width; x++)
            printf("%lc", SYMBOLS[maze->cells[i++]]);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    srand(time(NULL));
    setlocale(LC_ALL, "");

    Maze *maze = maze_new(10, 10);
    maze->in = maze_get_cell(maze, 0, 1);
    maze->out = maze_get_cell(maze, 0, 8);
    maze_generate_rdfs(maze);
    maze_print(maze);

    return 0;
}
