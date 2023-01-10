#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getELF.h"


//
// Fusionner le contenu des sections de code des deux fichiers
//
void FusionSectionsCode(ELF_FILE *felf1, ELF_FILE *felf2, ELF_FILE *felfR, ELF_FUSION *fusion)
{
    int i, j, k;

    felfR->scodetab = (unsigned char **)malloc(sizeof(unsigned char *) * (felf1->ehdr.e_shnum + felf2->ehdr.e_shnum));
    fusion->off_concat = (int *)malloc(sizeof(int) * felf2->ehdr.e_shnum);

// parcourir les sections du premier fichier

    for (i = 0; i < felf1->ehdr.e_shnum; i++)
    {
        if (felf1->shdrtab[i].sh_type != SHT_PROGBITS)
            continue;
    
    // copier section header

        memcpy(&felfR->shdrtab[i], &felf1->shdrtab[i], sizeof(Elf32_Shdr));

    // fusionner sections de code
    
        felfR->scodetab[i] = (unsigned char *)malloc(sizeof(unsigned char) * felf1->shdrtab[i].sh_size);
        for (j = 0; j < felf1->shdrtab[i].sh_size; j++)
        {
            felfR->scodetab[i][j] = felf1->scodetab[i][j];
        }

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
                }

            // modifier infos section header et memoriser offset concatenation

                felfR->shdrtab[i].sh_size += felf2->shdrtab[j].sh_size;
                fusion->off_concat[j] = felf1->shdrtab[i].sh_size;

                break;
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
            memcpy(&felfR->shdrtab[felf1->ehdr.e_shnum + j], &felf2->shdrtab[j], sizeof(Elf32_Shdr));

            felfR->scodetab[felf1->ehdr.e_shnum + j] = (unsigned char *)malloc(sizeof(unsigned char) * felf2->shdrtab[j].sh_size);
            for (k = 0; k < felf2->shdrtab[j].sh_size; k++)
            {
                felfR->scodetab[felf1->ehdr.e_shnum + j][k] = felf2->scodetab[j][k];
            }
        }
    }

	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	FILE *f1, *f2;
	ELF_FILE felf1, felf2, felfR;
    ELF_FUSION fusion;
    int j, k;

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

    felfR.ehdr.e_shnum = felf1.ehdr.e_shnum + felf2.ehdr.e_shnum;
    felfR.shdrtab = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * felfR.ehdr.e_shnum);

    FusionSectionsCode(&felf1, &felf2, &felfR, &fusion);

    for (j = 0; j < felfR.ehdr.e_shnum; j++)
    {
        if (felfR.scodetab[j])
        {
            printf("contenu section de code de numero %d :\n", j);
            for (k = 0; k < felfR.shdrtab[j].sh_size; k++)
            {
                printf("%02x ", felfR.scodetab[j][k]);
            }
            printf("\n");
        }
    }

	fclose(f1);
	fclose(f2);

	return 0;
}
