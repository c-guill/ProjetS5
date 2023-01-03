#include <elf.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfile.h"

#define FILE1	0
#define FILE2	1
#define FILER	2


FILE *flist[3];   		// pointeurs fichier objet
BFILE *bflist[3]; 		// pointeurs acces bit a bit fichier objet
char fnames[3][1024]; 	// noms des fichiers objet
char fmodes[3][2]; 		// modes d'ouverture des fichiers objet


//
// Ouvrir un fichier binaire en acces bit a bit
//
void openBinFile(int fnum)
{
	flist[fnum] = fopen(fnames[fnum], fmodes[fnum]);
	if (!flist[fnum])
	{
        printf("ERREUR: le fichier %s n'a pas pu être ouvert\n", fnames[fnum]);
        exit(1);
    }
	
	bflist[fnum] = bstart(flist[fnum], fmodes[fnum]);
	if (!bflist[fnum])
	{
		printf("ERREUR: impossible d'ouvrir le fichier %s en acces bit a bit\n", fnames[fnum]);
		fclose(flist[fnum]);
		exit(1);
	}

	return;
}

//
// Fermer un fichier binaire en acces bit a bit
//
void closeBinFile(int fnum)
{
	if (bflist[fnum])
	{
		bstop(bflist[fnum]);
	}
	
	fclose(flist[fnum]);

	return;
}

//
// Renvoyer le contenu en decimal de T entre les bornes "start" et "end" (incluses)
//
int hextoDec(int T[], int start, int end)
{
	int j = 0;
    int r = 0;

    for (int i = end; i >= start; i--)
	{
        r += T[i] * (int) pow(16,j);
        j++;
    }

    return r;
}

//
// Convertir "size" bits du tableau T en hexadecimal
//
void bintoHex(int T[], int size)
{
	int dec, y;

    for (int i = 0; i < size/4; i++)
	{
        dec = 0;
        y = 0;
        for (int j = 3; j >= 0; j--)
		{
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
void readData(int fnum, int T[], int size)
{
    for (int i = 0; i < size; i++)
	{
        T[i] = bitread(bflist[fnum]);
    }
    bintoHex(T,size);

	return;
}

//
// Ignorer "size" bits du fichier
//
void skipData(int fnum, int size)
{
    for (int i = 0; i < size; i++)
	{
        bitread(bflist[fnum]);
    }

	return;
}

//
// Lire 32 bits du fichier et renvoyer la valeur lue
//
uint32_t readWord(int fnum)
{
	uint32_t val;
	int T[32];
	int i, y;

	readData(fnum, T, 32);
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
uint16_t readHalf(int fnum)
{
	uint16_t val;
	int T[16];
	int i, y;

	readData(fnum, T, 16);
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
uint8_t readByte(int fnum)
{
	uint8_t val;
	int T[8];
	int i, y;

	readData(fnum, T, 8);
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
// Afficher dans la console la fusion des sections de code
//
// TODO: memoriser les changements de numero des sections du second fichier
//
void afficherFusionSectionsCode(void)
{
	Elf32_Ehdr f1_ehdr, f2_ehdr;
	Elf32_Shdr f1_shdr, f2_shdr;
	Elf32_Off *concat_off;  // indice i du tableau correspond a l'offset de concat (en octets) de la section n°i
	Elf32_Off f1_shstrtaboff, f2_shstrtaboff;
	char f1_shnamestr[1024], f2_shnamestr[1024];
	char c;
	int i, j, n;

// recuperer infos dans le ELF header

	skipData(FILE1, 256);
	skipData(FILE2, 256);
	f1_ehdr.e_shoff = readWord(FILE1);
	f2_ehdr.e_shoff = readWord(FILE2);
	skipData(FILE1, 80);
	skipData(FILE2, 80);
	f1_ehdr.e_shentsize = readHalf(FILE1);
	f2_ehdr.e_shentsize = readHalf(FILE2);
	f1_ehdr.e_shnum = readHalf(FILE1);
	f2_ehdr.e_shnum = readHalf(FILE2);
	f1_ehdr.e_shstrndx = readHalf(FILE1);
	f2_ehdr.e_shstrndx = readHalf(FILE2);

// initialiser la table d'offsets de concat des sections du deuxieme fichier

	concat_off = (Elf32_Off *)malloc(f2_ehdr.e_shnum * sizeof(Elf32_Off));
	memset(concat_off, 0, f2_ehdr.e_shnum * sizeof(Elf32_Off));

// recuperer l'offset de la section header string table pour chaque fichier

	closeBinFile(FILE1);
	closeBinFile(FILE2);
	openBinFile(FILE1);
	openBinFile(FILE2);
	skipData(FILE1, (f1_ehdr.e_shoff + (f1_ehdr.e_shentsize * f1_ehdr.e_shstrndx)) * 8 + 128);
	skipData(FILE2, (f2_ehdr.e_shoff + (f2_ehdr.e_shentsize * f2_ehdr.e_shstrndx)) * 8 + 128);
	f1_shstrtaboff = readWord(FILE1);
	f2_shstrtaboff = readWord(FILE2);

// afficher sections de code du premier fichier

	closeBinFile(FILE1);
	openBinFile(FILE1);
	skipData(FILE1, f1_ehdr.e_shoff * 8);

	for (i = 0; i < f1_ehdr.e_shnum; i++)
	{
		f1_shdr.sh_name = readWord(FILE1);
		f1_shdr.sh_type = readWord(FILE1);
		skipData(FILE1, 64);
		f1_shdr.sh_offset = readWord(FILE1);
		f1_shdr.sh_size = readWord(FILE1);

		if (f1_shdr.sh_type != SHT_PROGBITS)
		{
			closeBinFile(FILE1);
			openBinFile(FILE1);
			skipData(FILE1, (f1_ehdr.e_shoff + (f1_ehdr.e_shentsize * (i+1))) * 8);
			continue;
		}

	// stocker le string correspondant au nom de la section

		closeBinFile(FILE1);
		openBinFile(FILE1);
		skipData(FILE1, (f1_shstrtaboff + f1_shdr.sh_name) * 8);

		n = 0;
		c = readByte(FILE1);
		while (c)
		{
			f1_shnamestr[n] = c;
			n++;
			c = readByte(FILE1);
		}
		f1_shnamestr[n] = '\0';

	// afficher le contenu de la section

		closeBinFile(FILE1);
		openBinFile(FILE1);
		skipData(FILE1, f1_shdr.sh_offset * 8);

		for (j = 0; j < f1_shdr.sh_size * 8; j++)
		{
			printf("%d", bitread(bflist[FILE1]));
			//bitwrite(bflist[FILER], bitread(bflist[FILE1]));
		}
		//printf("FILE1 (section n°%d / nom %s / type %d / taille 0x%x)", i, f1_shnamestr, f1_shdr.sh_type, f1_shdr.sh_size);

	// si section correspondante dans le second fichier, concatener son contenu

		closeBinFile(FILE2);
		openBinFile(FILE2);
		skipData(FILE2, f2_ehdr.e_shoff * 8);

		for (j = 0; j < f2_ehdr.e_shnum; j++)
		{
			f2_shdr.sh_name = readWord(FILE2);
			f2_shdr.sh_type = readWord(FILE2);
			skipData(FILE2, 64);
			f2_shdr.sh_offset = readWord(FILE2);
			f2_shdr.sh_size = readWord(FILE2);

			if (f2_shdr.sh_type != SHT_PROGBITS)
			{
				closeBinFile(FILE2);
				openBinFile(FILE2);
				skipData(FILE2, (f2_ehdr.e_shoff + (f2_ehdr.e_shentsize * (j+1))) * 8);
				continue;
			}

			closeBinFile(FILE2);
			openBinFile(FILE2);
			skipData(FILE2, (f2_shstrtaboff + f2_shdr.sh_name) * 8);

			n = 0;
			c = readByte(FILE2);
			while (c)
			{
				f2_shnamestr[n] = c;
				n++;
				c = readByte(FILE2);
			}
			f2_shnamestr[n] = '\0';
			
			if (!strcmp(f1_shnamestr, f2_shnamestr))
			{
				concat_off[j] = f1_shdr.sh_size;
				closeBinFile(FILE2);
				openBinFile(FILE2);
				skipData(FILE2, f2_shdr.sh_offset * 8);

				//printf("\n+\n");

				for (j = 0; j < f2_shdr.sh_size * 8; j++)
				{
					printf("%d", bitread(bflist[FILE2]));
					//bitwrite(bflist[FILER], bitread(bflist[FILE2]));
				}
				//printf(" + FILE2 (section n°%d / nom %s / type %d / offset concat 0x%x)", j, f2_shnamestr, f2_shdr.sh_type, concat_off[j]);
				break;
			}

			closeBinFile(FILE2);
			openBinFile(FILE2);
			skipData(FILE2, (f2_ehdr.e_shoff + (f2_ehdr.e_shentsize * (j+1))) * 8);
		}

		//printf("\n\n");

	// revenir au prochain nom de section

		closeBinFile(FILE1);
		openBinFile(FILE1);

		skipData(FILE1, (f1_ehdr.e_shoff + (f1_ehdr.e_shentsize * (i+1))) * 8);
	}

// parcourir les sections de code du deuxieme fichier

	closeBinFile(FILE2);
	openBinFile(FILE2);
	skipData(FILE2, f2_ehdr.e_shoff * 8);

	for (i = 0; i < f2_ehdr.e_shnum; i++)
	{
		f2_shdr.sh_name = readWord(FILE2);
		f2_shdr.sh_type = readWord(FILE2);
		skipData(FILE2, 64);
		f2_shdr.sh_offset = readWord(FILE2);
		f2_shdr.sh_size = readWord(FILE2);

		if (f2_shdr.sh_type != SHT_PROGBITS)
		{
			closeBinFile(FILE2);
			openBinFile(FILE2);
			skipData(FILE2, (f2_ehdr.e_shoff + (f2_ehdr.e_shentsize * (i+1))) * 8);
			continue;
		}

		closeBinFile(FILE2);
		openBinFile(FILE2);
		skipData(FILE2, (f2_shstrtaboff + f2_shdr.sh_name) * 8);

		n = 0;
		c = readByte(FILE2);
		while (c)
		{
			f2_shnamestr[n] = c;
			n++;
			c = readByte(FILE2);
		}
		f2_shnamestr[n] = '\0';

	// si pas de section correspondante dans le premier fichier, afficher son contenu

		closeBinFile(FILE1);
		openBinFile(FILE1);
		skipData(FILE1, f1_ehdr.e_shoff * 8);

		for (j = 0; j < f1_ehdr.e_shnum; j++)
		{
			f1_shdr.sh_name = readWord(FILE1);
			f1_shdr.sh_type = readWord(FILE1);

			if (f1_shdr.sh_type != SHT_PROGBITS)
			{
				closeBinFile(FILE1);
				openBinFile(FILE1);
				skipData(FILE1, (f1_ehdr.e_shoff + (f1_ehdr.e_shentsize * (j+1))) * 8);
				continue;
			}

			closeBinFile(FILE1);
			openBinFile(FILE1);
			skipData(FILE1, (f1_shstrtaboff + f1_shdr.sh_name) * 8);

			n = 0;
			c = readByte(FILE1);
			while (c)
			{
				f1_shnamestr[n] = c;
				n++;
				c = readByte(FILE1);
			}
			f1_shnamestr[n] = '\0';

			if (!strcmp(f2_shnamestr, f1_shnamestr))
			{
				break;
			}

			closeBinFile(FILE1);
			openBinFile(FILE1);
			skipData(FILE1, (f1_ehdr.e_shoff + (f1_ehdr.e_shentsize * (j+1))) * 8);
		}

		if (j == f1_ehdr.e_shnum)
		{
			closeBinFile(FILE2);
			openBinFile(FILE2);
			skipData(FILE2, f2_shdr.sh_offset * 8);

			for (j = 0; j < f2_shdr.sh_size * 8; j++)
			{
				printf("%d", bitread(bflist[FILE2]));
				//bitwrite(bflist[FILER], bitread(bflist[FILE2]));
			}
			//printf("FILE2 (section n°%d / nom %s / type %d / offset concat 0x%x)\n", i, f2_shnamestr, f2_shdr.sh_type, concat_off[i]);
		}

	// revenir au prochain nom de section

		closeBinFile(FILE2);
		openBinFile(FILE2);
		skipData(FILE2, (f2_ehdr.e_shoff + (f2_ehdr.e_shentsize * (i+1))) * 8);
	}

	printf("\n");

// TEST: affichage de la table des offsets de concat
/*
	printf("\n[DEBUG] Table d'offsets de concat des sections du second fichier :\n");
	for (i = 0; i < f2_ehdr.e_shnum; i++)
	{
		printf("section n°%d: 0x%x\n", i, concat_off[i]);
	}
*/	
	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    if (argc != 3)
	{
        printf("usage: %s <fichier1> <fichier2>\n", argv[0]);
        return 1;
    }
	
	strcpy(fnames[FILE1], argv[1]);
	strcpy(fmodes[FILE1], "r");
	strcpy(fnames[FILE2], argv[2]);
	strcpy(fmodes[FILE2], "r");

    openBinFile(FILE1);
	openBinFile(FILE2);
    afficherFusionSectionsCode();
    closeBinFile(FILE1);
    closeBinFile(FILE2);

    return 0;
}
