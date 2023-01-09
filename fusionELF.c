#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getELF.h"


//
// Afficher dans la console la fusion des sections de code
//
// TODO: memoriser les changements de numero des sections du second fichier
//
void afficherFusionSectionsCode(ELF_FILE *felf1, ELF_FILE *felf2, ELF_FILE *felfR, ELF_FUSION *fusion)
{
    int i, j, k;

    felfR->scodetab = (unsigned char **)malloc(sizeof(unsigned char *) * (felf1->ehdr.e_shnum + felf2->ehdr.e_shnum));
    fusion->off_concat = (int *)malloc(sizeof(int) * felf2->ehdr.e_shnum);

// parcourir les sections du premier fichier

    for (i = 0; i < felf1->ehdr.e_shnum; i++)
    {
        if (felf1->shdrtab[i].sh_type != SHT_PROGBITS)
            continue;

    // fusionner sections de code
        //free(felfR->scodetab[i]);
        felfR->scodetab[i] = (unsigned char *)malloc(sizeof(unsigned char) * felf1->shdrtab[i].sh_size);
        for (j = 0; j < felf1->shdrtab[i].sh_size; j++)
        {
            felfR->scodetab[i][j] = felf1->scodetab[i][j];
            //c = felf1->scodetab[i][j];
            //printf("%02x ", c);
        }/*
        printf("section %d\nfelf1->shdrtab[i] avant concat : ", i);
        for (j = 0; j < felf1->shdrtab[i].sh_size; j++)
        {
            printf("%02x ", felfR->scodetab[i][j]);
        }
        printf("\n");*/
        //fwrite(felf1->scodetab[i], felf1->shdrtab[i].sh_size, 1, stdin);

    // si section correspondante dans le second fichier, concatener son contenu

        for (j = 0; j < felf2->ehdr.e_shnum; j++)
        {
            if (felf2->shdrtab[j].sh_type != SHT_PROGBITS)
                continue;
            
            if (!strcmp((const char *)&felf1->shstrtab[felf1->shdrtab[i].sh_name], (const char *)&felf2->shstrtab[felf2->shdrtab[j].sh_name]))
            {
                felfR->scodetab[i] = (unsigned char *)realloc(felfR->scodetab[i], sizeof(unsigned char) * (felf1->shdrtab[i].sh_size + felf2->shdrtab[j].sh_size));
                for (k = 0; k < felf2->shdrtab[j].sh_size; k++)
                {
                    felfR->scodetab[i][felf1->shdrtab[i].sh_size + k] = felf2->scodetab[j][k];
                    //c = felf1->scodetab[i][j];
                    //printf("%02x ", c);
                }

            // modifier infos section header et memoriser offset concatenation

                felfR->shdrtab[i].sh_size += felf2->shdrtab[j].sh_size;
                fusion->off_concat[j] = felf1->shdrtab[i].sh_size;
/*
                printf("felfR->scodetab[i] apres concat : ");
                for (k = 0; k < felf1->shdrtab[i].sh_size + felf2->shdrtab[j].sh_size; k++)
                {
                    printf("%02x ", felfR->scodetab[i][k]);
                }
                printf("\n");
                break;
                //printf("+ ");
                //for (k = 0; k < felf2->shdrtab[j].sh_size; k++)
                //{
                //    c = felf2->scodetab[j][k];
                //    printf("%02x ", c);
                //}
*/
            }
        }
    }

// parcourir les sections du deuxieme fichier

    for (j = 0; j < felf2->ehdr.e_shnum; j++)
    {
        if (felf2->shdrtab[j].sh_type != SHT_PROGBITS)
            continue;

    // si pas de section correspondante dans le premier fichier, stocker son contenu

        for (i = 0; i < felf1->ehdr.e_shnum; i++)
        {
            if (felf1->shdrtab[i].sh_type != SHT_PROGBITS)
                continue;
            
            if (!strcmp((const char *)&felf1->shstrtab[felf1->shdrtab[i].sh_name], (const char *)&felf2->shstrtab[felf2->shdrtab[j].sh_name]))
            {
                break;
            }
        }

        if (i == felf1->ehdr.e_shnum)
        {
            felfR->scodetab[felf1->ehdr.e_shnum + j] = (unsigned char *)malloc(sizeof(unsigned char) * felf2->shdrtab[j].sh_size);
            for (k = 0; k < felf2->shdrtab[j].sh_size; k++)
            {
                felfR->scodetab[felf1->ehdr.e_shnum + j][k] = felf2->scodetab[j][k];
            }
        }
    }

//	printf("\n");
	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	FILE *f1, *f2;
	ELF_FILE felf1, felf2, felfR;
    ELF_FUSION fusion;

	if (argc != 3)
	{
		printf("usage: %s <fichier1> <fichier2>\n", argv[0]);
		return 1;
	}

	f1 = fopen(argv[1], "r");
	f2 = fopen(argv[2], "r");

	if (!f1 || !f2)
	{
		printf("ERREUR d'ouverture des fichiers ELF\n");
		return 1;
	}

	felf1 = lireFichierELF(f1);
	felf2 = lireFichierELF(f2);

    afficherFusionSectionsCode(&felf1, &felf2, &felfR, &fusion);
/*
    int i;
    for (i = 0; )
*/
	fclose(f1);
	fclose(f2);

	return 0;
}
