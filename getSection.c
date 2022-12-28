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
// Renvoie la chaine de caractere transformee en un entier
//
int stringtoInt(char *str)
{
	int v, mul, i, j;

	v = 0;
	mul = 1;
	i = 0;

	while (str[i] != '\0') i++;

	for (j = i - 1; j >= 0; j--)
	{
		v = v + (str[j] - '0') * mul;
		mul = mul * 10;
	}

	return v;
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
// Renvoie le numero de la section du meme nom, ou -1 aucune section ne porte ce nom
//
int trouverNumSection(char *str)
{
	Elf32_Word shstrtab_offset, sh_name;
	Elf32_Off e_shoff;
	Elf32_Half e_shentsize, e_shnum, e_shstrndx;
	char sh_name_str[1024];
	char c;
	int i, nb;

	openBinFile();

// recuperer infos sur les section headers

	skipData(256);
	e_shoff = readWord();
	skipData(80);
	e_shentsize = readHalf();
	e_shnum = readHalf();
	e_shstrndx = readHalf();

// recuperer l'offset de la section header string table

	closeBinFile();
	openBinFile();

	skipData((e_shoff + (e_shentsize * e_shstrndx)) * 8);

	skipData(128);
	shstrtab_offset = readWord();

// parcourir les section headers en verifiant si le nom correspond

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	for (i = 0; i < e_shnum; i++)
	{
		sh_name = readWord();

	// stocker le string correspondant au nom de la section

		closeBinFile();
		openBinFile();

		skipData((shstrtab_offset + sh_name) * 8);

		nb = 0;
		c = readByte();
		while (c)
		{
			sh_name_str[nb] = c;
			nb++;
			c = readByte();
		}
		sh_name_str[nb] = '\0';
	
	// comparer avec le string donne et renvoyer le numero de section si match

		if (!strcmp(sh_name_str, str))
		{
			closeBinFile();
			return i;
		}

	// revenir au prochain nom de section

		closeBinFile();
		openBinFile();

		skipData((e_shoff + (e_shentsize * (i+1))) * 8);
	}

	closeBinFile();
	return -1;
}

//
// Afficher le contenu de la section du numero demande dans la console
//
void afficherSectionNum(int section_num)
{
	Elf32_Word shstrtab_offset;
	Elf32_Off e_shoff;
	Elf32_Half e_shentsize, e_shnum, e_shstrndx;
	Elf32_Shdr shdr, rel_shdr;
	unsigned char byte_line[16];
	unsigned char c;
	int i, j;

	openBinFile();

// recuperer infos sur les section headers

	skipData(256);
	e_shoff = readWord();
	skipData(80);
	e_shentsize = readHalf();
	e_shnum = readHalf();
	e_shstrndx = readHalf();

// recuperer l'offset de la section header string table

	closeBinFile();
	openBinFile();

	skipData((e_shoff + (e_shentsize * e_shstrndx)) * 8);

	skipData(128);
	shstrtab_offset = readWord();

// recuperer les infos de la section demandee

	if (section_num < 0 || section_num >= e_shnum)
	{
		fprintf(stderr, "readelf: AVERTISSEMENT: La section %d n'a pas été vidangée parce qu'inexistante !\n", section_num);
		return;
	}

	closeBinFile();
	openBinFile();

	skipData((e_shoff + (e_shentsize * section_num)) * 8);
	shdr.sh_name = readWord();
	shdr.sh_type = readWord();
	skipData(64);
	shdr.sh_offset = readWord();
	shdr.sh_size = readWord();

	if (shdr.sh_type == SHT_NULL || shdr.sh_type == SHT_NOBITS || shdr.sh_size == 0)
	{
		printf("La section « ");

	// se deplacer vers l'indice du section header string table correspondant au nom de la section

		closeBinFile();
		openBinFile();

		skipData((shstrtab_offset + shdr.sh_name) * 8);

	// afficher le nom de la section

		c = readByte();
		while (c)
		{
			printf("%c", c);
			c = readByte();
		}

		printf(" » n'a pas de données à vidanger.\n");
		exit(0);
	}

	printf("\nVidange hexadécimale de la section « ");

// se deplacer vers l'indice du section header string table correspondant au nom de la section

	closeBinFile();
	openBinFile();

	skipData((shstrtab_offset + shdr.sh_name) * 8);

// afficher le nom de la section

	c = readByte();
	while (c)
	{
		printf("%c", c);
		c = readByte();
	}

	printf(" » :\n");

// afficher une note si cette section a des readressages

	closeBinFile();
	openBinFile();

	skipData(e_shoff * 8);

	for (i = 0; i < e_shnum; i++)
	{
		skipData(32);
		rel_shdr.sh_type = readWord();
		skipData(160);
		rel_shdr.sh_info = readWord();

		if ((rel_shdr.sh_type == SHT_REL || rel_shdr.sh_type == SHT_RELA) && rel_shdr.sh_info == section_num)
		{
			printf(" NOTE : Cette section a des réadressages mais ils n'ont PAS été appliqués à cette vidange.\n");
			break;
		}
		else
		{
			skipData(64);
		}
	}

// vidanger la section en hexadecimal

	closeBinFile();
	openBinFile();

	skipData(shdr.sh_offset * 8);

	printf("  0x00 ");

	for (i = 0; i < shdr.sh_size; i++)
	{
		if (i != 0 && i % 16 == 0)
		{
			printf(" ");
			for (j = 0; j < 16; j++)
			{
				if (32 <= byte_line[j] && byte_line[j] <= 126)
					printf("%c", byte_line[j]);
				else
					printf(".");
			}
			printf("\n  0x%x ", i);
		}
		else if (i != 0 && i % 4 == 0)
		{
			printf(" ");
		}

		c = readByte();
		byte_line[i % 16] = c;
		if (c < 0x10)
			printf("0%x", c);
		else
			printf("%x", c);
		
		if (i+1 == shdr.sh_size)
		{
			printf(" ");
			for (j = 0; j < (i % 16) + 1; j++)
			{
				if (32 <= byte_line[j] && byte_line[j] <= 126)
					printf("%c", byte_line[j]);
				else
					printf(".");
			}
			printf("\n");
		}
	}
	printf("\n");

	closeBinFile();

	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	char est_num;
	int c, num_section;

    if (argc != 3)
	{
        printf("usage: %s <nom ou numero section> <fichier>\n", argv[0]);
        return 1;
    }
	
	strcpy(fname, argv[2]);
	strcpy(fmode, "r");

	// determiner si numero ou nom est donne

	est_num = 1;
	c = 0;
	while (argv[1][c] != '\0')
	{
		if (argv[1][c] < '0' || argv[1][c] > '9')
		{
			est_num = 0;
			break;
		}

		c++;
	}

	if (est_num)
	{
		afficherSectionNum(stringtoInt(argv[1]));
	}
	else
	{
		num_section = trouverNumSection(argv[1]);

		if (num_section != -1)
		{
			afficherSectionNum(num_section);
		}
		else
		{
			fprintf(stderr, "readelf: AVERTISSEMENT: La section « %s » n'a pas été vidangée parce qu'inexistante !\n", argv[1]);
		}
	}

    return 0;
}
