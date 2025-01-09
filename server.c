/**
 * @file server.c
 * @brief Serveur de réception de messages avec détection de langue
 * @author silverhawks
 * @date 06/01/25
 *
 * Ce programme serveur reçoit des messages envoyés bit par bit via des signaux UNIX
 * et détermine la langue probable du message reçu en analysant la fréquence des lettres.
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Alphabet
char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/** @brief Tableau des langues supportées */
char *languages[] = {"Français", "Anglais", "Allemand", "Espagnol"};

// def du fichier Log  
#define LOG_FILE "server_log.txt"  



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

/**
 * @brief Structure pour stocker les mots caractéristiques de chaque langue
 */
struct LanguageKeywords {
    const char* lang;
    const char* keywords[10];
};

/**
 * @brief Mots caractéristiques pour chaque langue
 */
struct LanguageKeywords keywords[] = {
    {"Français", {"le", "la", "les", "un", "une", "des", "est", "et", "en", "dans"}},
    {"Anglais", {"the", "is", "are", "and", "to", "of", "in", "for", "with", "on"}},
    {"Allemand", {"der", "die", "das", "und", "ist", "in", "den", "von", "zu", "für"}},
    {"Espagnol", {"el", "la", "los", "las", "un", "una", "es", "en", "de", "por"}}
};

/**
 * @brief Vérifie si un mot est présent dans le message
 */
int contains_word(const char* message, const char* word) {
    char* msg_lower = strdup(message);
    char* word_lower = strdup(word);
    
    // Convertir en minuscules
    for(int i = 0; msg_lower[i]; i++) {
        msg_lower[i] = tolower(msg_lower[i]);
    }
    for(int i = 0; word_lower[i]; i++) {
        word_lower[i] = tolower(word_lower[i]);
    }
    
    int result = strstr(msg_lower, word_lower) != NULL;
    free(msg_lower);
    free(word_lower);
    return result;
}

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
    double min_diff = 999999;
    int index = 0;
    int len = 0;
    int letter_count[26] = {0};
    double scores[4] = {0}; // Scores pour chaque langue

    // Compter les lettres
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

    // 1. Calcul basé sur la fréquence des lettres (50% du score final)
    double observed_freq[26];
    for(int i = 0; i < 26; i++) {
        observed_freq[i] = (double)letter_count[i] / len * 100.0;
    }

    for (int i = 0; i < 4; i++) {
        double diff_sum = 0;
        for (int j = 0; j < 26; j++) {
            double diff = observed_freq[j] - probabilities[i][j];
            diff_sum += diff * diff;
        }
        scores[i] = -diff_sum; // Score négatif car plus la différence est petite, meilleur est le score
    }

    // 2. Recherche de mots caractéristiques (50% du score final)
    for(int i = 0; i < 4; i++) {
        int word_matches = 0;
        for(int j = 0; j < 10; j++) {
            if(contains_word(message, keywords[i].keywords[j])) {
                word_matches++;
            }
        }
        scores[i] += word_matches * 50.0; // Bonus pour chaque mot trouvé
    }

    // Trouver la langue avec le meilleur score
    double max_score = scores[0];
    int best_index = 0;
    //printf("\nScores par langue:\n");
    for(int i = 0; i < 4; i++) {
        //printf("%s: %.2f\n", languages[i], scores[i]);
        if(scores[i] > max_score) {
            max_score = scores[i];
            best_index = i;
        }
    }

    return languages[best_index];
}

/** @brief Buffer pour stocker le message reçu */
char message[1024];
/** @brief Index courant dans le buffer de message */
volatile int message_length = 0;
/** @brief Compteur de bits reçus */
volatile int bits = 0;
/** @brief Caractère en cours de construction */
volatile unsigned char mots = 0;
/** @brief PID du client pour l'accusé de réception */
volatile pid_t client_pid = -1;

/**
 * @brief Gestionnaire de signaux pour la réception des messages
 * @param sig Signal reçu (SIGUSR1, SIGUSR2 ou SIGQUIT)
 * @param info Informations supplémentaires sur le signal
 * @param context Contexte de l'appel
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



FILE *log_file;


void load_previous_messages() {
    log_file = fopen(LOG_FILE, "a+");  // Ouvre le fichier en lecture et éc>    
    if (!log_file) {
        perror("Erreur lors de l'ouverture du fichier log");
        exit(EXIT_FAILURE);
    }

    // Lire les messages et les afficher
    char line[256];
    printf("Messages précédents :\n");
    while (fgets(line, sizeof(line), log_file)) {
        printf("%s", line);
    }
    printf("\n");
}

void save_message(const char *msg) {
    if (log_file) {
        fprintf(log_file, "%s\n", msg);
        fflush(log_file);
    }
}

void handler(int sig, siginfo_t *info, void *context) {
    if (sig == SIGUSR1 || sig == SIGUSR2) {
        // Obtenir le PID du client depuis siginfo
        client_pid = info->si_pid;
        
        // Traitement du bit reçu
        if (sig == SIGUSR1) {
            mots = (mots << 1) | 1;
        } else {
            mots = (mots << 1) | 0;
        }
        bits++;

        // Envoyer l'accusé de réception avec un petit délai
        if (client_pid > 0) {
            usleep(100);  // Petit délai avant l'envoi de l'ACK
            kill(client_pid, SIGUSR1);
        }

        if (bits == 8) {
            if (message_length < sizeof(message) - 1) {
                message[message_length++] = mots;
            }
            bits = 0;
            mots = 0;
        }
    } else if (sig == SIGQUIT) {
        if (message_length > 0) {
            message[message_length] = '\0';
            printf("\nMessage reçu du client PID %d : %s\n", client_pid, message);
            printf("Langue détectée : %s\n", getlangue(message));
            if (log_file) {
                    fprintf(log_file, "Client PID: %d, Message complet reçu : %s\n", client_pid, message);
                    fflush(log_file);
            }            
            memset(message, 0, sizeof(message));
            message_length = 0;
            bits = 0;
            mots = 0;
            client_pid = -1;
        }
    }
}

/**
 * @brief Point d'entrée du programme
 * @return 0 en cas de succès
 *
 * Le programme affiche son PID et attend les signaux
 * pour recevoir des messages.
 */
int main() {
    // Configuration des gestionnaires de signaux avec sigaction
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;  // Pour obtenir les informations supplémentaires sur le signal
    sigemptyset(&sa.sa_mask);
    load_previous_messages();
    if (sigaction(SIGUSR1, &sa, NULL) == -1 ||
        sigaction(SIGUSR2, &sa, NULL) == -1 ||
        sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Server PID: %d\n", getpid());
    
    while(1) {
        pause();
    }
    fclose(log_file); l
    return 0;
}
