/* Genetic algo, proof-of-concept (14/11/2005) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

float resolve(char *);
float fitness(char *);
char* decode(char *);
char* clear(char *);
char* god_create(void);

int ELEMENTS = 120;
int GENES;
int CROM_NUMB = 13;

float number = 12.00;
int verbose = 0;
char *bro = 0;
int cifre = 1;

int main(int argc, char **argv) {
	unsigned int reich = 0, new_gen = 0, c= 0, fr=0, old_gen = 0, limit, ok = 0;
	register int i = 0, n, crosspoint,a = 0;
	float fit;
	char	**individuo,
			*figli[2],
			*save,
			*save_d;

	if(argc==0) printf("usage: %s <magic number> <elements> <chromosomes> <length>\n", argv[0]); exit;
	if(argc==4) CROM_NUMB = atoi(argv[3]);
	if(argc==5) cifre = atoi(argv[4]);

	individuo = (char**)calloc(atoi(argv[2]),1024);
	ELEMENTS = atoi(argv[2]);
	number = atof(argv[1]);

	GENES = CROM_NUMB*4;

	printf("\n\n* Creazione della prima generazione\n");
	printf("\tnumero di elementi: %d\n", ELEMENTS);
	printf("\tcromosomi per ogni elemento: %d\n", CROM_NUMB);

	srand(time(NULL));

	for(i=0;i<ELEMENTS;i++) {
		if((individuo[i] = (char *)malloc(GENES+1))==NULL) {
			printf("errore: impossibile allocare nuovo spazio in memoria\n");
			while(i>=0) {
				free(individuo[i]);
			}
			return 0;
		}

		strncpy(individuo[i],god_create(), GENES+1);
		if(bro) free(bro);
		if(verbose==0) printf("individuo %d\t: %s\n", i, individuo[i]);
	}

	printf("\n* Selezione degli individui migliori in corso\n");

	old_gen = ELEMENTS;
	do {

		figli[0] = (char *)malloc(GENES+1);
		figli[1] = (char *)malloc(GENES+1);
	
		for(i=0;i<old_gen;i++) {
	
			printf("\tindividuo(%d)\t- %da gen.: ",i+1, ok+1);

			fit = fitness(individuo[i]);
				if(i==0) { limit = 1/fit; } 
				//printf("fitness = %2.2f\n", fit);
		
			if(fit==0) {
				i+=old_gen;
				ok=500;
			}
			else if (ok==0 && (1/fit)<limit) {
				printf("\t+ individuo adatto alla riproduzione\n");
				strcpy(figli[new_gen], individuo[i]);
				new_gen++;
			} else if (ok>0) {
				strcpy(figli[new_gen], individuo[i]);
				new_gen++;
			}

			if(new_gen==2) {

				crosspoint = rand() % (GENES-1) + 1;
				//printf("* crosspoint: %d\n", crosspoint);

				save = (char *)malloc(GENES+1);
				save_d = (char *)malloc(GENES+1);
				memset(save, '\0', sizeof(save));
				memset(save_d, '\0', sizeof(save_d));
				//printf("figlio: %s\n", figli[0]);

				strncat(save, figli[0], crosspoint);
				strncat(save, &figli[1][crosspoint], GENES-crosspoint);
				strncat(save_d, figli[1], crosspoint);
				strncat(save_d, &figli[0][crosspoint], GENES-crosspoint);

				//printf("figlio: %s\n", save);

				strcpy(individuo[a], save);
				strcpy(individuo[a+1], save_d);
				a+=2;
				new_gen=0;
			}


		}

		old_gen = a;
		a = 0;
		new_gen = 0;
		ok++;
	
	}while(ok<400);

	for(i=0;i<ELEMENTS;i++) {
		if(individuo[i]) free(individuo[i]);
	}

	return 0;
}

float resolve(char *individuo) {
	int i = 0;
	float result;
	char *c = malloc(2);


	sprintf(c, "%s", individuo+i);
	result = (float)atoi(c);
	for(i=1; i<strlen(individuo); i++) {
		sprintf(c, "%s", individuo+i);
		switch(*c) {
			case '+':
				result+=atoi(individuo+i+1);
				break;
					case '-':
				result-=atoi(individuo+i+1);
				break;

				case '*':
				result*=atoi(individuo+i+1);
				break;

				case '/':
				result/=atoi(individuo+i+1);
				break;
		}
	}
	if(c) free(c);
	return result;
  }



float fitness(char *individuo) {
	char *decoded = malloc(CROM_NUMB+1);
	char *cleaned;
	float fit = 0;
	/* decodifico il genoma dell'individuo */
	strcpy(decoded, decode(individuo));
	free(bro);
	cleaned = malloc(strlen(decoded));
	strcpy(cleaned, clear(decoded));
	free(bro);

	if(verbose==0) printf("cleaned: %s = %2.2f\n", cleaned, resolve(cleaned));
	fit=number-resolve(cleaned);

	if(fit==0)
		printf("* Possibile soluzione: %s\n", cleaned);
	
	if(decoded) free(decoded);
	if(cleaned) free(cleaned);
	return fit;
}

char *clear(char *individuo) {
	int i, a = 0, flag = 0, num = cifre;
	char *c = malloc(2);
	char *result = malloc(CROM_NUMB);
	bro = result;
 	memset(result, '\0', sizeof(result));

	for(i=0;i<CROM_NUMB;i++) {
		strncpy(c,individuo+i,1);
		if(num>0) {
			if(atoi(c) >= 1 && atoi(c) <= 9) {
				if(verbose==1) printf("%d", atoi(c));
				strncpy(result+a, c, 1);
				a++;
				num--;
			}
		  } else if(strchr(c,'+')||strchr(c,'-')||strchr(c,'*')||strchr(c,'/')) {
					strncpy(result+a, c, 1);
					a++;
					num = cifre;
			}
	}

	strncpy(c, result+a-1, 1);
	if(strchr(c,'+')||strchr(c,'-')||strchr(c,'*')||strchr(c,'/')) memset(result+a-1, '\0', strlen(result));
	else memset(result+a, '\0', 1);

	if(c) free(c);
	return result;
}

char* decode(char *individuo) {
	char alphabet[14] = {'0', '1', '2', '3', '4' ,'5', '6', '7', '8', '9', '+', '-', '*', '/' };
	int n, a = 0;
	register int i;
	char *temp = malloc(5);
	char *result = (char *)malloc(CROM_NUMB+1);
	bro = result;
	memset(result, '\0', CROM_NUMB+1);
	for(i=0; i<GENES; i+=4) {

		snprintf(temp, 5, "%s", individuo+i);	// prendo la char individuo a blocchi di 4 byte
		n = strtol(temp, '\0', 2);				// li copio all'interno di n convertendo da bin a dec
		if(n<=13) {								// se il cromosoma è valido (dentro l'alfabeto)
			strncpy(result+a, alphabet+n,1);			// +inserisco in result il valore corrispondente
			a++;
		}
	}

	if(temp) free(temp);
	return result;
}

char* god_create() {
	char *individuo = (char *)malloc(GENES+1);
	register int i = 0, n;
	bro = individuo;

	while(i<(GENES)) {
		n = rand() % 10;
		if(n%2 == 0) individuo[i] = '0';
		else individuo[i] = '1';
		i++;
	}
	individuo[GENES] = '\0';

	return individuo;
}
