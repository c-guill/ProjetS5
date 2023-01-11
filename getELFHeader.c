#include <stdio.h>
#include <stdlib.h>
#include "getELF.h"


//
// Afficher le header ELF du fichier dans la console
//
void afficherELFHeader(FILE *file) {

    Elf32_Ehdr ehdr;
    ehdr=lireHeaderElf(file);

    if (ehdr.e_ident[0]!=0x7f || ehdr.e_ident[1]!='E' || ehdr.e_ident[2]!='L' || ehdr.e_ident[3]!='F') {
        printf("ERREUR: pas un fichier ELF – a les mauvais octets magiques au départ\n");
        exit(1);
    }

    printf("En-tête ELF:\n");

    printf("  Magique:  ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ",ehdr.e_ident[i]);
    }
    printf(" \n");

    printf("  Classe:                            ELF");

    if (ehdr.e_ident[4] == 1) {
        printf("32\n");
    } else {
        printf("64\n");
    }

    printf("  Données:                          ");
    if (ehdr.e_ident[5] == 1) {
        printf("complément à 2, système à octets de poids faible d'abord (little endian)\n");
    } else {
        printf("complément à 2, système à octets de poids fort d'abord (big endian)\n");
    }

    printf("  Version:                           ");
    if (ehdr.e_ident[6] == 1) {
        printf("1 (current)\n");
    } else {
        printf("0 (invalid)\n");
    }

    printf("  OS/ABI:                            ");
    if (ehdr.e_ident[7] == 0) {
        printf("UNIX - System V\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Version ABI:                       ");
    if (ehdr.e_ident[8] == 0) {
        printf("0\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Type:                              ");


    if (ehdr.e_type == 0x1) {
        printf("REL (Fichier de réadressage)\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Machine:                           ");

    if (ehdr.e_machine == 0x28) {
        printf("ARM\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Version:                           0x%x\n",ehdr.e_version);



    printf("  Adresse du point d'entrée:         0x%x\n",ehdr.e_entry);



    printf("  Début des en-têtes de programme :  %d (octets dans le fichier)\n", ehdr.e_phoff);
    printf("  Début des en-têtes de section :    %d (octets dans le fichier)\n", ehdr.e_shoff);

    printf("  Fanions:                           ");
    if (ehdr.e_flags==0x5000000) {
        printf("0x5000000, Version5 EABI\n");
    } else {
        printf("Non géré par notre programme\n");
    }

    printf("  Taille de cet en-tête:             %d (octets)\n",ehdr.e_ehsize);

    printf("  Taille de l'en-tête du programme:  %d (octets)\n",ehdr.e_phentsize);
    printf("  Nombre d'en-tête du programme:     %d\n",ehdr.e_phnum);
    printf("  Taille des en-têtes de section:    %d (octets)\n",ehdr.e_shentsize);
    printf("  Nombre d'en-têtes de section:      %d\n",ehdr.e_shnum);
    printf("  Table d'indexes des chaînes d'en-tête de section: %d\n",ehdr.e_shstrndx);

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

    afficherELFHeader(f);
    fclose(f);

    return 0;
}