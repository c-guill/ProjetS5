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
void openBinFile(char *name, char *mode)
{
	f = fopen(name, mode);
	if (!f)
	{
        printf("ERREUR: le fichier n'a pas pu être ouvert\n");
        exit(1);
    }

	bf = bstart(f, mode);
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
void readData(BFILE *file, int T[], int size) {
    for (int i = 0; i < size; i++) {
        T[i] = bitread(file);
    }
    bintoHex(T,size);

	nbits += size;

	return;
}

//
// Ignorer "size" bits du fichier
//
void skipData(BFILE *file, int size) {
    for (int i = 0; i < size; i++) {
        bitread(file);
    }

	nbits += size;

	return;
}

//
// Lire 32 bits du fichier et renvoyer la valeur lue
//
uint32_t readWord(BFILE *file)
{
	uint32_t val;
	int T[32];
	int i, y;

	readData(file, T, 32);
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
uint16_t readHalf(BFILE *file)
{
	uint16_t val;
	int T[16];
	int i, y;

	readData(file, T, 16);
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
// Afficher la table des symboles du fichier dans la console
//
// TODO :
//	- verifier que le fichier est un ELF au bon format
//	- gerer les fichiers sans section headers, sans table des symboles etc.. (robustesse)
//
void afficherSymbolTable(void)
{
	Elf32_Word e_shtype, sh_size;
	Elf32_Off e_shoff, sh_offset;
	Elf32_Half e_shentsize, e_shnum;
	int i;

// recuperer infos dans le ELF header
	
	skipData(bf, 256);
	e_shoff = readWord(bf);
	printf("offset tableau en-tete section : %d\n", e_shoff);
	skipData(bf, 80);
	e_shentsize = readHalf(bf);
	printf("taille en-tete section : %d\n", e_shentsize);
	e_shnum = readHalf(bf);
	printf("nombre d'en-tetes section : %d\n", e_shnum);

// se deplacer vers le debut du tableau de section headers

	closeBinFile();
	openBinFile(fname, fmode);

	skipData(bf, e_shoff * 8);

// parcourir les section headers a la recherche de la table des symboles

	if (e_shnum == 0)
	{
		printf("Le fichier ne possede aucune en-tete de section\n");
		return;
	}

	skipData(bf, 32);
	e_shtype = readWord(bf);
	printf("type section : %d\n", e_shtype);

	i = 0;
	while (e_shtype != SHT_SYMTAB)
	{
		i++;

		if (i == e_shnum)
		{
			printf("Le fichier ne possede pas de table des symboles\n");
			return;
		}

		skipData(bf, e_shentsize * 8 - 32);
		e_shtype = readWord(bf);
		printf("type section : %d\n", e_shtype);
	}

// recuperer sh_offset dans le section header

	skipData(bf, 64);
	sh_offset = readWord(bf);
	printf("offset table des symboles : 0x%x\n", sh_offset);
	sh_size = readWord(bf);
	printf("taille table des symboles : 0x%x\n", sh_size);

// lire la table des symboles de taille sh_size

	closeBinFile();
	openBinFile(fname, fmode);
	
	skipData(bf, sh_offset * 8);
    
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

    openBinFile(fname, fmode);
    afficherSymbolTable();
    closeBinFile();

    return 0;
}
