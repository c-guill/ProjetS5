#include "getELF.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
// Renvoie le numero de la section du meme nom, ou -1 aucune section ne porte ce nom
//
int trouverNumSection(FILE *f,Elf32_Ehdr *ehdr, Elf32_Shdr *shdrtab, char *chaine)
{
	int i;
    unsigned char *tabC;
    char nomSection[1024];

// parcourir les section headers en verfiant si le nom correspond
	int c;
	fseek(f,shdrtab[ehdr->e_shstrndx].sh_offset,SEEK_SET);
	tabC=lireSection(f,shdrtab[ehdr->e_shstrndx]);

	for (i = 0; i < ehdr->e_shnum; i++)
	{
		c=0;
		while(tabC[shdrtab[i].sh_name+c]!='\0')
		{
			nomSection[c]=tabC[shdrtab[i].sh_name+c];
			c++;
		}
		nomSection[c]='\0';
		//printf("%s\n", nomSection);
	// stocker le string correspondant au nom de la section
	
	// comparer avec le string donne et renvoyer le numero de section si match

		if (!strcmp(nomSection, chaine))
		{
			return i;
		}

	}

	return -1;
}

//
// Afficher le contenu de la section du numero demande dans la console
//
void afficherSectionNum(FILE *f, Elf32_Ehdr *ehdr, Elf32_Shdr *shdrtab, int section_num)
{
    unsigned char *c_shstrtab, *c_sparam;
	unsigned char byte_line[16];
	unsigned char c;
	int i, j;

	Elf32_Shdr shdr_shstrtab = shdrtab[ehdr->e_shstrndx];
	Elf32_Shdr shdr_sparam = shdrtab[section_num];

	if (section_num < 0 || section_num >= ehdr->e_shnum)
	{
		fprintf(stderr, "readelf: AVERTISSEMENT: La section %d n'a pas été vidangée parce qu'inexistante !\n", section_num);
		return;
	}

	fseek(f, shdr_shstrtab.sh_offset, SEEK_SET);
	c_shstrtab = lireSection(f, shdr_shstrtab);

	if (shdr_sparam.sh_type == SHT_NULL || shdr_sparam.sh_type == SHT_NOBITS || shdr_sparam.sh_size == 0)
	{
		printf("La section « ");

		i=0;
		while(c_shstrtab[shdr_sparam.sh_name+i]!='\0')
		{
			printf("%c", c_shstrtab[shdr_sparam.sh_name+i]);
			i++;
		}
		

		printf(" » n'a pas de données à vidanger.\n");
		exit(0);
	}

	printf("\nVidange hexadécimale de la section « ");

	i=0;
	while(c_shstrtab[shdr_sparam.sh_name+i]!='\0')
	{
		printf("%c", c_shstrtab[shdr_sparam.sh_name+i]);
		i++;
	}

	printf(" » :\n");

	fseek(f, shdr_sparam.sh_offset, SEEK_SET);
	c_sparam = lireSection(f, shdr_sparam);

// afficher une note si cette section a des readressages

	for (i = 0; i < ehdr->e_shnum; i++)
	{
		if ((shdrtab[i].sh_type == SHT_REL || shdrtab[i].sh_type == SHT_RELA) && shdrtab[i].sh_info == section_num)
		{
			printf(" NOTE : Cette section a des réadressages mais ils n'ont PAS été appliqués à cette vidange.\n");
			break;
		}
	}

// vidanger la section en hexadecimal

	printf("  0x00000000 ");

	for (i = 0; i < shdr_sparam.sh_size; i++)
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
			printf("\n  0x%08x ", i);
		}
		else if (i != 0 && i % 4 == 0)
		{
			printf(" ");
		}

		c = c_sparam[i];
		byte_line[i % 16] = c;
		printf("%02x", c);
		
		if (i+1 == shdr_sparam.sh_size)
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

	return;
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	FILE *f;
	Elf32_Ehdr ehdr;
	Elf32_Shdr *shdrtab;
	char est_num;
	int c, num_section;

	if (argc != 3)
	{
        printf("usage: %s <nom ou numero section> <fichier>\n", argv[0]);
        return 1;
    }

	f=fopen(argv[2],"r");

	if(f==NULL){
		printf("Erreur d'ouverture\n");
		exit(1);
	}

	ehdr=lireHeaderElf(f);
	fseek(f, ehdr.e_shoff, SEEK_SET);
	shdrtab=lireSecHeaTable(f,ehdr);

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
		afficherSectionNum(f, &ehdr, shdrtab, stringtoInt(argv[1]));
	}
	else
	{
		num_section = trouverNumSection(f, &ehdr, shdrtab, argv[1]);

		if (num_section != -1)
		{
			afficherSectionNum(f, &ehdr, shdrtab, num_section);
		}
		else
		{
			fprintf(stderr, "readelf: AVERTISSEMENT: La section « %s » n'a pas été vidangée parce qu'inexistante !\n", argv[1]);
		}
	}

    return 0;
}
