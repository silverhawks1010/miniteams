//
// Created by silverhawks on 06/01/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void char_to_binary(char c, char *binary) {
    int ascii_value = (int)c;
    for (int i = 7; i >= 0; i--) {
        binary[7 - i] = (ascii_value & (1 << i)) ? '1' : '0';
    }
    binary[8] = '\0';
}

int main(int argc, char *argv[]) {
	int pid = atoi(argv[1]);
	char *message = argv[2];

    for (int i = 0; i < strlen(message); i++) {
        char binary[9];
        char_to_binary(message[i], binary);
        printf("Sending message to %d\n", pid);
        for (int j = 0; j < 8; j++) {
            if (binary[j] == '1') {
                kill(pid, SIGUSR1);
            } else {
                kill(pid, SIGUSR2);
            }
            usleep(1000);
        }
    }
    printf("Sending final message to %d\n", pid);
    kill(pid, SIGQUIT);
}

