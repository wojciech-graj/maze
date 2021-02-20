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

    bit set - path exists

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

//generate maze using Ranomized Prim's algorithm
//has been modified to not select random walls but random walls from random visited cells
void maze_generate_rprim(Maze *maze)
{
    maze_cell_remove_edge(maze, maze->in);
    size_t list_size = 1;
    byte *list[maze->width * maze->height];
    list[0] = maze->in;
    while(list_size > 0) {
        size_t list_loc = rand_int(list_size), direction, num_neighbors = 0, neighbor_directions[3];
        byte *neighbors[3];
        for(direction = 0; direction < 4; direction++) {
            byte *cell = maze_get_cell_in_direction(maze, list[list_loc], direction);
            if(cell)
               if(! (*cell && BITMASKS[V])) {
                   neighbor_directions[num_neighbors] = direction;
                   neighbors[num_neighbors++] = cell;
               }
        }
        if(num_neighbors) {
            size_t index = rand_int(num_neighbors);
            *list[list_loc] |= BITMASKS[neighbor_directions[index]];
            *neighbors[index] |= BITMASKS[INV_DIRECTIONS[neighbor_directions[index]]];
            list[list_size++] = neighbors[index];
        } else
           memmove(&list[list_loc], &list[list_loc + 1], sizeof(byte*) * (list_size-- - list_loc));
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

void maze_save_image(Maze *maze, char *filename)
{
    FILE *file = fopen(filename, "wb");
    assert(file);

    size_t img_width = 2 * maze->width + 1,
        img_height = 2 * maze->height + 1,
        bytes_per_row = ((img_width + 7) & ~0x07) / 8,
        i, j;
    byte img[img_height][bytes_per_row];

    //set corners and unset rest
    for(i = 0; i < img_height; i ++)
        memset(img[i], (i & 0x01) ? 0x00 : 0xaa, bytes_per_row);

    //TODO: optimize this
    //set cell sides
    for(i = 0; i < maze->height; i++)
        for(j = 0; j < maze->width; j++) {
            byte cell = maze->cells[i * maze->width + j];
            size_t x_l = 2 * j, x_m = 2 * j + 1, x_r = 2 * j + 2;
            img[2 * i][x_m / 8] |= ((cell & BITMASKS[N]) == 0) << (7 - (x_m & 0x07));
            img[2 * i + 1][x_l / 8] |= ((cell & BITMASKS[W]) == 0) << (7 - (x_l & 0x07));
            img[2 * i + 1][x_r / 8] |= ((cell & BITMASKS[E]) == 0) << (7 - (x_r & 0x07));
            img[2 * i + 2][x_m / 8] |= ((cell & BITMASKS[S]) == 0) << (7 - (x_m & 0x07));
        }

    fprintf(file, "P4\n%ld %ld\n", img_width, img_height);
    fwrite(img, 1, bytes_per_row * img_height, file);
    fclose(file);
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    srand(time(NULL));
    setlocale(LC_ALL, "");

    Maze *maze = maze_new(30, 30);
    maze->in = maze_get_cell(maze, 0, 1);
    maze->out = maze_get_cell(maze, 29, 28);
    maze_generate_rprim(maze);

    maze_save_image(maze, "img.pbm");
    maze_print(maze);

    return 0;
}
