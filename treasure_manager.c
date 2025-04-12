#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define TREASURE_FILE_NAME "treasureInfo.txt"

#define NAME_SIZE 31
#define CLUE_SIZE 127

#define DIRECTORY_EXISTS_ERROR 1
#define DIRECTORY_CLOSE_ERROR 2
#define DIRECTORY_OPEN_ERROR 3
#define FILE_OPEN_ERROR 4
#define FILE_CLOSE_ERROR 5
#define FILE_WRITE_ERROR 6
#define MKDIR_ERROR 7
#define ARGUMENT_COUNT_ERROR 8
#define NO_HUNT_NAME_ERROR 9
#define TREASURE_FILE_EXISTS_ERROR 10
#define TREASURE_FILE_ACCESS_ERROR 11
#define MALLOC_ERROR 12
#define FILE_READ_ERROR 13
#define WRONG_ARGUMENT_NUMBER_ERROR 14
#define DIRECTORY_CHANGE_ERROR 15
#define TREASURE_OFFSET_ERROR 16

typedef struct dirent FileInfo;
typedef struct stat Stat;

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

void printTime(struct timespec *time) {
    struct tm *tmInfo = localtime(&time->tv_sec);
    char buffer[100];

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tmInfo);
    printf("[%s.%09ld]\n", buffer, time->tv_nsec);
}

void getNewTreasureFromStandardInput(Treasure *treasure) {
    printf("ID: ");
    scanf("%hd", &(treasure->id));

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
    printf("%hd, ", treasure.id);
    fputs(treasure.userName, stdout);
    printf(", %lg %lg, ", treasure.coordinates.x, treasure.coordinates.y);
    fputs(treasure.clueText, stdout);
    printf(", %d\n", treasure.value);
}

FileInfo *getFileByName(DIR *currentDir, char *fileName) {
    // un director e defapt un fisier deci se pot gasi si directoare
    // parcurge toate fisierele din directorul curent si vezi daca exista id-ul hunt-ului
    FileInfo *file = NULL;
    while((file = readdir(currentDir)) != NULL) {
        // printf("%s\n", file->d_name);
        if(strcmp(file->d_name, fileName) == 0) { // daca fisierul are numele hunt-ului ne oprim
            return file;
        }
    }
    return NULL;
}

DIR *openDirectory(char *directoryName) {
    DIR *directory = opendir(directoryName);
    if(directory == NULL) {
        perror("Directorul curent nu a putut fi deschis\n");
        exit(DIRECTORY_OPEN_ERROR);
    }
    return directory;
}

void closeDirectory(DIR *directory) {
    if(closedir(directory) == -1) {
        perror("Directorul nu a putut fi inchis\n");
        exit(DIRECTORY_CLOSE_ERROR);
    }
}

uint8_t add(char *huntId, Treasure treasure) {
    // verifica daca exista deja hunt-ul folosind functia de biblioteca
    DIR *currentDir = openDirectory(".");
    
    FileInfo *hunt = getFileByName(currentDir, huntId);

    // daca directorul nu exista il cream
    if(!hunt) {
        if(mkdir(huntId, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            return MKDIR_ERROR;
        }
    }

    // daca exista il deschidem
    DIR *huntDir = openDirectory(huntId);

    // deschidem/cream fisierul in directorul hunt-ului
    int treasureInfoFile = openat(dirfd(huntDir), "treasureInfo.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if(treasureInfoFile == -1) {
        perror("Fisierul nu a putut fi deschis\n");
        exit(FILE_OPEN_ERROR);
    }

    // scriem datele in fisier
    if(write(treasureInfoFile, &treasure, sizeof(Treasure)) == -1) return FILE_WRITE_ERROR;

    // inchidem fisierul
    if(close(treasureInfoFile) == -1) {
        perror("Fisierul nu a putut fi inchis\n");
        exit(FILE_CLOSE_ERROR);
    }

    // inchidem directorul
    closeDirectory(huntDir);

    closeDirectory(currentDir);

    return 0;
}

uint8_t list(char *huntId) {
    DIR *currentDir = openDirectory(".");

    FileInfo *hunt = getFileByName(currentDir, huntId);
    if(!hunt) {
        printf("Directorul nu exista\n");
        return DIRECTORY_EXISTS_ERROR;
    }

    DIR *huntDir = openDirectory(huntId);

    // print hunt name
    printf("Hunt: ");
    printf("%s\n", huntId);

    // print total file size
    FileInfo *treasureFileInfo = getFileByName(huntDir, TREASURE_FILE_NAME);
    if(!treasureFileInfo) {
        perror("Fisierul cu treasures nu exista\n");
        return TREASURE_FILE_EXISTS_ERROR;
    }

    // retine datele fisierului intr-un Stat = struct stat folosind functia stat
    chdir(huntId);
    Stat *treasureFileStat = (Stat *)malloc(sizeof(Stat));
    if(!treasureFileStat) {
        perror("Eroare la alocare memorie statFile\n");
        exit(MALLOC_ERROR);
    }
    if(stat(TREASURE_FILE_NAME, treasureFileStat) == -1) {
        perror("Eroare la accesare fisier treasures\n");
        return TREASURE_FILE_EXISTS_ERROR;
    }
    chdir("..");

    // printeaza numele, marimea si data ultimei modificari
    printf("%s\t%ld\t", TREASURE_FILE_NAME, treasureFileStat->st_size);
    printTime(&(treasureFileStat->st_mtim));

    // for treasure in the file, print the treasure
    // open the file, write the binary values to a treasure, print treasure, close file
    int treasureInfoFile = openat(dirfd(huntDir), "treasureInfo.txt", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if(treasureInfoFile == -1) {
        perror("Fisierul nu a putut fi deschis\n");
        exit(FILE_OPEN_ERROR);
    }

    // citim datele din fisier in Treasure
    Treasure *treasure = (Treasure *)malloc(sizeof(Treasure));
    if(!treasure) {
        perror("Eroare la alocare memorie treasure\n");
        exit(MALLOC_ERROR);
    }
    ssize_t readValue = 0;
    while((readValue = read(treasureInfoFile, treasure, sizeof(Treasure))) != 0) {
        if(readValue == -1) {
            perror("Eroare la citire din fisier\n");
            exit(FILE_READ_ERROR);
        }
        printTreasure(*treasure);
    }

    free(treasure);

    // inchidem fisierul
    if(close(treasureInfoFile) == -1) {
        perror("Fisierul nu a putut fi inchis\n");
        exit(FILE_CLOSE_ERROR);
    }

    closeDirectory(huntDir);

    closeDirectory(currentDir);

    free(treasureFileStat);

    return 0;
}

uint8_t view(char *huntId, int treasureId) {
    DIR *currentDir = openDirectory(".");

    FileInfo *hunt = getFileByName(currentDir, huntId);
    if(!hunt) {
        printf("Directorul nu exista\n");
        return DIRECTORY_EXISTS_ERROR;
    }

    DIR *huntDir = openDirectory(huntId);

    int treasureInfoFile = openat(dirfd(huntDir), "treasureInfo.txt", O_RDONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if(treasureInfoFile == -1) {
        perror("Fisierul nu a putut fi deschis\n");
        exit(FILE_OPEN_ERROR);
    }

    // citim datele din fisier in Treasure
    int treasureFound = 0;
    Treasure *treasure = (Treasure *)malloc(sizeof(Treasure));
    if(!treasure) {
        perror("Eroare la alocare memorie treasure\n");
        exit(MALLOC_ERROR);
    }
    ssize_t readValue = 0;
    while((readValue = read(treasureInfoFile, treasure, sizeof(Treasure))) != 0) {
        if(readValue == -1) {
            perror("Eroare la citire din fisier\n");
            exit(FILE_READ_ERROR);
        }
        // printTreasure(*treasure);
        if(treasure->id == treasureId) {
            printTreasure(*treasure);
            treasureFound = 1;
            break; // nu mai exista alt treasure cu acelasi ID
        }
    }

    free(treasure);

    // inchidem fisierul
    if(close(treasureInfoFile) == -1) {
        perror("Fisierul nu a putut fi inchis\n");
        exit(FILE_CLOSE_ERROR);
    }

    closeDirectory(huntDir);

    closeDirectory(currentDir);

    if(treasureFound != 1) {
        printf("Nu s-a gasit treasure-ul\n");
    }

    return 0;
}

int getTreasureOffsetFromFileById(int treasureFileDescriptor, Treasure *treasure, int treasureId) {
    ssize_t readValue = 0;
    int treasuresPassed = 0;
    uint8_t treasureFound = 0;
    while((readValue = read(treasureFileDescriptor, treasure, sizeof(Treasure))) != 0) {
        if(readValue == -1) {
            perror("Eroare la citire din fisier\n");
            exit(FILE_READ_ERROR);
        }

        if(treasure->id == treasureId) {
            treasureFound = 1;
            break; // nu mai exista alt treasure cu acelasi ID
        } else {
            treasuresPassed++;
        }
    }
    if(treasureFound) return treasuresPassed * sizeof(Treasure);
    else return -1;
}

void removeFileFromDirectory(DIR *directory, char *fileName) {
    // sterge fisierul cu numele fileName din directorul dat de calea RELATIVA directory

    if(fchdir(dirfd(directory)) == -1) {
        perror("Nu s-a putut deschide directorul\n");
        exit(DIRECTORY_CHANGE_ERROR);
    }

    if(remove(fileName) != 0) {
        perror("Nu s-a putut sterge fisierul\n");
    }

    chdir("..");
}

unsigned int getFileSize(int fileDescriptor) {
    Stat fileStat;
    if(fstat(fileDescriptor, &fileStat) != 0) {
        // printf("Size: %ld\n", fileStat.st_size);
        perror("Eroare fstat\n");
    }
    return fileStat.st_size;
}

uint8_t removeTreasure(char *huntId, int treasureId) {
    DIR *currentDir = openDirectory(".");

    FileInfo *hunt = getFileByName(currentDir, huntId);
    if(!hunt) {
        printf("Directorul nu exista\n");
        return DIRECTORY_EXISTS_ERROR;
    }

    DIR *huntDir = openDirectory(huntId);

    int treasureInfoFile = openat(dirfd(huntDir), "treasureInfo.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(treasureInfoFile == -1) {
        perror("Fisierul nu a putut fi deschis\n");
        exit(FILE_OPEN_ERROR);
    }

    Treasure *treasure = (Treasure *)malloc(sizeof(Treasure));
    if(!treasure) {
        perror("Eroare la alocare memorie treasure\n");
        exit(MALLOC_ERROR);
    }
    int treasureOffset = getTreasureOffsetFromFileById(treasureInfoFile, treasure, treasureId);
    if(treasureOffset == -1) {
        free(treasure);
        return TREASURE_OFFSET_ERROR;
    }
    free(treasure);

    // copiez treasure-urile de dupa treasure intr-un buffer
    unsigned int fileSize = getFileSize(treasureInfoFile);
    unsigned int bufferSize = fileSize - (treasureOffset + sizeof(Treasure));

    // printf("fileSize: %d\n", fileSize);
    // printf("bufferSize: %d\n", bufferSize);

    unsigned char *buffer = (unsigned char *)malloc(bufferSize);
    if(!buffer) {
        perror("Eroare la alocare memorie buffer\n");
        exit(MALLOC_ERROR);
    }

    // setez offset-ul fisierului sa fie la treasure-ul gasit plus o pozitie pentru a citi datele urmatoarelor treasure-uri
    lseek(treasureInfoFile, treasureOffset + sizeof(Treasure), SEEK_SET);
    // printf("current pos: %ld\n", lseek(treasureInfoFile, 0, SEEK_CUR));

    // citeste din fisier treasure-urile de dupa si apoi scrie-le cu un spatiu mai la stanga
    if(read(treasureInfoFile, buffer, bufferSize) == -1) {
        perror("Eroare la citire din fisier\n");
        exit(FILE_READ_ERROR);
    }

    // mut cursorul inapoi la treasure si il suprascriu
    lseek(treasureInfoFile, treasureOffset, SEEK_SET); // setez offset-ul fisierului sa fie la treasure-ul gasit
    // printf("current pos: %ld\n", lseek(treasureInfoFile, 0, SEEK_CUR));

    if(write(treasureInfoFile, buffer, bufferSize) == -1) {
        perror("Eroare la scriere in fisier\n");
        exit(FILE_WRITE_ERROR);
    }

    ftruncate(treasureInfoFile, fileSize - sizeof(Treasure));

    if(close(treasureInfoFile) == -1) {
        perror("Fisierul nu a putut fi inchis\n");
        exit(FILE_CLOSE_ERROR);
    }

    // printf("%d\n", treasureOffset);

    closeDirectory(huntDir);

    closeDirectory(currentDir);

    return 0;
}

uint8_t removeHunt(char *huntId) {
    DIR *currentDir = openDirectory(".");

    FileInfo *hunt = getFileByName(currentDir, huntId);
    if(!hunt) {
        printf("Directorul nu exista\n");
        return DIRECTORY_EXISTS_ERROR;
    }

    DIR *huntDir = openDirectory(huntId);

    // cauta daca exista fisierul
    int treasureFileFound = 0;
    FileInfo* entry = NULL;
    while ((entry = readdir(huntDir)) != NULL) {
        if(strcmp(entry->d_name, TREASURE_FILE_NAME) == 0) {
            treasureFileFound = 1;
        }
    }

    if(treasureFileFound) removeFileFromDirectory(huntDir, TREASURE_FILE_NAME);

    if(remove(huntId) != 0 ) {
        perror("Nu s-a putut sterge directorul\n");
    }

    closeDirectory(huntDir);

    closeDirectory(currentDir);

    return 0;
}

typedef enum {
    ADD,
    LIST,
    VIEW,
    REMOVE_TREASURE,
    REMOVE_HUNT
}OPERATION;

int8_t encodeOperation(char *arg) {
    if(strcmp(arg, "--add") == 0) return ADD;
    else if(strcmp(arg, "--list") == 0) return LIST;
    else if(strcmp(arg, "--view") == 0) return VIEW;
    else if(strcmp(arg, "--remove_treasure") == 0) return REMOVE_TREASURE;
    else if(strcmp(arg, "--remove_hunt") == 0) return REMOVE_HUNT;
    else return -1;
}

int main(int argc, char *argv[]) {    
    if(argc <= 1) {
        printf("Nu sunt destule argumente\n");
        exit(ARGUMENT_COUNT_ERROR);
    }

    int8_t operation = encodeOperation(argv[1]);

    switch(operation) {
        case ADD:
            if(argc != 3) {
                printf("No hunt name\n");
                exit(NO_HUNT_NAME_ERROR);
            }
            Treasure treasure = {0};
            getNewTreasureFromStandardInput(&treasure);
            printTreasure(treasure);
            if(add(argv[2], treasure) != 0) printf("Eroare la adaugare\n");
            break;
        case LIST:
            if(argc != 3) {
                printf("No hunt name\n");
                exit(NO_HUNT_NAME_ERROR);
            }
            list(argv[2]);
            break;
        case VIEW:
            if(argc != 4) {
                printf("Nu sunt destule argumente\nview <huntId> <treasureId>\n");
                exit(WRONG_ARGUMENT_NUMBER_ERROR);
            }
            if(view(argv[2], atoi(argv[3])) != 0) {
                printf("Eroare la view\n");
            }
            break;
        case REMOVE_TREASURE:
            if(argc != 4) {
                printf("Nu sunt destule argumente\nremove_treasure <huntId> <treasureId>\n");
                exit(WRONG_ARGUMENT_NUMBER_ERROR);
            }
            if(removeTreasure(argv[2], atoi(argv[3])) != 0) printf("Eroare la stergere treasure\n");
            break;
        case REMOVE_HUNT:
            if(argc != 3) {
                printf("Nu sunt destule argumente\nremove_hunt <huntId>\n");
                exit(WRONG_ARGUMENT_NUMBER_ERROR);
            }
            if(removeHunt(argv[2]) != 0) printf("Eroare la stergere hunt\n");
            break;
        default:
            printf("Wrong operation\n");
            break;
    }

    return 0;
}
