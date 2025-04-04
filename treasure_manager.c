#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define NAME_SIZE 30
#define CLUE_SIZE 100

#define DIRECTORY_EXISTS_ERROR 1
#define DIRECTORY_CLOSE_ERROR 2
#define DIRECTORY_OPEN_ERROR 3
#define FILE_OPEN_ERROR 4
#define FILE_CLOSE_ERROR 5

typedef struct dirent FileInfo;

typedef struct {
    double x;
    double y;
}Coordinates;

typedef struct {
    uint8_t id;
    char userName[NAME_SIZE + 1];
    Coordinates coordinates;
    char clueText[CLUE_SIZE + 1];
    int value;
}Treasure;

uint8_t add(char *huntId) {
    // verifica daca exista deja hunt-ul folosind functia de biblioteca
    DIR *currentDir = opendir(".");
    if(currentDir == NULL) {
        fprintf(stderr, "Directorul nu a putut fi deschis\n");
        exit(DIRECTORY_OPEN_ERROR);
    }

    FileInfo *file = NULL;
    while((file = readdir(currentDir)) != NULL) {
        // printf("%s\n", file->d_name);

        // deschide directorul
        DIR *dir = opendir(huntId);
        if(dir == NULL) {
            fprintf(stderr, "Directorul nu a putut fi deschis\n");
            exit(DIRECTORY_OPEN_ERROR);
        }

        int treasureInfoFile = open("treasureInfo.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR, S_IWUSR, S_IXUSR);
        if(treasureInfoFile == -1) {
            fprintf(stderr, "Fisierul nu a putut fi deschis\n");
            exit(FILE_OPEN_ERROR);
        }

        if(close(treasureInfoFile) == -1) {
            fprintf(stderr, "Fisierul nu a putut fi inchis\n");
            exit(FILE_CLOSE_ERROR);
        }

        if(closedir(dir) == -1) {
            fprintf(stderr, "Directorul nu a putut fi inchis\n");
            exit(DIRECTORY_CLOSE_ERROR);
        }
    }

    if(mkdir(huntId, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        return 1;
    }

    return 0;

    // creaza fisierul in care se retine info despre treasures

    // scrie informatiile despre treasure
}

int main(void) {
    // printf("Hello\n");

    if(add("000") == DIRECTORY_EXISTS_ERROR) printf("Directorul exista deja\n");

    return 0;
}
