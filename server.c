/**
 * @file server.c
 * @brief Serveur de réception de messages avec détection de langue
 * @author silverhawks
 * @date 06/01/25
 *
 * Ce programme serveur reçoit des messages envoyés bit par bit via des signaux UNIX
 * et détermine la langue probable du message reçu en analysant la fréquence des lettres.
 */

<<<<<<< HEAD
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Alphabet
char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/** @brief Tableau des langues supportées */
char *languages[] = {"Français", "Anglais", "Allemand", "Espagnol"};

/**
 * @brief Tableau des fréquences d'apparition des lettres pour chaque langue
 * Source : https://fr.wikipedia.org/wiki/Fr%C3%A9quence_d%27apparition_des_lettres
 */
double probabilities[4][26] = {
    // Français
    {7.64, 0.90, 3.26, 3.67, 14.72, 1.06, 0.87, 0.74, 7.53, 0.61, 0.05, 5.45, 2.96, 7.09, 5.28, 3.02, 1.29, 6.69, 7.95, 7.24, 6.31, 1.83, 0.04, 0.42, 0.19, 0.21},
    // Anglais
    {8.17, 1.49, 2.78, 4.25, 12.70, 2.23, 2.02, 6.09, 6.97, 0.15, 0.77, 4.03, 2.41, 6.75, 7.51, 1.93, 0.10, 5.99, 6.33, 9.06, 2.76, 0.98, 2.36, 0.15, 1.97, 0.07},
    // Allemand
    {6.51, 1.89, 2.73, 5.08, 16.40, 1.66, 3.01, 4.57, 7.55, 0.27, 1.42, 3.44, 2.53, 9.78, 2.51, 0.79, 0.02, 7.00, 7.27, 6.15, 4.35, 0.67, 1.89, 0.03, 0.04, 1.13},
    // Espagnol
    {12.53, 1.42, 4.68, 5.86, 13.68, 0.69, 1.01, 0.70, 6.25, 0.44, 0.01, 4.97, 3.15, 6.71, 8.68, 2.51, 0.88, 6.87, 7.98, 4.63, 3.93, 0.90, 0.01, 0.22, 0.90, 0.52}
};

/** @brief Compteur de bits reçus */
int bits = 0;
/** @brief Caractère en cours de construction */
char mots = 0;
/** @brief Buffer pour stocker le message reçu */
char message[256]; // Buffer to store the message
/** @brief Index courant dans le buffer de message */
int message_index = 0;

/**
 * @brief Détermine la langue probable d'un message
 * @param message Le message à analyser
 * @return Un pointeur vers la chaîne contenant le nom de la langue
 *
 * Cette fonction analyse la fréquence des lettres dans le message
 * et la compare aux fréquences connues de différentes langues
 * pour déterminer la langue la plus probable.
 */
char* getlangue(char *message) {
    double min_diff = 999999;  // On cherche la plus petite différence
    int index = 0;
    int len = 0;
    int letter_count[26] = {0};

    for(int i = 0; message[i] != '\0'; i++) {
        char c = message[i];
        if(c >= 'a' && c <= 'z') {
            letter_count[c - 'a']++;
            len++;
        } else if(c >= 'A' && c <= 'Z') {
            letter_count[c - 'A']++;
            len++;
        }
    }

    if(len == 0) return languages[0];


    double observed_freq[26];
    for(int i = 0; i < 26; i++) {
        observed_freq[i] = (double)letter_count[i] / len * 100.0;
    }

    //printf("Scores de différence (plus petit = meilleur):\n");
    for (int i = 0; i < 4; i++) {
        double diff_sum = 0;
        for (int j = 0; j < 26; j++) {
            double diff = observed_freq[j] - probabilities[i][j];
            diff_sum += diff * diff;
        }
        //printf("%s: %f\n", languages[i], diff_sum);
        
        if (diff_sum < min_diff) {
            min_diff = diff_sum;
            index = i;
        }
    }

    return languages[index];
}

/**
 * @brief Gestionnaire de signaux pour la réception des messages
 * @param sig Signal reçu (SIGUSR1, SIGUSR2 ou SIGQUIT)
 *
 * Cette fonction traite les signaux reçus :
 * - SIGUSR1 : bit 1
 * - SIGUSR2 : bit 0
 * - SIGQUIT : fin du message
 *
 * Les bits reçus sont assemblés en caractères, qui sont
 * ensuite ajoutés au message. À la réception de SIGQUIT,
 * le message complet est affiché et sa langue est déterminée.
 */
void handler(int sig) {
    if (sig == SIGQUIT) {
        printf("Final message: %s\n", message);
        printf("Langue: %s\n", getlangue(message));
        message_index = 0;
        bits = 0;
        mots = 0;
        memset(message, 0, sizeof(message)); 
        return;
    }
    if (sig == SIGUSR1) {
        mots |= (1 << (7 - bits));
    } else if (sig == SIGUSR2) {
        mots &= ~(1 << (7 - bits));
    }
    bits++;
    if (bits == 8) {
        message[message_index++] = mots; 
        message[message_index] = '\0';
        bits = 0;
        mots = 0;
    }
    fflush(stdout);
}

/**
 * @brief Point d'entrée du programme
 * @return 0 en cas de succès
 *
 * Le programme affiche son PID et attend les signaux
 * pour recevoir des messages.
 */
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
=======
#include <stdio.h>
#include <unistd.h>

void main() {
    pid_t pid = getpid();
    printf("PID: %d\n", pid);

}



>>>>>>> 1b2e4673abf496fca0b0849480315e7808c30cfb
