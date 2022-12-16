#include <stdio.h>
#include "math.h"
#include "bfile.h"
#include <stdlib.h>

// clang getELFHeader.c -o test -lm

void toHex(int T[], int *size){
    for (int i = 0; i < *size/4; ++i) {
        int dec = 0;
        int y = 0;
        for (int j = 3; j >= 0; --j) {
                dec += T[j+4*i] * ((int) pow(2,y));
            y++;
        }
        T[i] = dec;

    }
    *size = *size/4;
}

//size = nombre de bits
void readData(BFILE *file, int T[], int *size){
    for (int i = 0; i < *size; ++i) {
        T[i] = bitread(file);
    }
    toHex(T,size);
}

void afficherheader(BFILE *file){
    int size = 416;
    int T[size];
    int result = 0;
    readData(file, T, &size);

    printf("En-tête ELF:\n");
    printf("  Magique:  ");
    for (int i = 0; i < 32; ++i) {
        if(i%2==0){
            printf(" ");
        }
        printf("%x",T[i]);
    }
    printf("\n");
    printf("  Classe:\t\t\t ELF");
    if(T[9]==1){
        printf("32\n");
    }else{
        printf("64\n");
    }
    printf("  Données:\t\t\t ");
    if(T[11]==1){
        printf("complément à 2, système à octets de poids faible d'abord (little endian)\n");
    }else{
        printf(" complément à 2, système à octets de poids fort d'abord (big endian)\n");
    }
    printf("  version: \t\t\t");
    if(T[13]==1){
        printf("1 (current)\n");
    }else{
        printf("0 (invalid)\n");
    }
    printf("  OS/ABI:\t\t\t A FAIRE\n");
    printf("  Version ABI:\t\t\t A FAIRE\n");
    printf("  Type:\t\t\t ");
    if(T[35]==1){
        printf("REL (Fichier de réadressage)\n");
    } else{
        printf("NON gérer par notre programme\n");
    }
    printf("  Machine:\t\t\t ");
    if(T[38]==2 && T[39] == 8){
        printf("ARM\n");
    } else{
        printf("NON gérer par notre programme\n");
    }
    printf("  Version:\t\t\t ");
    for (int i = 40; i < 47; ++i) {
        if(T[i]!=0){
            printf("0x%x",T[i]);
        }
    }
    printf("0x%x\n",T[47]);

    printf("  Adresse du point d'entrée: \t");
    for (int i = 48; i < 55; ++i) {
        if(T[i]!=0){
            printf("0x%x",T[i]);
        }
    }
    printf("0x%x\n",T[55]);
    printf("Début des en-têtes de programme: \t\t\t");
    printf("")
}



int main(int argc, char **argv){
    if(argc!=2){
        printf("Pas assez d'argument\n");
        return 1;
    }
    FILE *f = fopen(argv[1],"r");
    if(f == NULL){
        printf("le fichier n'a pas pu être ouvert\n");
        return 1;
    }
    BFILE *file = bstart(f,"r");
    afficherheader(file);
    printf("\n");
    return 0;
}