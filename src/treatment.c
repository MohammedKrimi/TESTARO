#include "treatment.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/** fonction permettant de savoir si la ligne est vide ou pas  **/
int isEmpty(char* s, int nb_char){
	for (int i = 0; i < nb_char; i++)
	{
		if( !(*(s+i)=='\n') && !(*(s+i)=='\t') && !(*(s+i)==' ') && !(*(s+i)=='\0'))
		{
			return 0;
		}
	}
	return 1;
}


/** fonction permettant de savoir si la ligne a besoin d'être concaténée ('\' à la fin) **/
int isSeperated(char* s, int nb_char) {
		if(*(s+nb_char-2)=='\\')
		{	
			
			return 1; 
		}
	
	return 0;
}

/** accumulateur d'entrées et de sorties */
void copie(char* ligne, char* entree_sortie){
	for (int i = 0; i < strlen(ligne)-1; ++i) 		/** -1 pour ne pas copier '\0' de chaque ligne */
	{
		strncat(entree_sortie, (ligne+2)+i, 1);		/** +2 pour ne pas copier "< " ou "> " */
	}
	strcat(entree_sortie, "\0");					/** indiquer la fin de l'entrée | sortie */
}

/** fonction permettant de traiter chaque ligne suivant son premier caractère **/
char* traitement_ligne(char* ligne, int num_ligne, char* entree, char* sortie) {
		//pour les commentaires
		if (*ligne == '#')
		{
			return NULL;
		}

		// pour les commandes shell
		else if (*ligne == '$')
		{
			return realisation_test((ligne+2), num_ligne, entree, sortie);	
		}

		// pour les écrits en entrée
		else if (*ligne == '<') 
		{
			copie(ligne, entree);
			return NULL;
		}

		// pour les affichages
		else if (*ligne == '>')
		{
			copie(ligne, sortie);
			return NULL;
		}

		// pour les lignes 'p'
		else if (*ligne == 'p')
		{
			printf("%s", ligne+2);
			return NULL;
		}

		// ligne inconnue
		else
		{
			printf("ligne %i : Problème à la lecture de la suite (erreur de syntaxe)\n", num_ligne);
			exit(1);
		}
}

/** realisation d'une commande avec l'aide des tubes pour les redirections et de fork pour les processus*/
char* realisation_test(char* ligne, int num_ligne, char*  entree, char* sortie) {

    /** Lecture de la commande*/
    char* ligne_copy = malloc(sizeof(char)*MAXLIGNE);
    strcpy(ligne_copy,ligne);
    char* cwd = malloc(sizeof(char)*MAXLIGNE);
    getcwd(cwd, MAXLIGNE);
    char delimiters[]=" \t\r\n";
    char* token = strtok(ligne_copy, delimiters);

    /** si la commande est cd */
	if (!strcmp(token, "cd")){
		token = strtok(NULL, delimiters); // token correspond au chemin (paramètre du cd)
		chdir(token);
		getcwd(cwd, MAXLIGNE);
		return cwd;	// curent working directory
	}

	/** si la commande n'est pas cd */
	else{
		char* bufferPere = malloc(sizeof(char)*MAXLIGNE);
		int tube1[2],tube2[2];
		
		/** création des tubes*/
		int stat1 = pipe(tube1);
		int stat2 = pipe(tube2);

		/** création d'un processus fils*/
		int fils = fork();

		int status;
		if (stat1 || stat2)
		{
			printf("ligne %i : Problème lors d'un appel système\n", num_ligne);
			exit(4);
		}
		if (fils < 0)
		{
			printf("ligne %i : Problème lors d'un appel système\n", num_ligne);
			exit(4);
		}

		/** Dans le processus père */
		if (fils > 0)
		{	
			/** fermetures */
			close(tube1[0]);
	        close(tube2[1]);
			
	        /** transmettre l'accumulateur d'entrées sur le stdin du processus fils */
			write(tube1[1],entree,MAXLIGNE);
			
			close(tube1[1]);
			while(read(tube2[0],bufferPere, MAXLIGNE)) {

			}
			close(tube2[0]);
			waitpid(fils, &status, 0);

			/** récupération du code de sortie du fils */
			if ( WIFEXITED(status) ) { 
		        int exit_status = WEXITSTATUS(status);
		        if (exit_status)
		        {	
		        	printf("ligne %i : Problème lors d'un appel système\n", num_ligne);
		        	exit(4);
		        }
		    }
		}

		else if (fils == 0)
		{	
			/** fermetures */
			close(tube1[1]);
			close(tube2[0]);
			
			/** redirection de l'entrée standard */
			dup2(tube1[0],0);
			/** redirection de la sortie standard */
			dup2(tube2[1],1);
			
			execution(ligne);	
		}
			
		return bufferPere;
	}
}

/** code de sortie de l'alarme en cas de délai dépassé */
void handler(int sig){
	printf("Une commande n'a pas terminé avant l'expiration du délai de garde\n");
	exit(3);
}

/** exécution d'une commande */
int execution(char* ligne) {
	/** mise en place de l'alarme à 5s pour chaque commande */
	struct sigaction nvt, old;
	memset(&nvt, 0, sizeof(nvt));
	nvt.sa_handler = handler;
	sigaction(SIGALRM, &nvt, &old);
	alarm(5);
	
	/** exécution de la commande et retour de son code de sortie */
	return execlp("/bin/sh","/bin/sh","-c",ligne,NULL);
}

/** traitement du fichier de description ligne par ligne */
void traitement(FILE* file){
	
	/** problème lors de la lecture du fichier */
	if (file==NULL)
	{
		printf("Problème à la lecture de la suite (fichier inexistant)\n");
		exit(1);		
	}
	char* buf = NULL;
	size_t size = 0;
	int nb_char;
	char* entree = malloc(sizeof(char)*MAXLIGNE);
	char* sortie = malloc(sizeof(char)*MAXLIGNE);
	char* sortie_traitement = malloc(sizeof(char)*MAXLIGNE);
	char* sortie_test = malloc(sizeof(char)*MAXLIGNE);
	int num_ligne = 0;
	
	/** tant que le fichier n'est pas lu entièrement */
	/** stocker la ligne dans buf */
	while((nb_char=getline(&buf, &size, file)) != -1){
		/** incrémentation du numéro de la ligne */
		num_ligne++;
		
		/** si la ligne est vide */
		if (isEmpty(buf,nb_char))
		{
			/** passer à la suivante */
			nb_char = getline(&buf, &size, file);
			num_ligne++;
		}

		/** si la ligne est séparée sur plusieurs ligne (se termmine par '\') */
		if (isSeperated(buf,nb_char))
		{
			char* temp = malloc(sizeof(char)*MAXLIGNE);
			int length_temp = 0;
			
			/** copier la première ligne sans '\' et '\0' (2 dernier caractères) */
			strncpy(temp,buf,nb_char-2);

			/** terminer la chaîne de caractères*/
			*(temp+nb_char-2) = '\0';

			length_temp = nb_char-2;
			nb_char = getline(&buf, &size, file);

			/** concaténer la 2è ligne avec la 1ère */
			strcat(temp,buf);
			length_temp+=nb_char;

			/** si les lignes suivantes sont aussi séparées */
			while (isSeperated(temp,length_temp))
			{	
				/** appliquer le même schéma d'instructions que précédemment */
				char* temp2 = malloc(sizeof(char)*MAXLIGNE);
				strncpy(temp2,temp,length_temp-2);
				*(temp2+length_temp-2) = '\0';
				free(temp);
				strcpy(temp,temp2);
				length_temp = length_temp-2;
				nb_char = getline(&buf, &size, file);
				strcat(temp,buf);
				length_temp+=nb_char;
				free(temp2);
			}
			/** récupérer (copier) la ligne complétement concaténée dans buf */
			strcpy(buf,temp);
			free(temp);
		}
			/** une fois la ligne prête, on peut la traiter
			  * si le traitement de la ligne retourne une sortie à comparer, la stocker dans sortie_test
			  * (nécessaire si la ligne 'p' est la dernière ligne) 
			  */
			if((sortie_traitement = traitement_ligne(buf, num_ligne, entree, sortie)) != NULL){
				strcpy(sortie_test, sortie_traitement);
			}	
	}

	/** si la sortie de la commande (sortie_test) est la même que la sortie prévue (>) */
	if(!strcmp(sortie, sortie_test)){
		printf("La suite de tests s'est bien passée\n");
		exit(0);
	}
	/** sinon une commande n'a pas la sortie prévue */
	else{
		printf("Une commande n'a pas renvoyé le texte attendu\n");
		exit(2);
	}

}

