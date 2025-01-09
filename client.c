/**
 * @file client.c
 * @brief Client pour l'envoi de messages avec détection de langue
 * @author silverhawks
 * @date 06/01/25
 *
 * Ce programme client permet d'envoyer des messages à un serveur en utilisant
 * des signaux UNIX. Chaque caractère est converti en binaire et envoyé bit par bit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief Convertit un caractère en sa représentation binaire
 * @param c Le caractère à convertir
 * @param binary Buffer de sortie pour stocker la représentation binaire (doit être de taille 9)
 * 
 * Cette fonction convertit un caractère en sa représentation binaire sur 8 bits.
 * Le résultat est stocké dans le buffer binary sous forme de chaîne de '0' et '1'.
 */
void char_to_binary(char c, char *binary) {
    int ascii_value = (int)c;
    for (int i = 7; i >= 0; i--) {
        binary[7 - i] = (ascii_value & (1 << i)) ? '1' : '0';
    }
    binary[8] = '\0';
}

// Variable globale pour l'accusé de réception
volatile sig_atomic_t ack_received = 0;

// Handler pour recevoir l'accusé de réception
void ack_handler(int signo) {
    if (signo == SIGUSR1) {
        ack_received = 1;
    }
}

/**
 * @brief Point d'entrée du programme
 * @param argc Nombre d'arguments
 * @param argv Tableau des arguments
 * @return 0 en cas de succès
 * 
 * Usage: ./client PID MESSAGE
 * - PID: ID du processus serveur
 * - MESSAGE: Message à envoyer
 * 
 * Le programme envoie chaque caractère du message bit par bit au serveur
 * en utilisant SIGUSR1 pour 1 et SIGUSR2 pour 0.
 * Un signal SIGQUIT est envoyé à la fin du message.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s PID MESSAGE\n", argv[0]);
        return 1;
    }

    // Configuration du handler pour l'accusé de réception
    struct sigaction sa;
    sa.sa_handler = ack_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    int pid = atoi(argv[1]);
    char *message = argv[2];
    int timeout_count;

    printf("Envoi du message au serveur (PID: %d)\n", pid);

    for (int i = 0; i < strlen(message); i++) {
        char binary[9];
        char_to_binary(message[i], binary);
        
        for (int j = 0; j < 8; j++) {
            ack_received = 0;
            timeout_count = 0;
            
            // Envoi du bit
            if (binary[j] == '1') {
                kill(pid, SIGUSR1);
            } else {
                kill(pid, SIGUSR2);
            }

            // Attente de l'accusé de réception avec timeout
            while (!ack_received && timeout_count < 1000) {
                usleep(100);
                timeout_count++;
            }

            if (!ack_received) {
                printf("Erreur: Pas de réponse du serveur\n");
                return 1;
            }

            // Petit délai entre chaque bit pour stabilité
            usleep(100);
        }
    }

    printf("Message envoyé, envoi du signal de fin...\n");
    usleep(1000);  // Attendre un peu avant d'envoyer le signal de fin
    kill(pid, SIGQUIT);
    printf("Terminé.\n");
    
    return 0;
}
