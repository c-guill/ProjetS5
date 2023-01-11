#include <elf.h>
#include <stdio.h>
#include "getELF.h"

#define FLAG_WRITE (1 << 0)
#define FLAG_ALLOC (1 << 1)
#define FLAG_EXECINSTR (1 << 2)
#define FLAG_M (1 << 4)
#define FLAG_S (1 << 5)
#define FLAG_I (1 << 6)
#define FLAG_ARM_NOREAD (1 << 29)

//
// Afficher la table des en-tetes de sections du fichier dans la console
//
void afficherSectionHeaderTable(FILE *file)
{
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    Elf32_Shdr *shdrtab;
    unsigned char *tabC;
    char c;
    int n;
    ehdr=lireHeaderElf(file);
    fseek(file,ehdr.e_shoff-ehdr.e_ehsize,SEEK_CUR);
    shdrtab=lireSecHeaTable(file,ehdr);
    fseek(file,shdrtab[ehdr.e_shstrndx].sh_offset,SEEK_SET);
    tabC=lireSection(file,shdrtab[ehdr.e_shstrndx]);

    if (ehdr.e_shnum == 0)
	{
		printf("Le fichier ne possede aucune en-tete de section\n");
		return;
	}

	printf("Il y a %d en-têtes de section, débutant à l'adresse de décalage 0x%x:", ehdr.e_shnum, ehdr.e_shoff);
	printf("\n\nEn-têtes de section :\n");
	printf("  [Nr] Nom Type Adr Décala. Taille ES Fan LN Inf Al\n");
	for (int i = 0; i < ehdr.e_shnum; i++)
	{
        printf("  [%d] ", i);
        shdr=shdrtab[i];

	    // afficher le nom de la section (readelf n'affiche que les 17 premiers caracteres)

		n = 0;
		c = tabC[shdr.sh_name];
		while (c && n < 17)
		{
			printf("%c", c);
			n++;
            c = tabC[shdr.sh_name+n];
        }

		printf(" ");

		switch (shdr.sh_type)
		{
			case SHT_NULL:
				printf("NULL ");
				break;
			case SHT_PROGBITS:
				printf("PROGBITS ");
				break;
			case SHT_SYMTAB:
				printf("SYMTAB ");
				break;
			case SHT_STRTAB:
				printf("STRTAB ");
				break;
			case SHT_RELA:
				printf("RELA ");
				break;
			case SHT_HASH:
				printf("HASH ");
				break;
			case SHT_DYNAMIC:
				printf("DNYNAMIC ");
				break;
			case SHT_NOTE:
				printf("NOTE ");
				break;
			case SHT_NOBITS:
				printf("NOBITS ");
				break;
			case SHT_REL:
				printf("REL ");
				break;
			case SHT_SHLIB:
				printf("SHLIB ");
				break;
			case SHT_DYNSYM:
				printf("DYNSYM ");
				break;
			case SHT_LOPROC:
				printf("LOPROC ");
				break;
			case SHT_HIPROC:
				printf("HIPROC ");
				break;
			case SHT_LOUSER:
				printf("LOUSER ");
				break;
			case SHT_HIUSER:
				printf("HIUSER ");
				break;
			case SHT_ARM_EXIDX:
				printf("ARM_EXIDX ");
				break;
			case SHT_ARM_PREEMPTMAP:
				printf("ARM_PREEMPTMAP ");
				break;
			case SHT_ARM_ATTRIBUTES:
				printf("ARM_ATTRIBUTES ");
				break;
			case 0x70000004: // pas de macro definie dans elf.h
				printf("ARM_DEBUGOVERLAY ");
				break;
			case 0x70000005: // pas de macro definie dans elf.h
				printf("ARM_OVERLAYSECTION ");
				break;
			default:
				printf("ERREUR: type de section header inconnu ou non géré par notre programme ");
		}



		printf("%08x %06x %06x %02x ", shdr.sh_addr, shdr.sh_offset, shdr.sh_size, shdr.sh_entsize);

        if ((shdr.sh_flags & FLAG_WRITE) == FLAG_WRITE) printf("W");
		if ((shdr.sh_flags & FLAG_ALLOC) == FLAG_ALLOC) printf("A");
		if ((shdr.sh_flags & FLAG_EXECINSTR) == FLAG_EXECINSTR) printf("X");
		if ((shdr.sh_flags & FLAG_M) == FLAG_M) printf("M");
		if ((shdr.sh_flags & FLAG_S) == FLAG_S) printf("S");
		if ((shdr.sh_flags & FLAG_I) == FLAG_I) printf("I");
		if ((shdr.sh_flags & FLAG_ARM_NOREAD) == FLAG_ARM_NOREAD) printf("p");

		printf(" %d %d %d\n", shdr.sh_link, shdr.sh_info, shdr.sh_addralign);
    }

	printf("Clé des fanions :\n  W (écriture), A (allocation), X (exécution), M (fusion), S (chaînes), I (info),\n  L (ordre des liens), O (traitement supplémentaire par l'OS requis), G (groupe),\n  T (TLS), C (compressé), x (inconnu), o (spécifique à l'OS), E (exclu),\n  y (purecode), p (processor specific)\n");

	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <fichier>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1],"r");
    if (f == NULL)
    {
        printf("ERREUR: le fichier n'a pas pu être ouvert\n");
        return 1;
    }

    afficherSectionHeaderTable(f);
    fclose(f);

    return 0;
}
