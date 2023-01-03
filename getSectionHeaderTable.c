#include <elf.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfile.h"

#define FLAG_WRITE (1 << 0)
#define FLAG_ALLOC (1 << 1)
#define FLAG_EXECINSTR (1 << 2)
#define FLAG_M (1 << 4)
#define FLAG_S (1 << 5)
#define FLAG_I (1 << 6)
#define FLAG_ARM_NOREAD (1 << 29)
// d'autres flags apparaissent dans readelf pour file1.o mais non documentes
// va surement falloir les ajouter a la main


FILE *f;   // pointeur fichier objet
BFILE *bf; // pointeur acces bit a bit fichier objet
char fname[1024]; // nom du fichier objet
char fmode[1]; // mode d'ouverture du fichier objet
int nbits; // nombre de bits lus dans le fichier



//
// Ouvrir un fichier binaire en acces bit a bit
//
void openBinFile(void)
{
	f = fopen(fname, fmode);
	if (!f)
	{
        printf("ERREUR: le fichier n'a pas pu être ouvert\n");
        exit(1);
    }

	bf = bstart(f, fmode);
	if (!bf)
	{
		printf("ERREUR: impossible d'ouvrir le fichier en acces bit a bit\n");
		fclose(f);
		exit(1);
	}

	nbits = 0;

	return;
}

//
// Fermer un fichier binaire en acces bit a bit
//
void closeBinFile(void)
{
	if (bf)
	{
		bstop(bf);
	}
	
	fclose(f);

	return;
}

//
// Renvoyer le contenu en decimal de T entre les bornes "start" et "end" (incluses)
//
int hextoDec(int T[], int start, int end) {
	int j = 0;
    int r = 0;

    for (int i = end; i >= start; i--) {
        r += T[i] * (int) pow(16,j);
        j++;
    }

    return r;
}

//
// Convertir "size" bits du tableau T en hexadecimal
//
void bintoHex(int T[], int size) {
	int dec, y;

    for (int i = 0; i < size/4; i++) {
        dec = 0;
        y = 0;
        for (int j = 3; j >= 0; j--) {
            dec += T[j+4*i] * ((int) pow(2,y));
            y++;
        }
        T[i] = dec;
    }

	return;
}

//
// Stocker "size" bits du fichier dans le tableau T en hexadecimal
//
void readData(int T[], int size) {
    for (int i = 0; i < size; i++) {
        T[i] = bitread(bf);
    }
    bintoHex(T,size);

	nbits += size;

	return;
}

//
// Ignorer "size" bits du fichier
//
void skipData(int size) {
    for (int i = 0; i < size; i++) {
        bitread(bf);
    }

	nbits += size;

	return;
}

//
// Lire 32 bits du fichier et renvoyer la valeur lue
//
uint32_t readWord(void)
{
	uint32_t val;
	int T[32];
	int i, y;

	readData(T, 32);
	val = 0;
	y = 0;

	for (i = 7; i >= 0; i--)
	{
		val += T[i] * pow(16, y);
		y++;
	}

	return val;
}

//
// Lire 16 bits du fichier et renvoyer la valeur lue
//
uint16_t readHalf(void)
{
	uint16_t val;
	int T[16];
	int i, y;

	readData(T, 16);
	val = 0;
	y = 0;

	for (i = 3; i >= 0; i--)
	{
		val += T[i] * pow(16, y);
		y++;
	}

	return val;
}

//
// Lire 8 bits du fichier et renvoyer la valeur lue
//
uint8_t readByte(void)
{
	uint8_t val;
	int T[8];
	int i, y;

	readData(T, 8);
	val = 0;
	y = 0;

	for (i = 1; i >= 0; i--)
	{
		val += T[i] * pow(16, y);
		y++;
	}

	return val;
}

//
// Afficher la table des en-tetes de sections du fichier dans la console
//
void afficherSectionHeaderTable(void)
{
	Elf32_Off e_shoff, shstrtab_offset;
	Elf32_Half e_shentsize, e_shnum, e_shstrndx;
	Elf32_Shdr shdr;
	char c;
	int i, n;

// recuperer infos dans le ELF header
	
	skipData(256);
	e_shoff = readWord();
	skipData(80);
	e_shentsize = readHalf();
	e_shnum = readHalf();
	e_shstrndx = readHalf();

// recuperer l'offset de la section header string table

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	for (i = 0; i < e_shstrndx; i++)
	{
		skipData(e_shentsize * 8);
	}

	skipData(128);
	shstrtab_offset = readWord();

// parcourir les section headers en les affichant

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	if (e_shnum == 0)
	{
		printf("Le fichier ne possede aucune en-tete de section\n");
		return;
	}

	printf("Il y a %d en-têtes de section, débutant à l'adresse de décalage 0x%x:", e_shnum, e_shoff);
	printf("\n\nEn-têtes de section :\n");
	printf("  [Nr] Nom Type Adr Décala. Taille ES Fan LN Inf Al\n");

	for (i = 0; i < e_shnum; i++)
	{
		shdr.sh_name = readWord();
		shdr.sh_type = readWord();
		shdr.sh_flags = readWord();
		shdr.sh_addr = readWord();
		shdr.sh_offset = readWord();
		shdr.sh_size = readWord();
		shdr.sh_link = readWord();
		shdr.sh_info = readWord();
		shdr.sh_addralign = readWord();
		shdr.sh_entsize = readWord();

		printf("  [%d] ", i);
	
	// se deplacer vers l'indice du section header string table correspondant au nom de la section

		closeBinFile();
		openBinFile();

		skipData((shstrtab_offset + shdr.sh_name) * 8);

	// afficher le nom de la section (readelf n'affiche que les 17 premiers caracteres)

		n = 0;
		c = readByte();
		while (c && n < 17)
		{
			printf("%c", c);
			c = readByte();
			n++;
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

	// revenir au prochain en-tete de section a afficher

		closeBinFile();
		openBinFile();

		skipData((e_shoff + ((i + 1) * e_shentsize)) * 8);
	}

	printf("Clé des fanions :\n  W (écriture), A (allocation), X (exécution), M (fusion), S (chaînes), I (info),\n  L (ordre des liens), O (traitement supplémentaire par l'OS requis), G (groupe),\n  T (TLS), C (compressé), x (inconnu), o (spécifique à l'OS), E (exclu),\n  y (purecode), p (processor specific)\n");

	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <fichier>\n", argv[0]);
        return 1;
    }
	
	strcpy(fname, argv[1]);
	strcpy(fmode, "r");

    openBinFile();
    afficherSectionHeaderTable();
    closeBinFile();

    return 0;
}
