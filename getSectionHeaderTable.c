#include <stdio.h>
#include "math.h"
#include "bfile.h"
#include <stdlib.h>
#include "getELFHeader.h"



//progm fihcier.txt dest.o

// clang getELFSection.c -o test -lm
//start et end inclus
int hextoDec(int T[], int start, int end){
    int j = 0;
    int r = 0;
    for (int i = end; i>=start ;i--){
        r += T[i] * (int) pow(16,j);
        j++;
    }
    return r;
}

void bintoHex(int T[], int *size){
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
    bintoHex(T,size);
}






//start at 00000550 middle 65
/*on aura besion des 4 info sur elfsection lu dans elfheader pour
-voir le debut et la taille du section depui elfheader,
 donc faut modifier size et le debut dans readData
-nombre de section headers et indexes
*/
void affichersection(BFILE *file,int debsec,int taillesec,int nbsec,int tabledenom){
    int size = debsec+(taillesec*nbsec);
    int T[size];
    int result = 0;
    readData(file, T, &size);
    
//on recupere les info concernant elfsection depuis elfheader
   /* int debsec=hextoDec(T,64,71);
    int taillesec=hextoDec(T,92,95);
    int nbsec=hextoDec(T,96,99);
    int tabledenom=hextoDec(T,100,103);*/
    //free(T);
   
//on relit le fichier avec un nouveau table
   // size = (taillesec*nbsec)+debsec; //le nombre de bytes par section *le nombre de section
    //readData(file, T, &size);
  
    printf("there are %d sections, starting at offset %d:\n\n",nbsec,debsec);
    printf("section headers:\n");
    printf("[nr] name\t type\t adrr\t off\t size    Es  flg  lk  inf  al  \n\n");

    for(int i=0;i<nbsec;i++){
        printf("[%d]      \t type\t adrr\t off\t size    Es  flg  lk  inf  al  \n",i);

    }

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
    //afficherheader(file);
    //ces 4 entier suivant sont a modifie pour chaque nouveau fichier.o 
    // ce qui veut dire qu'il faut execute getELFHeader du fichier avant getELFSection
    int debsec=1400;
    int taillesec=40; //change pas d'apres les infos sur internet
    int nbsec=23;
    int tabledenom=22;
    affichersection(file,debsec,taillesec,nbsec,tabledenom);
    printf("\n");
    bstop(file);
    fclose(f);
    return 0;
}
