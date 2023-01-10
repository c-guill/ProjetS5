#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getELF.h"

void createString(ELF_FILE *felf, Elf32_Word name, char *result){
    int n = 0;
    char c = felf->strtab[name];
    while (c)
    {
        result[n]=c;
        n++;
        c = felf->strtab[name+n];
    }
    result[n]='\0';
}

void setDonnee(ELF_FILE *felf1, ELF_FILE *felf2, ELF_FILE *felfR, int bool){
    Elf32_Shdr shdr1;
    Elf32_Shdr shdr2;
    Elf32_Sym sym1;
    Elf32_Sym sym2;
    int n = 0, j = 0, b = 1, q1, q2;
    char c;
    while (shdr1.sh_type != SHT_SYMTAB){
        shdr1 = felf1->shdrtab[n];
        n++;
    }
    n=0;
    while (shdr2.sh_type != SHT_SYMTAB){
        shdr2 = felf2->shdrtab[n];
        n++;
    }
    q1 = shdr1.sh_size/16;
    q2 = shdr2.sh_size/16;
    for (int i = 0; i < q1; ++i) {
        sym1 = felf1->symtab[i];
        n = 0;
        c = felf1->strtab[sym1.st_name];
        while (c)
        {
            n++;
            c = felf1->strtab[sym1.st_name+n];
        }
        char *result1 = malloc(sizeof(char) * (n+1));
        createString(felf1,sym1.st_name,result1);
        if(ELF32_ST_BIND(sym1.st_info) == STB_LOCAL){
            memcpy(&felfR->symtab[i],&felf1->symtab[i],sizeof(Elf32_Sym));
        } else if (ELF32_ST_BIND(sym1.st_info) == STB_GLOBAL){
            n=0;
            b=1;
            while (j < q2){
                sym2 = felf2->symtab[j];
                if (ELF32_ST_BIND(sym2.st_info) == STB_GLOBAL){
                    n = 0;
                    c = felf2->strtab[sym2.st_name];
                    while (c)
                    {
                        n++;
                        c = felf1->strtab[sym1.st_name+n];
                    }
                    char *result2 = malloc(sizeof(char) * (n+1));
                    createString(felf2,sym2.st_name,result2);
                    if(!strcmp(result1,result2)){
                        b=0;
                        if (sym1.st_shndx != SHN_UNDEF && sym2.st_shndx != SHN_UNDEF ){
                            printf("Conflit symbole globaux, Edition de lien annulé.\n");
                            exit(1);
                        }
                        if (sym1.st_shndx != SHN_UNDEF && sym2.st_shndx == SHN_UNDEF ){
                            memcpy(&felfR->symtab[i],&felf1->symtab[i],sizeof(Elf32_Sym));
                        }else if(bool && sym1.st_shndx == SHN_UNDEF && sym2.st_shndx == SHN_UNDEF ){
                            memcpy(&felfR->symtab[i],&felf1->symtab[i],sizeof(Elf32_Sym));
                        }
                    }
                }
                j++;
            }
            if(b){
                memcpy(&felfR->symtab[i],&felf1->symtab[i],sizeof(Elf32_Sym));
            }
        }
    }
}


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

void Fusionrenumerotationsymboles(ELF_FILE *felf1, ELF_FILE *felf2, ELF_FILE *felfR){

    setDonnee(felf1, felf2, felfR, 1);
    setDonnee(felf2, felf1, felfR, 0);

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
    Fusionrenumerotationsymboles(&felf1, &felf2, &felfR);

    fclose(f1);
	fclose(f2);

	return 0;
}
