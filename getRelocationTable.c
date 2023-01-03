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
// Afficher une section de relogement du fichier dans la console
//
void afficherRelocationSection(Elf32_Shdr *rel_shdr, Elf32_Shdr *symtab_shdr, Elf32_Off sh_off)
{
	Elf32_Word r_info;
	Elf32_Addr r_offset;
	Elf32_Sword r_addend;
	Elf32_Sym sym;
	int i, j;

// se deplacer au debut de la section de relogement

	closeBinFile();
	openBinFile();

	skipData(rel_shdr->sh_offset * 8);

// afficher la section de relogement

	printf("Décalage Info Type Val.-sym Noms-symboles\n");
	
	for (i = 0; i < rel_shdr->sh_size / rel_shdr->sh_entsize; i++)
	{
		r_offset = readWord();
		r_info = readWord();
		if (rel_shdr->sh_type == SHT_RELA)
			r_addend = readWord();

		printf("%x %x ", r_offset, r_info);

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

		// afficher la valeur et le nom du symbole a reloger

		closeBinFile();
		openBinFile();

		skipData(symtab_shdr->sh_offset * 8);

		for (j = 0; j < ELF32_R_SYM(r_info); j++)
		{
			skipData(symtab_shdr->sh_entsize * 8);
		}

		sym.st_name = readWord();
		sym.st_value = readWord();

		printf("%x \n", sym.st_value);

		closeBinFile();
		openBinFile();

		skipData(rel_shdr->sh_link * 8);

		// revenir a la prochaine entree de la section de relogement
	}

	return;
}

//
// Afficher la table des relogements du fichier dans la console
//
void afficherRelocationTable(void)
{
	Elf32_Word r_info, sh_name;
	Elf32_Off e_shoff, strtab_offset, shstrtab_offset;
	Elf32_Addr r_offset;
	Elf32_Sword r_addend;
	Elf32_Half e_shentsize, e_shnum, e_shstrndx;
	Elf32_Shdr rel_shdr, symtab_shdr;
	Elf32_Sym sym;
	char c;
	int i, j, k;

// recuperer infos sur les section headers
	
	skipData(256);
	e_shoff = readWord();
	skipData(80);
	e_shentsize = readHalf();
	e_shnum = readHalf();
	e_shstrndx = readHalf();

// recuperer infos sur la table des symboles

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	if (e_shnum == 0)
	{
		printf("Le fichier ne possede aucune en-tete de section\n");
		return;
	}

	skipData(32);
	symtab_shdr.sh_type = readWord();

	i = 0;
	while (symtab_shdr.sh_type != SHT_SYMTAB)
	{
		i++;

		if (i == e_shnum)
		{
			printf("Le fichier ne possede pas de table des symboles\n");
			return;
		}

		skipData(e_shentsize * 8 - 32);
		symtab_shdr.sh_type = readWord();
	}
	
	skipData(64);
	symtab_shdr.sh_offset = readWord();
	skipData(32);
	symtab_shdr.sh_link = readWord();
	skipData(64);
	symtab_shdr.sh_entsize = readWord();

// recuperer l'offset de la string table contenant le nom des symboles

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	for (i = 0; i < symtab_shdr.sh_link; i++)
	{
		skipData(e_shentsize * 8);
	}

	skipData(128);
	strtab_offset = readWord();

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

// parcourir les section headers a la recherche de sections de relogements

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	for (i = 0; i < e_shnum; i++)
	{
		rel_shdr.sh_name = readWord();
		rel_shdr.sh_type = readWord();

		if (rel_shdr.sh_type == SHT_REL || rel_shdr.sh_type == SHT_RELA)
		{
		// recuperer infos sur la section de relogement

			skipData(64);
			rel_shdr.sh_offset = readWord();
			rel_shdr.sh_size = readWord();
			rel_shdr.sh_link = readWord();
			skipData(64);
			rel_shdr.sh_entsize = readWord();
			
			printf("\nSection de réadressage '");

		// afficher infos sur la section de relogement

			closeBinFile();
			openBinFile();

			skipData((shstrtab_offset + rel_shdr.sh_name) * 8);

			c = readByte();
			while (c)
			{
				printf("%c", c);
				c = readByte();
			}

			printf("' à l'adresse de décalage 0x%x contient %d ", rel_shdr.sh_offset, rel_shdr.sh_size / rel_shdr.sh_entsize);
			if (rel_shdr.sh_size / rel_shdr.sh_entsize == 1)
				printf("entrée:\n");
			else
				printf("entrées:\n");
		
		// afficher le contenu de la section de relogement

			closeBinFile();
			openBinFile();

			skipData(rel_shdr.sh_offset * 8);

			printf("Décalage Info Type Val.-sym Noms-symboles\n");
	
			for (j = 0; j < rel_shdr.sh_size / rel_shdr.sh_entsize; j++)
			{
			// afficher infos sur le relogement

				r_offset = readWord();
				r_info = readWord();
				if (rel_shdr.sh_type == SHT_RELA)
					r_addend = readWord();

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
			// QUAND UN CERTAIN TYPE OU ST_NAME VAUT 0, AFFICHER UN NOM DE SECTION
			// OU EST-CE QUE L'ON RETROUVE LE NOM DE SECTION A AFFICHER ?

				closeBinFile();
				openBinFile();

				skipData(symtab_shdr.sh_offset * 8);

				if (ELF32_R_SYM(r_info) == STN_UNDEF)
				{
					printf("symbole non defini\n");
				}
				else
				{
					for (k = 0; k < ELF32_R_SYM(r_info); k++)
					{
						skipData(symtab_shdr.sh_entsize * 8);
					}

					sym.st_name = readWord();
					sym.st_value = readWord();
					skipData(32);
					sym.st_info = readByte();
					skipData(8);
					sym.st_shndx = readHalf();

					printf("%08x ", sym.st_value);

					closeBinFile();
					openBinFile();

					if (ELF32_ST_TYPE(sym.st_info) == STT_SECTION)
					{
						closeBinFile();
						openBinFile();

						skipData((e_shoff + (e_shentsize * sym.st_shndx)) * 8);
						sh_name = readWord();

						closeBinFile();
						openBinFile();

						skipData((shstrtab_offset + sh_name) * 8);
					}
					else
					{
						skipData((strtab_offset + sym.st_name) * 8);
					}

					c = readByte();
					while (c)
					{
						printf("%c", c);
						c = readByte();
					}
					printf("\n");
				}

			// revenir a la prochaine entree a afficher

				closeBinFile();
				openBinFile();

				skipData(rel_shdr.sh_offset * 8);
				skipData(rel_shdr.sh_entsize * (j+1) * 8);
			}

		// revenir a la prochaine section de relogement a afficher

			closeBinFile();
			openBinFile();

			skipData((e_shoff + ((i + 1) * e_shentsize)) * 8);
		}
		else
		{
		// ce n'est pas une section de relogement, la sauter

			skipData(256);
		}
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
    afficherRelocationTable();
    closeBinFile();

    return 0;
}
