#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define NAME_SIZE 31
#define CLUE_SIZE 127

#define DIRECTORY_EXISTS_ERROR 1
#define DIRECTORY_CLOSE_ERROR 2
#define DIRECTORY_OPEN_ERROR 3
#define FILE_OPEN_ERROR 4
#define FILE_CLOSE_ERROR 5
#define FILE_WRITE_ERROR 6
#define MKDIR_ERROR 7

typedef struct dirent FileInfo;

typedef struct {
    double x;
    double y;
}Coordinates;

typedef struct {
    uint8_t id;
    int value;
    Coordinates coordinates;
    char userName[NAME_SIZE + 1];
    char clueText[CLUE_SIZE + 1];
}Treasure;

void getNewTreasureFromStandardInput(Treasure *treasure) {
    printf("ID: ");
    scanf("%hhd", &(treasure->id));

    getchar(); // !! citeste \n-ul dat la citirea anterioara ca sa nu se poata stoca in string-ul citit dupa
    printf("User Name: ");
    char userNameBuffer[NAME_SIZE + 1];
    fgets(userNameBuffer, sizeof(userNameBuffer), stdin);
    if(userNameBuffer[strlen(userNameBuffer) - 1] == '\n') userNameBuffer[strlen(userNameBuffer) - 1] = '\0';
    strcpy(treasure->userName, userNameBuffer);

    printf("X Coordinate: ");
    scanf("%lg", &(treasure->coordinates.x));

    printf("Y Coordinate: ");
    scanf("%lg", &(treasure->coordinates.y));

    getchar(); // !! citeste \n-ul dat la citirea anterioara ca sa nu se poata stoca in string-ul citit dupa
    printf("Clue Text: ");
    char clueTextBuffer[CLUE_SIZE + 1];
    fgets(clueTextBuffer, sizeof(clueTextBuffer), stdin);
    if(clueTextBuffer[strlen(clueTextBuffer) - 1] == '\n') clueTextBuffer[strlen(clueTextBuffer) - 1] = '\0';
    strcpy(treasure->clueText, clueTextBuffer);

    printf("Value: ");
    scanf("%d", &(treasure->value));
}

void printTreasure(Treasure treasure) {
    printf("%hhd, ", treasure.id);
    fputs(treasure.userName, stdout);
    printf(", %lg %lg, ", treasure.coordinates.x, treasure.coordinates.y);
    fputs(treasure.clueText, stdout);
    printf(", %d\n", treasure.value);
}

uint8_t add(char *huntId, Treasure treasure) {
    // verifica daca exista deja hunt-ul folosind functia de biblioteca
    DIR *currentDir = opendir(".");
    if(currentDir == NULL) {
        fprintf(stderr, "Directorul curent nu a putut fi deschis\n");
        exit(DIRECTORY_OPEN_ERROR);
    }

    // parcurge toate fisierele din directorul curent si vezi daca exista id-ul hunt-ului
    FileInfo *file = NULL;
    uint8_t directoryExists = 0;
    while((file = readdir(currentDir)) != NULL) {
        // printf("%s\n", file->d_name);
        if(strcmp(file->d_name, huntId) == 0) { // daca fisierul are numele hunt-ului ne oprim
            directoryExists = 1;
            break;
        }
    }

    // daca directorul nu exista il cream
    if(!directoryExists) {
        if(mkdir(huntId, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            return MKDIR_ERROR;
        }
    }

    // daca exista il deschidem
    DIR *huntDir = opendir(huntId);
    if(huntDir == NULL) {
        fprintf(stderr, "Directorul nu a putut fi deschis\n");
        exit(DIRECTORY_OPEN_ERROR);
    }

    // deschidem/cream fisierul in directorul hunt-ului
    int treasureInfoFile = openat(dirfd(huntDir), "treasureInfo.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if(treasureInfoFile == -1) {
        fprintf(stderr, "Fisierul nu a putut fi deschis\n");
        exit(FILE_OPEN_ERROR);
    }

    // scriem datele in fisier
    if(write(treasureInfoFile, &treasure, sizeof(Treasure)) == -1) return FILE_WRITE_ERROR;

    // inchidem fisierul
    if(close(treasureInfoFile) == -1) {
        fprintf(stderr, "Fisierul nu a putut fi inchis\n");
        exit(FILE_CLOSE_ERROR);
    }

    // inchidem directorul
    if(closedir(huntDir) == -1) {
        fprintf(stderr, "Directorul nu a putut fi inchis\n");
        exit(DIRECTORY_CLOSE_ERROR);
    }

    return 0;
}

int main(void) {
    Treasure treasure = {0};
    getNewTreasureFromStandardInput(&treasure);

    printTreasure(treasure);

    if(add("000", treasure) != 0) printf("Eroare la adaugare\n");

    return 0;
}
