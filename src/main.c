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

size_t maze_get_cell_index(Maze *maze, byte *cell)
{
    return cell - maze->cells;
}

void maze_remove_edge_wall(Maze *maze, byte *cell)
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

void maze_remove_wall(byte *src, byte *dest, size_t direction)
{
    *src |= BITMASKS[direction];
    *dest |= BITMASKS[INV_DIRECTIONS[direction]];
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
     maze_remove_edge_wall(maze, maze->in);
     byte *stack[maze->width * maze->height];
     ptrdiff_t stack_loc = 0;
     stack[0] = maze->in;
     while(stack_loc >= 0) {
         size_t direction,
            num_neighbors = 0,
            neighbor_directions[3];
         byte *neighbors[3];
         for(direction = 0; direction < 4; direction++) {
             byte *cell = maze_get_cell_in_direction(maze, stack[stack_loc], direction);
             if(cell)
                if(! VISITED(*cell)) {
                    neighbor_directions[num_neighbors] = direction;
                    neighbors[num_neighbors++] = cell;
                }
         }
         if(num_neighbors) {
             size_t index = rand_int(num_neighbors);
             maze_remove_wall(stack[stack_loc], neighbors[index], neighbor_directions[index]);
             stack[++stack_loc] = neighbors[index];
         } else
            stack_loc--;
     }
     maze_remove_edge_wall(maze, maze->out);
}

//generate maze using Ranomized Prim's algorithm
//has been modified to not select random walls but random walls from random visited cells
void maze_generate_rprim(Maze *maze)
{
    maze_remove_edge_wall(maze, maze->in);
    size_t list_size = 1;
    byte *list[maze->width * maze->height];
    list[0] = maze->in;
    while(list_size > 0) {
        size_t list_loc = rand_int(list_size),
            num_neighbors = 0,
            neighbor_directions[3],
            direction;
        byte *neighbors[3];
        for(direction = 0; direction < 4; direction++) {
            byte *cell = maze_get_cell_in_direction(maze, list[list_loc], direction);
            if(cell)
               if(! VISITED(*cell)) {
                   neighbor_directions[num_neighbors] = direction;
                   neighbors[num_neighbors++] = cell;
               }
        }
        if(num_neighbors) {
            size_t index = rand_int(num_neighbors);
            maze_remove_wall(list[list_loc], neighbors[index], neighbor_directions[index]);
            list[list_size++] = neighbors[index];
        } else
           memmove(&list[list_loc], &list[list_loc + 1], sizeof(byte*) * (list_size-- - list_loc));
    }
    maze_remove_edge_wall(maze, maze->out);
}

//generate maze using Aldous-Broder algorithm
void maze_generate_aldous_broder(Maze *maze)
{
    maze_remove_edge_wall(maze, maze->in);
    size_t remaining = maze->width * maze->height - 1;
    byte *cell = maze->in;
    while(remaining > 0) {
        size_t direction;
        byte *next_cell;
        do {
            direction = rand_int(4);
            next_cell = maze_get_cell_in_direction(maze, cell, direction);
        } while(! next_cell);
        if(! VISITED(*next_cell)) {
            maze_remove_wall(cell, next_cell, direction);
            remaining--;
        }
        cell = next_cell;
    }
    maze_remove_edge_wall(maze, maze->out);
}

//generate maze using Wilson's algorithm
//has been modified for the first visited cell to not be random, but to be the in and out cells
void maze_generate_wilson(Maze *maze)
{
    maze_remove_edge_wall(maze, maze->in);
    maze_remove_edge_wall(maze, maze->out);
    size_t maze_size = maze->width * maze->height,
        num_unvisited_cells = 0,
        i;
    byte *unvisited_cells[maze_size - 2],
        walk_directions[maze_size];
    for(i = 0; i < maze_size; i++)
        if(! VISITED(maze->cells[i]))
            unvisited_cells[num_unvisited_cells++] = &maze->cells[i];
    while(num_unvisited_cells > 0) {
        byte *start_cell = unvisited_cells[rand_int(num_unvisited_cells)],
            *cell = start_cell;
        do {
            size_t direction;
            byte *next_cell;
            do {
                direction = rand_int(4);
                next_cell = maze_get_cell_in_direction(maze, cell, direction);
            } while(! next_cell);
            walk_directions[maze_get_cell_index(maze, cell)] = direction;
            cell = next_cell;
        } while(! VISITED(*cell));
        byte *end_cell = cell;
        cell = start_cell;
        while(cell != end_cell) {
            for(i = 0; i < num_unvisited_cells; i++)
                if(unvisited_cells[i] == cell) {
                    memmove(&unvisited_cells[i], &unvisited_cells[i + 1], sizeof(byte*) * (num_unvisited_cells-- - i));
                    break;
                }
            size_t direction = walk_directions[maze_get_cell_index(maze, cell)];
            byte *next_cell = maze_get_cell_in_direction(maze, cell, direction);
            maze_remove_wall(cell, next_cell, direction);
            cell = next_cell;
        }
    }
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
    size_t len = strlen(filename);
    assert(len > 4);
    assert(! strcmp(".pbm", filename + len - 4));

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

    //TODO: rewrite/optimize this
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

    setlocale(LC_ALL, "");
    srand(time(NULL));

    Maze *maze = maze_new(30, 30);
    maze->in = maze_get_cell(maze, 0, 1);
    maze->out = maze_get_cell(maze, 29, 28);
    maze_generate_wilson(maze);

    maze_save_image(maze, "img.pbm");
    maze_print(maze);

    return 0;
}
