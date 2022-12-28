#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "bfile.h"

#define TAILLE_ELF_HEADER 52 // taille en octets


//
// Renvoie le contenu en decimal de T entre les bornes "start" et "end" (incluses)
//
int hextoDec(int T[], int start, int end) {
    int j = 0;
    int r = 0;

    for (int i = end; i >= start; i--) {
        r += T[i] * (int) pow(16,j);
        j++;
    }

    return r;
}

//
// Converti "size" bits du tableau T en hexadecimal
//
void bintoHex(int T[], int size) {
	int dec, y;

    for (int i = 0; i < size/4; i++) {
        dec = 0;
        y = 0;
        for (int j = 3; j >= 0; j--) {
            dec += T[j+4*i] * ((int) pow(2,y));
            y++;
        }
        T[i] = dec;
    }

	return;
}

//
// Stocke "size" bits du fichier dans le tableau T en hexadecimal
//
void readData(BFILE *file, int T[], int size) {
    for (int i = 0; i < size; i++) {
        T[i] = bitread(file);
    }
    bintoHex(T,size);

	return;
}

//
// Afficher le header ELF du fichier dans la console
//
void afficherELFHeader(BFILE *file) {
    int T[TAILLE_ELF_HEADER * 8];
    int flag;

    readData(file, T, TAILLE_ELF_HEADER * 8); // T[0] correspondra au premier caractere hexadecimal du fichier

    if (hextoDec(T,0,7) != 2135247942) {
        printf("ERREUR: pas un fichier ELF – a les mauvais octets magiques au départ\n");
        exit(1);
    }

    printf("En-tête ELF:\n");

    printf("  Magique:  ");
    for (int i = 0; i < 32; i++) {
        if (i%2 == 0) {
            printf(" ");
        }
        printf("%x",T[i]);
    }
    printf(" \n");

    printf("  Classe:                            ELF");
    if (T[9] == 1) {
        printf("32\n");
    } else {
        printf("64\n");
    }

    printf("  Données:                          ");
    if (T[11] == 1) {
        printf("complément à 2, système à octets de poids faible d'abord (little endian)\n");
    } else {
        printf("complément à 2, système à octets de poids fort d'abord (big endian)\n");
    }

    printf("  Version:                           ");
    if (T[13] == 1) {
        printf("1 (current)\n");
    } else {
        printf("0 (invalid)\n");
    }

    printf("  OS/ABI:                            ");
    if (T[14] == 0 && T[15] == 0) {
        printf("UNIX - System V\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Version ABI:                       ");
	if (T[16] == 0 && T[17] == 0) {
		printf("0\n");
	} else {
		printf("Non géré par notre programme\n");
	}

    printf("  Type:                              ");
    if (T[35] == 1) {
        printf("REL (Fichier de réadressage)\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Machine:                           ");
    if (T[38] == 2 && T[39] == 8) {
        printf("ARM\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Version:                           0x");
	flag = 0;
    for (int i = 40; i < 47; i++) {
        if (T[i] != 0 || flag == 1) {
            flag = 1;
            printf("%x",T[i]);
        }
    }
    printf("%x\n",T[47]);

    printf("  Adresse du point d'entrée:         0x");
    flag = 0;
    for (int i = 48; i < 55; i++) {
        if (T[i] != 0 || flag == 1) {
            flag = 1;
            printf("%x",T[i]);
        }
    }
    printf("%x\n",T[55]);

    printf("  Début des en-têtes de programme :  %d (octets dans le fichier)\n", hextoDec(T,56,63));
    printf("  Début des en-têtes de section :    %d (octets dans le fichier)\n", hextoDec(T,64,71));

    printf("  Fanions:                           ");
    if (hextoDec(T,72,79) == 83886080) {
        printf("0x5000000, Version5 EABI\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Taille de cet en-tête:             ");
    if (T[9] == 1) {
        printf("52 (octets)\n");
    } else {
        printf("64 (octets)\n");
    }

    printf("  Taille de l'en-tête du programme:  %d (octets)\n",hextoDec(T,84,87));
    printf("  Nombre d'en-tête du programme:     %d\n",hextoDec(T,88,91));
    printf("  Taille des en-têtes de section:    %d (octets)\n",hextoDec(T,92,95));
    printf("  Nombre d'en-têtes de section:      %d\n",hextoDec(T,96,99));
    printf("  Table d'indexes des chaînes d'en-tête de section: %d\n",hextoDec(T,100,103));

	return;
}



int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <fichier>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1],"r");
    if (f == NULL) {
        printf("ERREUR: le fichier n'a pas pu être ouvert\n");
        return 1;
    }

    BFILE *bf = bstart(f,"r");
	if (bf == NULL) {
		printf("ERREUR: impossible d'ouvrir le fichier en acces bit a bit\n");
		return 1;
	}

    afficherELFHeader(bf);
    bstop(bf);
    fclose(f);

    return 0;
}
