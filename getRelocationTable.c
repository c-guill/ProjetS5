#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include "getELF.h"


//
// Afficher la table des relogements du fichier dans la console
//
void afficherRelocationTable(FILE *file)
{
        char c;
        int i, j, q, n =0;
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    Elf32_Sym sym;
    Elf32_Sym *symtab;
    Elf32_Shdr *shdrtab;
    Elf32_Word r_info;
    Elf32_Addr r_offset;
    Elf32_Sword r_addend;
    unsigned char *tabCSym;
    unsigned char *tabC;


// recuperer infos sur le header

    ehdr=lireHeaderElf(file);

// recuperer infos sur les section headers

    if (ehdr.e_shnum == 0)
    {
        printf("Le fichier ne possede aucune en-tete de section\n");
        return;
    }

    fseek(file,ehdr.e_shoff-ehdr.e_ehsize,SEEK_CUR);
    shdrtab=lireSecHeaTable(file,ehdr);
    while (shdr.sh_type != SHT_SYMTAB){
        shdr = shdrtab[n];
        n++;
    }

// recuperer infos sur la table des symboles

    fseek(file,shdr.sh_offset,SEEK_SET);
    symtab = lireSymTable(file, shdr);
    q = shdr.sh_size/16;

// recuperer l'offset de la string table contenant le nom des symboles

    fseek(file,shdrtab[shdr.sh_link].sh_offset,SEEK_SET);
    tabCSym=lireSection(file,shdrtab[shdr.sh_link]);


// recuperer l'offset de la section header string table
    fseek(file,shdrtab[ehdr.e_shstrndx].sh_offset,SEEK_SET);
    tabC=lireSection(file,shdrtab[ehdr.e_shstrndx]);


// parcourir les section headers a la recherche de sections de relogements
    int q1 = 0;
    int q2 = 0;
    int m1 = 0;
    int m2 = 0;
    int curs1 = 0;
    int curs2 = 0;
    for (int i = 0; i < ehdr.e_shnum; i++){
        shdr=shdrtab[i];
        if (shdr.sh_type == SHT_REL) {
            if(q1==0 && q2==0){
                fseek(file,shdr.sh_offset,SEEK_SET);
            }
            q1++;
            if (m1 < shdr.sh_size / shdr.sh_entsize){
                m1 = shdr.sh_size / shdr.sh_entsize;
            }
        } else if(shdr.sh_type == SHT_RELA){
            if(q1==0 && q2==0){
                fseek(file,shdr.sh_offset,SEEK_SET);
            }
            q2++;
            if (m2 < shdr.sh_size / shdr.sh_entsize){
                m2 = shdr.sh_size / shdr.sh_entsize;
            }
        }
    }

    Elf32_Rel **rel=malloc(sizeof(Elf32_Rel *)*q1*m1);
    Elf32_Rela **rela=malloc(sizeof(Elf32_Rela *)*q2*m2);
    if(rel==NULL || rela == NULL){
        printf("Erreur d'allocation du tableau de relocation\n");
        exit(1);
    }
    lireRelocationTableComplete(file,ehdr,shdrtab,rel,rela);

        for (i = 0; i < ehdr.e_shnum; i++)
        {
        shdr=shdrtab[i];
                if (shdr.sh_type == SHT_REL || shdr.sh_type == SHT_RELA)
                {
                        printf("\nSection de réadressage '");

                // afficher infos sur la section de relogement

        n=0;
        c = tabC[shdr.sh_name];
        while (c)
        {
            printf("%c", c);
            n++;
            c = tabC[shdr.sh_name+n];
        }

        printf("' à l'adresse de décalage 0x%x contient %d ", shdr.sh_offset, shdr.sh_size / shdr.sh_entsize);
        if (shdr.sh_size / shdr.sh_entsize == 1)
            printf("entrée:\n");
        else
            printf("entrées:\n");
                
                // afficher le contenu de la section de relogement

                        printf("Décalage Info Type Val.-sym Noms-symboles\n");


        
                        for (j = 0; j < shdr.sh_size / shdr.sh_entsize; j++)
                        {
                        // afficher infos sur le relogement
                if (shdr.sh_type == SHT_REL){
                    r_offset = rel[curs1][j].r_offset;
                    r_info = rel[curs1][j].r_info;
                } else if(shdr.sh_type == SHT_RELA){
                    r_offset = rela[curs2][j].r_offset;
                    r_info = rela[curs2][j].r_info;
                    r_addend = rela[curs2][j].r_addend;
                }

                sym=symtab[ELF32_R_SYM(r_info)];


                            printf("%08x %08x ", r_offset, r_info);

                                switch (ELF32_R_TYPE(r_info))
                                {
                                        case R_ARM_ABS32:
                                                printf("R_ARM_ABS32 ");
                                                break;
                                        case R_ARM_CALL:
                                                printf("R_ARM_CALL ");
                                                break;
                                        case R_ARM_JUMP24:
                                                printf("R_ARM_JUMP24 ");
                                                break;
                                        default:
                                                printf("Non géré par notre programme ");
                                                break;
                                }
                        // afficher infos sur le symbole a reloger

                                if (ELF32_R_SYM(r_info) == STN_UNDEF)
                                {
                                        printf("symbole non defini\n");
                                }
                                else
                                {
                                        printf("%08x ", sym.st_value);

                    n=0;

                    if(ELF32_ST_TYPE(sym.st_info) == STT_SECTION){
                        c = tabC[shdrtab[sym.st_shndx].sh_name];
                        while (c)
                        {
                            printf("%c", c);
                            n++;
                            c = tabC[shdrtab[sym.st_shndx].sh_name+n];
                        }
                    }else{
                        c = tabCSym[sym.st_name];
                        while (c)
                        {
                            printf("%c", c);
                            n++;
                            c = tabCSym[sym.st_name+n];
                        }
                    }

                                    printf("\n");


                }


            }
            if (shdr.sh_type == SHT_REL){
                curs1++;
            } else if(shdr.sh_type == SHT_RELA){
                curs2++;
            }

                }
        }

}

//------------------------------------------------------------------------------

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <fichier>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1],"r");
    if (f == NULL) {
        printf("ERREUR: le fichier n'a pas pu être ouvert\n");
        return 1;
    }
    afficherRelocationTable(f);
    fclose(f);

    return 0;
}
