#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "utils.h"

#define MONITOR_STOP_DELAY_SECONDS 7

char commandName[MAX_COMMAND_SIZE];
FILE *commandsFile = NULL;

void cleanExit() {
    if(fclose(commandsFile) != 0) {
        perror("Eroare la inchidere fisier\n");
        exit(-1);
    }

    exit(0);
}

void handlerFunction() {
    if(fgets(commandName, MAX_COMMAND_SIZE, commandsFile) == NULL) {
        perror("Eroare la citire in monitor\n");
        exit(-1);
    }
    // printf("Codul comenzii: %s\n", commandName);

    if(system(commandName) < 0) {
        perror("Eroare la apelare treasure manager\n");
        exit(-1);
    }
}

void stopMonitorHandler() {
    printf("Monitor ending in %d seconds...\n", MONITOR_STOP_DELAY_SECONDS);

    sleep(MONITOR_STOP_DELAY_SECONDS);

    cleanExit();
}

void initSignalHandler() {
    struct sigaction processActions;
    memset(&processActions, 0x00, sizeof(struct sigaction));
    
    processActions.sa_handler = handlerFunction;
    if(sigaction(SIGUSR1, &processActions, NULL) < 0) {
        perror("Process set SIGUSR1\n");
        exit(-1);
    }

    processActions.sa_handler = stopMonitorHandler;
    if(sigaction(SIGUSR2, &processActions, NULL) < 0) {
        perror("Process set SIGUSR2\n");
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        perror("Numar gresit de argumente\n");
        exit(-1);
    }

    initSignalHandler();

    commandsFile = fopen(argv[1], "r");
    if(commandsFile == NULL) {
        perror("Eroare la deschidere fisier\n");
        exit(-1);
    }

    fseek(commandsFile, 0, SEEK_END);

    while(1) {
        pause();
        // loop infinit in care astept comanda de la hub
    }

    return 0;
}