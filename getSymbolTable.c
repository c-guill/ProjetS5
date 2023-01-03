#include <elf.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfile.h"


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
// Afficher la table des symboles du fichier dans la console
//
// TODO :
//	- verifier que le fichier est un ELF au bon format
//	- gerer les fichiers sans section headers, sans table des symboles etc.. (robustesse)
//
void afficherSymbolTable(void)
{
	Elf32_Word symtab_name, symtab_type, symtab_size, symtab_link, symtab_entsize;
	Elf32_Off e_shoff, symtab_offset, shstrtab_offset, strtab_offset;
	Elf32_Half e_shentsize, e_shnum, e_shstrndx;
	Elf32_Sym sym;
	char c;
	int i;

// recuperer infos dans le ELF header
	
	skipData(256);
	e_shoff = readWord();
	
	skipData(80);
	e_shentsize = readHalf();
	e_shnum = readHalf();
	e_shstrndx = readHalf();

// se deplacer vers le debut du tableau de section headers

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

// parcourir les section headers a la recherche de la table des symboles

	if (e_shnum == 0)
	{
		printf("Le fichier ne possede aucune en-tete de section\n");
		return;
	}

	symtab_name = readWord();
	symtab_type = readWord();

	i = 0;
	while (symtab_type != SHT_SYMTAB)
	{
		i++;

		if (i == e_shnum)
		{
			printf("Le fichier ne possede pas de table des symboles\n");
			return;
		}

		skipData(e_shentsize * 8 - 64);
		symtab_name = readWord();
		symtab_type = readWord();
	}
	
	skipData(64);
	symtab_offset = readWord();
	symtab_size = readWord();
	symtab_link = readWord();
	skipData(64);
	symtab_entsize = readWord();

// recuperer l'offset de la string table contenant le nom des symboles

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	for (i = 0; i < symtab_link; i++)
	{
		skipData(e_shentsize * 8);
	}

	skipData(128);
	strtab_offset = readWord();

// se deplacer vers l'indice du section header string table correspondant au nom de la table des symboles

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	for (i = 0; i < e_shstrndx; i++)
	{
		skipData(e_shentsize * 8);
	}

	skipData(128);
	shstrtab_offset = readWord();

	closeBinFile();
	openBinFile();

	skipData((shstrtab_offset + symtab_name) * 8);

// afficher le nom de la section table des symboles

	printf("\nLa table de symboles « ");

	c = readByte();
	while (c)
	{
		printf("%c", c);
		c = readByte();
	}

	printf(" » contient %d entrées :\n", symtab_size / symtab_entsize);

// se deplacer vers le debut de la table des symboles et la lire

	closeBinFile();
	openBinFile();

	printf("   Num: Valeur Tail Type Lien Vis Ndx Nom\n");
	
	skipData(symtab_offset * 8);
	
	for (i = 0; i < symtab_size / symtab_entsize; i++)
	{
		sym.st_name = readWord();
		sym.st_value = readWord();
		sym.st_size = readWord();
		sym.st_info = readByte();
		sym.st_other = readByte();
		sym.st_shndx = readHalf();

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

		closeBinFile();
		openBinFile();

		skipData((strtab_offset + sym.st_name) * 8);

		c = readByte();
		while (c)
		{
			printf("%c", c);
			c = readByte();
		}
		printf("\n");

	// revenir au prochain symbole a afficher

		closeBinFile();
		openBinFile();

		skipData((symtab_offset + ((i + 1) * symtab_entsize)) * 8);
	}
    
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
    afficherSymbolTable();
    closeBinFile();

    return 0;
}
