#include <stdio.h>
#include "../bfile.h"

int HextoDec(char c){
    if(c=='f'){
        return 15;
    }
    if(c=='e'){
        return 14;
    }
    if(c=='d'){
        return 13;
    }
    if(c=='c'){
        return 12;
    }
    if(c=='b'){
        return 11;
    }
    if(c=='a'){
        return 10;
    }
    if(c >= 48 && c <58){
        return c-48;
    }
    return c;
}

void Dectobin(int T[], int i){
    for (int j = 0; j < 4; ++j) {
        T[j]=0;
    }
    if(i>=8){
        T[0]=1;
        i-=8;
    }
    if(i>=4){
        T[1]=1;
        i-=4;
    }
    if(i>=2){
        T[2]=1;
        i-=2;
    }
    if(i>=1){
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
    while (c!=83){
        fscanf(source,"%c",&c);
        if(c >= 48 && c <58 || c >= 97 && c <103){
            Dectobin(T,HextoDec(c));
            for (int i = 0; i < 4; ++i) {
                bitwrite(dest,T[i]);
            }
        }
    }
    printf("Le fichier de destination est prêt.\n");
    bstop(dest);
    fclose(source);
    fclose(f2);
    return 0;

}