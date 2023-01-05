#include <elf.h>
#include <stdio.h>
#include "getELF.h"


//
// Afficher la table des symboles du fichier dans la console
//
void afficherSymbolTable(FILE *file)
{
	char c;
	int i;
    int n;
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    Elf32_Sym sym;
    Elf32_Sym *symtab;
    Elf32_Shdr *shdrtab;
    char *tabC;
    char *tabCSym;
    int quantite;


    ehdr=lireHeaderElf(file);


    // parcourir les section headers a la recherche de la table des symboles

    if (ehdr.e_shnum == 0)
	{
		printf("Le fichier ne possede aucune en-tete de section\n");
		return;
	}
    fseek(file,ehdr.e_shoff-ehdr.e_ehsize,SEEK_CUR);
    shdrtab=lireSecHeaTable(file,ehdr);
    n=0;
    while (shdr.sh_type != SHT_SYMTAB){
        shdr = shdrtab[n];
        n++;
    }
    fseek(file,shdr.sh_offset,SEEK_SET);
    symtab = lireSymTable(file, shdr);
    quantite = shdr.sh_size/16;
    fseek(file,shdrtab[shdr.sh_link].sh_offset,SEEK_SET);
    tabCSym=lireSymbolName(file,shdrtab[shdr.sh_link]);
    fseek(file,shdrtab[ehdr.e_shstrndx].sh_offset,SEEK_SET);
    tabC=lireSectionName(file,shdrtab[ehdr.e_shstrndx]);


// afficher le nom de la section table des symboles

	printf("\nLa table de symboles « ");
    n=0;
    c = tabC[shdr.sh_name];
    while (c)
    {
        printf("%c", c);
        n++;
        c = tabC[shdr.sh_name+n];
    }

    printf(" ");

	printf(" » contient %d entrées :\n", quantite);

// se deplacer vers le debut de la table des symboles et la lire

	printf("   Num: Valeur Tail Type Lien Vis Ndx Nom\n");

	for (i = 0; i < quantite; i++)
	{
		sym = symtab[i];

		printf("     %d: %08x %d ", i, sym.st_value, sym.st_size);

		switch (ELF32_ST_TYPE(sym.st_info))
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

		switch (ELF32_ST_BIND(sym.st_info))
		{
			case STB_LOCAL:
				printf("LOCAL ");
				break;
			case STB_GLOBAL:
				printf("GLOBAL ");
				break;
			case STB_WEAK:
				printf("WEAK ");
				break;
			case STB_NUM:
				printf("NUM ");
				break;
			case STB_LOOS:
				printf("LOOS ");
				break;
			case STB_HIOS:
				printf("HIOS ");
				break;
			case STB_LOPROC:
				printf("LOPROC ");
				break;
			case STB_HIPROC:
				printf("HIPROC ");
				break;
			default:
				printf("ERREUR: bind du symbole inconnu ");
				break;
		}

		switch (ELF32_ST_VISIBILITY(sym.st_other))
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

		switch (sym.st_shndx)
		{
			case SHN_UNDEF:
				printf("UND ");
				break;
			case SHN_ABS:
				printf("ABS ");
				break;
			default:
				printf("%d ", sym.st_shndx);
				break;
		}

	// se deplacer vers l'indice du string table correspondant au nom du symbole et afficher le nom
        n = 0;
        c = tabCSym[sym.st_name];
        while (c && n < 17)
        {
            printf("%c", c);
            n++;
            c = tabCSym[sym.st_name+n];
        }

		printf("\n");

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


    afficherSymbolTable(f);
    fclose(f);

    return 0;
}
