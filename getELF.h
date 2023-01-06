#ifndef __GETELF_H__
#define __GETELF_H__
#include <elf.h>
#include <stdio.h>

Elf32_Ehdr lireHeaderElf(FILE* f);
Elf32_Shdr lireSectionHeader(FILE* f);
Elf32_Shdr *lireSecHeaTable(FILE* f, Elf32_Ehdr ehdr);
Elf32_Sym *lireSymTable(FILE* f, Elf32_Shdr shdr);
Elf32_Sym lireSymbol(FILE* f);
Elf32_Rel lireRelocation(FILE* f);
Elf32_Rela lireRelocationA(FILE* f);
Elf32_Rel *lireRelocationTable(FILE *f,Elf32_Shdr shdr);
Elf32_Rela *lireRelocationATable(FILE *f,Elf32_Shdr shdr);
void lireRelocationTableComplete(FILE *f, Elf32_Ehdr ehdr, Elf32_Shdr *shdrtab, Elf32_Rel **rel, Elf32_Rela **rela);
char *lireSection(FILE* f, Elf32_Shdr shdr);



#endif
