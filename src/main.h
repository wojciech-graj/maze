#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#define VISITED(val) (val & BITMASKS[V])

const char MAZE_STATS_PRINT_FORMAT[] = "\
Feature   |Count|Percent\n\
Total     |%5ld|100.000\n\
Dead-Ends |%5ld|%7.3f\n\
Corridors |%5ld|%7.3f\n\
Junctions |%5ld|%7.3f\n\
Crossroads|%5ld|%7.3f\n";

#endif
