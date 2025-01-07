//
// Created by silverhawks on 06/01/25.
//

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int bits = 0;
char mots = 0;
char message[256]; // Buffer to store the message
int message_index = 0;

void handler(int sig) {
    if (sig == SIGQUIT) {
        printf("Final message: %s\n", message);
        message_index = 0;
        bits = 0;
        mots = 0;
        memset(message, 0, sizeof(message));  // Clear the entire message buffer
        return;
    }
    if (sig == SIGUSR1) {
        mots |= (1 << (7 - bits)); // Set the bit to 1
    } else if (sig == SIGUSR2) {
        mots &= ~(1 << (7 - bits)); // Set the bit to 0
    }
    bits++;
    if (bits == 8) {
        message[message_index++] = mots; // Add character to message
        message[message_index] = '\0'; // Null-terminate the string
        printf("Received character: %c\n", mots);
        printf("Current message: %s\n", message);
        bits = 0;
        mots = 0;
    }
    fflush(stdout);
}

int main() {
      printf("Server PID: %d\n", getpid());
      printf("Waiting for signals...\n");

    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);
    signal(SIGQUIT, handler);


    while(1) {
        pause();
    }
    return 0;
}