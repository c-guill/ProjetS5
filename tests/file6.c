#include <stdio.h>    // pour printf, scanf, etc
#include <stdlib.h>   // pour malloc
#include <string.h>   // pour strcpy

//----------------------------------------
typedef struct
{
    char nom[100];
    char prenom[50];
    double taille;
} personne;

//----------------------------------------
void affiche_personne (personne *pers)
{
    // verifie que le pointeur n'est pas nul
    if (pers != NULL) {
        // pers est un pointeur, donc il faut utiliser la fleche -> 
        printf("%s %s : %.2f m\n", pers->nom, pers->prenom, pers->taille);
    }
    else {
        printf ("pointeur nul !\n");
    }
}

//----------------------------------------
int main()
{
    FILE *fichier;
    char nomfichier[500];
    personne pers;
    int nb;
    
    printf("nom du fichier ? ");
    fgets(nomfichier, 500, stdin);
    nomfichier[strlen(nomfichier)-1] = '\0';
    
    fichier = fopen(nomfichier, "rb");   // fichier binaire (b) ouvert en lecture (r=read)
    
    if (fichier != NULL) 
    {
        // tant que on arrive Ã  lire les infos d'une personne
        // et en meme temps a les stocker a l'adresse de la variable pers
        nb = 0;
        while ( (fread(&pers, sizeof(personne), 1, fichier)) == 1)
        {
            affiche_personne (&pers);
            nb ++;
        }
        fclose (fichier); // NE JAMAIS OUBLIER DE FERMER LES FICHIERS OUVERTS
        
        printf ("Il y a %d personnes dans le fichier %s\n", nb, nomfichier);
    }
    else 
    {
        printf("Erreur : le fichier %s ne peut etre ouvert.\n", nomfichier);
    }
    
    return 0;
}







