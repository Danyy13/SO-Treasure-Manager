#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#define TREASURE_FILE_NAME "treasureInfo.txt"

#define NAME_SIZE 31
#define CLUE_SIZE 127

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

int main() {
    DIR *currentDir = opendir(".");
    if(currentDir == NULL) {
        perror("Directorul curent nu a putut fi deschis\n");
        exit(-1);
    }

    int treasureInfoFile = openat(dirfd(currentDir), TREASURE_FILE_NAME, O_RDONLY, S_IRUSR | S_IWUSR);
    if(treasureInfoFile == -1) {
        perror("Fisierul nu a putut fi deschis\n");
        exit(-40);
    }

    int scoreSum = 0;

    Treasure *treasure = (Treasure *)malloc(sizeof(Treasure));
    if(!treasure) {
        perror("Eroare la alocare memorie treasure\n");
        exit(-20);
    }
    ssize_t readValue = 0;
    while((readValue = read(treasureInfoFile, treasure, sizeof(Treasure))) != 0) {
        if(readValue == -1) {
            perror("Eroare la citire din fisier\n");
            exit(-30);
        }

        scoreSum += treasure->value;
    }

    printf("Suma este %d\n", scoreSum);

    close(treasureInfoFile);

    if(closedir(currentDir) == -1) {
        perror("Directorul nu a putut fi inchis\n");
        exit(-2);
    }
    return 0;
}