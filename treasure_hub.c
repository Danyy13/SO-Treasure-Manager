#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include "utils.h"

#define COMMANDS_FILENAME "commands.txt"

#define MANAGER_CALL "./treasure_manager"
#define CALCULATE_SCORE_CALL "./calc"
#define LIST_TAG "--list"
#define VIEW_TAG "--view"
#define LIST_HUNTS_TAG "--list_hunts"

typedef enum {
    START_MONITOR,
    LIST_HUNTS,
    LIST_TREASURES,
    VIEW_TREASURE,
    STOP_MONITOR,
    EXIT,
    CALCULATE_SCORE
}CommandCode;

int childPid = 0;
int monitorRunning = 0;
int monitorStopping = 0; // daca monitorul e in starea cand se inchide si trebuie sa asteptam dupa el
FILE *commandsFile = NULL;

int checkMonitorNotRunning() {
    if(monitorRunning == 0) {
        printf("Monitor is not running\n");
    }

    return !monitorRunning;
}

void cleanBeforeExit() {
    if(fclose(commandsFile) != 0) {
        perror("Eroare la inchidere fisier\n");
        exit(-1);
    }
}

void hubExit() {
    if(monitorRunning == 1) {
        printf("Error: Stop the monitor first or wait for monitor to stop\n");
        return;
    }

    cleanBeforeExit();

    exit(0);
}

void sigchldHandler() {
    // cand primim semnalul SIGCHLD atunci procesul chiar s-a oprit
    monitorStopping = 0;
    // printf("Monitor ended\n");
}

void assignSignals() {
    struct sigaction processActions;
    memset(&processActions, 0x00, sizeof(struct sigaction));
    
    processActions.sa_handler = hubExit;
    if(sigaction(SIGINT, &processActions, NULL) < 0) {
        perror("Process set SIGUSR1\n");
        exit(-1);
    }

    processActions.sa_handler = sigchldHandler;
    if(sigaction(SIGCHLD, &processActions, NULL) < 0) {
        perror("Process set SIGCHLD\n");
        exit(-1);
    }
}

void stopMonitor() {
    monitorRunning = 0;
    monitorStopping = 1;
    kill(childPid, SIGUSR2);
}

void startMonitor() {
    if((childPid = fork()) < 0) {
        printf("Eroare la deschidere proces\n");
        exit(-1);
    }

    if(childPid == 0) { // proces copil
        // pornesc procesul de monitor
        char *args[] = {"./monitor", COMMANDS_FILENAME, NULL}; // dam numele fisierului unde vom scrie comenzile ca argument
        if(execv(args[0], args) == -1) {
            perror("Eroare deschiere proces monitor\n");
            exit(-1);
        }
    }

    monitorRunning = 1;
}

int encodeCommand(char *commandName) {
    if(strcmp(commandName, "start_monitor") == 0) return START_MONITOR;
    if(strcmp(commandName, "list_hunts") == 0) return LIST_HUNTS;
    if(strcmp(commandName, "list_treasures") == 0) return LIST_TREASURES;
    if(strcmp(commandName, "view_treasure") == 0) return VIEW_TREASURE;
    if(strcmp(commandName, "stop_monitor") == 0) return STOP_MONITOR;
    if(strcmp(commandName, "exit") == 0) return EXIT;
    if(strcmp(commandName, "calculate_score") == 0) return CALCULATE_SCORE;
    return -1;
}

#define MAX_HUNT_NAME_SIZE 5

void executeCommand(int commandCode) {
    char huntId[MAX_HUNT_NAME_SIZE];
    char stringToSend[100]; // string-ul ce se trimite la monitor prin scrierea in fisier care contine codul operatiei si argumentele
    int treasureId = 0;

    if(monitorStopping == 1) {
        printf("Monitor is stopping, please wait\n");
        return;
    }

    switch(commandCode) {
        case START_MONITOR:
            if(monitorRunning == 0) {
                startMonitor();
                monitorRunning = 1;
            }
            break;
        case LIST_HUNTS:
            if(checkMonitorNotRunning() == 1) break;

            sprintf(stringToSend, "%s %s", MANAGER_CALL, LIST_HUNTS_TAG);

            fprintf(commandsFile, "%s\n", stringToSend);
            fflush(commandsFile);

            kill(childPid, SIGUSR1);
            break;
        case LIST_TREASURES:
            if(checkMonitorNotRunning() == 1) break;

            printf("Give a hunt name\n");
            scanf("%s", huntId);

            sprintf(stringToSend, "%s %s %s", MANAGER_CALL, LIST_TAG, huntId);

            fprintf(commandsFile, "%s\n", stringToSend);
            fflush(commandsFile);

            kill(childPid, SIGUSR1);
            break;
        case VIEW_TREASURE:
            if(checkMonitorNotRunning() == 1) break;

            printf("Give a hunt name\n");
            scanf("%s", huntId);
            
            printf("Give a treasure ID\n");
            scanf("%d", &treasureId);

            sprintf(stringToSend, "%s %s %s %d", MANAGER_CALL, VIEW_TAG, huntId, treasureId);

            // fseek(commandsFile, 0, SEEK_SET);
            fprintf(commandsFile, "%s\n", stringToSend);
            fflush(commandsFile);

            kill(childPid, SIGUSR1);
            break;
        case STOP_MONITOR:
            if(checkMonitorNotRunning() == 1) break;

            stopMonitor();
            break;
        case EXIT:
            hubExit();
            break;
        case CALCULATE_SCORE:
            if(checkMonitorNotRunning() == 1) break;

            sprintf(stringToSend, CALCULATE_SCORE_CALL);

            fprintf(commandsFile, "%s\n", stringToSend);
            fflush(commandsFile);

            kill(childPid, SIGUSR1);
            break;
        default:
            printf("Comanda este gresita\n");
            break;
    }
}

int main() {
    char commandName[MAX_COMMAND_SIZE];
    int commandCode = -1;

    assignSignals();

    commandsFile = fopen(COMMANDS_FILENAME, "w");
    if(commandsFile == NULL) {
        perror("Eroare la deschidere fisier\n");
        exit(-1);
    }

    while(scanf("%s", commandName)) {
        // printf("%s\n", commandName);
        commandCode = encodeCommand(commandName);

        // printf("command code: %d\n", commandCode);

        executeCommand(commandCode);
    }

    return 0;
}