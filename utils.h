#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define TREASURE_FILE_NAME "treasureInfo.txt"

#define NAME_SIZE 31
#define CLUE_SIZE 127

#define MAX_COMMAND_SIZE 100

typedef struct dirent FileInfo;

typedef struct {
    double x;
    double y;
}Coordinates;

typedef struct {
    uint16_t id;
    int value;
    Coordinates coordinates;
    char userName[NAME_SIZE + 1];
    char clueText[CLUE_SIZE + 1];
}Treasure;

#endif // UTILS_H
