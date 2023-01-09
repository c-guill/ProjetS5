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
void afficherFusionSectionsCode(ELF_FILE *felf1, ELF_FILE *felf2)
{
    int i, j, k;
    unsigned char c;

// afficher sections de code du premier fichier

    for (i = 0; i < felf1->ehdr.e_shnum; i++)
    {
        if (felf1->shdrtab[i].sh_type != SHT_PROGBITS)
            continue;

    // afficher le contenu de la section

        for (j = 0; j < felf1->shdrtab[i].sh_size; j++)
        {
            c = felf1->scodetab[i][j];
            printf("%02x ", c);
        }
        //fwrite(felf1->scodetab[i], felf1->shdrtab[i].sh_size, 1, stdin);

    // si section correspondante dans le second fichier, concatener son contenu

        for (j = 0; j < felf2->ehdr.e_shnum; j++)
        {
            if (felf2->shdrtab[j].sh_type != SHT_PROGBITS)
                continue;
            
            if (!strcmp(&felf1->shstrtab[felf1->shdrtab[i].sh_name], &felf2->shstrtab[felf2->shdrtab[j].sh_name]))
            {
                printf("\n+ ");
                for (k = 0; k < felf2->shdrtab[j].sh_size; k++)
                {
                    c = felf2->scodetab[j][k];
                    printf("%02x ", c);
                }
            }
        }
        printf("\n\n");
    }
	
	printf("\n");
	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	FILE *f1, *f2;
	ELF_FILE felf1, felf2;

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

    afficherFusionSectionsCode(&felf1, &felf2);

	fclose(f1);
	fclose(f2);

	return 0;
}
