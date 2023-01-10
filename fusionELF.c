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

void setDonnee(ELF_FILE *felf1, ELF_FILE *felf2, ELF_FILE *felfR, int s, int bool){
    Elf32_Shdr shdr1;
    Elf32_Shdr shdr2;
    Elf32_Sym sym1;
    Elf32_Sym sym2;
    int n, j = 0, b, q1, q2, r = 0;
    char c;
    while (shdr1.sh_type != SHT_SYMTAB){
        shdr1 = felf1->shdrtab[r];
        r++;
    }
    n=0;
    while (shdr2.sh_type != SHT_SYMTAB){
        shdr2 = felf2->shdrtab[n];
        n++;
    }
    q1 = shdr1.sh_size/16;
    q2 = shdr2.sh_size/16;
    if(!bool){
        q1=q2;
    }
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
            memcpy(&felfR->symtab[felfR->shdrtab[s].sh_size/16],&felf1->symtab[i],sizeof(Elf32_Sym));
            felfR->shdrtab[s].sh_size+= sizeof(Elf32_Sym);
        } else if (ELF32_ST_BIND(sym1.st_info) == STB_GLOBAL){
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
                            memcpy(&felfR->symtab[felfR->shdrtab[s].sh_size/16],&felf1->symtab[i],sizeof(Elf32_Sym));
                            felfR->shdrtab[s].sh_size+= sizeof(Elf32_Sym);
                        }else if(bool && sym1.st_shndx == SHN_UNDEF && sym2.st_shndx == SHN_UNDEF ){
                            memcpy(&felfR->symtab[felfR->shdrtab[s].sh_size/16],&felf1->symtab[i],sizeof(Elf32_Sym));
                            felfR->shdrtab[s].sh_size+= sizeof(Elf32_Sym);
                        }
                    }
                }
                j++;
            }
            j=0;
            if(b){
                memcpy(&felfR->symtab[felfR->shdrtab[s].sh_size/16],&felf1->symtab[i],sizeof(Elf32_Sym));
                felfR->shdrtab[s].sh_size+= sizeof(Elf32_Sym);
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
    int n = 0;
    while (felf1->shdrtab[n].sh_type != SHT_SYMTAB){
        n++;
    }
    memcpy(&felfR->shdrtab[n],&felf1->shdrtab[n], sizeof(Elf32_Shdr));
    felfR->shdrtab[n].sh_size=0;

    setDonnee(felf1, felf2, felfR, n, 1);
    setDonnee(felf2, felf1, felfR, n, 0);

}

void afficherELF(ELF_FILE felfR){
    int k, j, i=0;
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
    while (felfR.shdrtab[i].sh_type != SHT_SYMTAB){
        i++;
    }
    printf("Table de symbole : \n");
    for (j = 0; j < felfR.shdrtab[i].sh_size/felfR.shdrtab[i].sh_entsize; j++)
    {
        printf("%d: %08x %d ",j, felfR.symtab[j].st_value, felfR.symtab[j].st_size);
        switch (ELF32_ST_TYPE(felfR.symtab[j].st_info))
        {
            case STT_NOTYPE:
                printf("NOTYPE ");
                break;
            case STT_OBJECT:
                printf("OBJECT ");
                break;
            case STT_FUNC:
                printf("FUNC ");
                break;
            case STT_SECTION:
                printf("SECTION ");
                break;
            case STT_FILE:
                printf("FILE ");
                break;
            case STT_COMMON:
                printf("COMMON ");
                break;
            case STT_TLS:
                printf("TLS ");
                break;
            case STT_NUM:
                printf("NUM ");
                break;
            case STT_LOOS:
                printf("LOOS ");
                break;
            case STT_HIOS:
                printf("HIOS ");
                break;
            case STT_LOPROC:
                printf("LOPROC ");
                break;
            case STT_HIPROC:
                printf("HIPROC ");
                break;
            default:
                printf("ERREUR: type de symbole inconnu ");
                break;
        }
        switch (ELF32_ST_BIND(felfR.symtab[j].st_info))
        {
            case STB_LOCAL:
                printf("LOCAL ");
                break;
            case STB_GLOBAL:
                printf("GLOBAL ");
                break;
            default:
                printf("ERREUR: bind du symbole inconnu ");
                break;
        }
        switch (ELF32_ST_VISIBILITY(felfR.symtab[j].st_other))
        {
            case STV_DEFAULT:
                printf("DEFAULT ");
                break;
            case STV_INTERNAL:
                printf("INTERNAL ");
                break;
            case STV_HIDDEN:
                printf("HIDDEN ");
                break;
            case STV_PROTECTED:
                printf("PROTECTED ");
                break;
            default:
                printf("ERREUR: visibilite du symbole inconnu ");
                break;
        }
        switch (felfR.symtab[j].st_shndx)
        {
            case SHN_UNDEF:
                printf("UND ");
                break;
            case SHN_ABS:
                printf("ABS ");
                break;
            default:
                printf("%d ", felfR.symtab[j].st_shndx);
                break;
        }
        printf(" %d\n",felfR.symtab[j].st_name);
    }
}



//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	FILE *f1, *f2;
	ELF_FILE felf1, felf2, felfR;
    ELF_FUSION fusion;
    int j = 0, k = 0;

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

    while (felf1.shdrtab[j].sh_type != SHT_SYMTAB){
        j++;
    }
    while (felf2.shdrtab[k].sh_type != SHT_SYMTAB){
        k++;
    }
    felfR.symtab = malloc(sizeof(Elf32_Sym) * (felf1.shdrtab[j].sh_size/felf1.shdrtab[j].sh_entsize+felf2.shdrtab[k].sh_size/felf2.shdrtab[k].sh_entsize));

    FusionSectionsCode(&felf1, &felf2, &felfR, &fusion);
    Fusionrenumerotationsymboles(&felf1, &felf2, &felfR);


    fclose(f1);
	fclose(f2);

    afficherELF(felfR);



	return 0;
}
