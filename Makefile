SRC_FILES := main.c
LINUX_CFLAGS := -lm -std=c11
RELEASE_CFLAGS := -O3
DEBUG_CFLAGS := -Wall -Wextra -Wdouble-promotion -Wpedantic -Wstrict-prototypes -Wshadow -g -Og -fsanitize=address -fsanitize=undefined

SRC_PATH = src/
FILENAME = maze

debug:
	gcc $(addprefix $(SRC_PATH), $(SRC_FILES)) -o maze $(LINUX_CFLAGS) $(DEBUG_CFLAGS)
release:
	gcc $(addprefix $(SRC_PATH), $(SRC_FILES)) -o maze $(LINUX_CFLAGS) $(RELEASE_CFLAGS)
