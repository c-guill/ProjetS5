#ifndef __GETELF_H__
#define __GETELF_H__
#include <elf.h>

Elf32_Ehdr lireHeaderElf(FILE* f);
Elf32_Shdr lireSectionHeader(FILE* f);
Elf32_Sym lireSymbol(FILE* f);
Elf32_Rel lireRelocation(FILE* f);
Elf32_Rela lireRelocationA(FILE* f);


#endif