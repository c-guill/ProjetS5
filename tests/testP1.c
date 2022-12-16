#include <stdio.h>
#include "../bfile.h"
#include <stdlib.h>
//fichier non terminé

void dectobin(int T[], int i){
    for (int j = 0; j < 4; ++j) {
        T[j]=0;
    }
    if(i<=8){
        T[0]=1;
        i-=8;
    }
    if(i<=4){
        T[1]=1;
        i-=4;
    }
    if(i<=2){
        T[2]=1;
        i-=2;
    }
    if(i<=1){
        T[3]=1;
    }
}

int main(int argc, char **argv){
    if(argc!=3){
        printf("<fichier.txt> nom_du_fichier\n");
        return 1;
    }
    FILE *source = fopen(argv[1],"r");
    if(source == NULL){
        printf("le fichier n'a pas pu être ouvert\n");
        return 1;
    }
    FILE *f2 = fopen(argv[2],"w");
    if(f2 == NULL){
        printf("le fichier 2 n'a pas pu être créer\n");
        return 1;
    }
    BFILE *dest = bstart(f2,"w");
    char c=0;
    int T[4];
    while (c==0){
        fscanf(source,"%c",&c);
        dectobin(T,c);
        for (int i = 0; i < 4; ++i) {
            bitwrite(dest,T[i]);
        }

    }
    bstop(dest);
    fclose(source);
    fclose(f2);


}