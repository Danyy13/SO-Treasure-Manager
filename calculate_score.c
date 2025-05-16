#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include "utils.h"

#define MAX_USERS 100

typedef struct {
    char userName[NAME_SIZE];
    int score;
}UserScore;

void printScores(UserScore userScores[], int userScoresSize) {
    printf("Scores:\n");
    for(int i=0;i<userScoresSize;i++) {
        printf("%s - %d\n", userScores[i].userName, userScores[i].score);
    }
}

void initScores(UserScore userScores[], int userScoresSize) {
    for(int i=0;i<userScoresSize;i++) {
        userScores[i].score = 0;
    }
}

void getScoresFromHunts(UserScore userScores[], int *userScoresSize) {
    // functie care traverseaza toate directoarele de hunt
    // si retine scorul fiecarui user in UserScores[]
    DIR *huntDir = opendir(".");
    if(huntDir == NULL) {
        perror("Directorul curent nu a putut fi deschis\n");
        exit(-1);
    }

    int treasureInfoFile = openat(dirfd(huntDir), TREASURE_FILE_NAME, O_RDONLY, S_IRUSR | S_IWUSR);
    if(treasureInfoFile == -1) {
        perror("Fisierul nu a putut fi deschis\n");
        exit(-40);
    }

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

        // printf("%s %d\n", treasure->userName, treasure->value);

        int userIndex = -1;
        // verifica user-ul
        for(int i=0;i<*userScoresSize;i++) {
            // printf("%s %s\n", userScores[i].userName, treasure->userName);
            if(strcmp(userScores[i].userName, treasure->userName) == 0) {
                userIndex = i;
                break; // am gasit indexul user-ului si vreau sa 
            }
        }

        if(userIndex == -1) { // adauga user-ul la array-ul de scoruri
            strcpy(userScores[*userScoresSize].userName, treasure->userName);
            userScores[*userScoresSize].score += treasure->value;
            (*userScoresSize)++;
        } else { // am gasit user-ul => doar dam update la scor
            userScores[userIndex].score += treasure->value;
        }
    }

    close(treasureInfoFile);

    if(closedir(huntDir) == -1) {
        perror("Directorul nu a putut fi inchis\n");
        exit(-2);
    }

}

int main() {
    // deschide directorul curent
    DIR *currentDir = opendir(".");
    if(currentDir == NULL) {
        perror("Directorul curent nu a putut fi deschis\n");
        exit(-1);
    }

    UserScore userScores[MAX_USERS];
    int userScoresSize = 0;
    initScores(userScores, MAX_USERS);

    // get each existing hunt
    FileInfo *file = NULL;
    while((file = readdir(currentDir)) != NULL) {
        if(file->d_type == DT_DIR && file->d_name[0] != '.') {
            // printf("%s\n", file->d_name);
            chdir(file->d_name);

            getScoresFromHunts(userScores, &userScoresSize);

            chdir("..");
        }
    }

    printScores(userScores, userScoresSize);

    if(closedir(currentDir) == -1) {
        perror("Directorul nu a putut fi inchis\n");
        exit(-2);
    }

    return 0;
}