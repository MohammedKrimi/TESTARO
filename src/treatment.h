#ifndef TREATMENT_H
#define TREATMENT_H value

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXLIGNE 1000

int isEmpty(char* s, int nb_char);
int isSeperated(char* s, int nb_char);
char* traitement_ligne(char* ligne, int num_ligne, char* entree, char* sortie);
void traitement_commentaire();
void traitement_commande_shell();
void copie(char* ligne, char* entree_sortie);
char* realisation_test(char* ligne, int num_ligne, char*entree, char*sortie);
void traitement(FILE* file);
void handler(int sig);
int execution(char* ligne);
#endif