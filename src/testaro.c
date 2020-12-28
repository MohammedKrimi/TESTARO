#include "treatment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char const *argv[])
{
	/** Ouverture du fichier de description */
	FILE* file = fopen(argv[1],"r");
	
	/** Traiter le ficher */
	traitement(file);

	fclose(file);

	exit(0);
}