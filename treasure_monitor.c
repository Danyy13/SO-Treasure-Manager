#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
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

    int pipeFunctionToMonitor[2] = {0, 0};
    int processId = 0;

    if(pipe(pipeFunctionToMonitor) < 0) {
        perror("Eroare la crearea pipe-ului\n");
	    exit(-1);
    }

    if((processId = fork()) < 0) {
        perror("Eroare la apelare sistem\n");
        exit(-1);
    }

    if(processId == 0) { // fiu
        close(pipeFunctionToMonitor[0]); // inchid capatul de citire

        dup2(pipeFunctionToMonitor[1], STDOUT_FILENO);

        // apelez un shell care sa execute comanda (prin argumentul -c) commandName
        // ca sa pot apela fork() si exec() separat folosind un singur string ca argument
        // altfel ar trebui sa fac probabil strtok() pe string si sa il dau in *args[] pt exec
        execlp("sh", "sh", "-c", commandName, (char *)NULL);

        perror("Eroare execlp sistem\n");
        exit(-1);
    }

    close(pipeFunctionToMonitor[1]); // inchid capatul de scriere

    char string[MAX_STRING_SIZE];
    ssize_t bytesRead = read(pipeFunctionToMonitor[0], string, MAX_STRING_SIZE);
    if(bytesRead < 0) {
        perror("Eroare la citire din pipe in monitor\n");
        exit(-1);
    }
    string[bytesRead] = '\0'; // punem caracterul terminal la finalul string-ului ca sa evitam existenta unei zone de memorie aditionale nedorite
    
    int status = 0;
    if(waitpid(processId, &status, 0) < 0) {
        perror("Eroare la waitpid\n");
        exit(-1);
    }

    close(pipeFunctionToMonitor[0]);

    // printam stringul la stdout dar vom face o redirectare la capatul pipe-ului in crearea procesului monitor din treasure_hub.c
    printf("%s\n", string);
    fflush(stdout);

    // metoda cu system => nu pot face RW in pipe
    // if(system(commandName) < 0) {
    //     perror("Eroare la apelare sistem\n");
    //     exit(-1);
    // }
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