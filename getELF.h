#ifndef __GETELF_H__
#define __GETELF_H__
#include <elf.h>
#include <stdio.h>

typedef struct {
    Elf32_Ehdr ehdr;            // en-tete ELF
    Elf32_Shdr *shdrtab;        // table d'en-tetes de section
    char *shstrtab;             // table de chaines d'en-tete de section
    char **scodetab;            // table des contenus des sections de code
    Elf32_Sym *symtab;          // table des symboles
    char *strtab;               // table de chaines
    Elf32_Rel **reltabtab;      // table des tables de reimplantations
    Elf32_Rela **relatabtab;    // table des tables de reimplantations addend
    char *arm_attrib;           // contenu de la section .ARM.attributes
} ELF_FILE;


Elf32_Ehdr lireHeaderElf(FILE* f);
Elf32_Shdr lireSectionHeader(FILE* f);
Elf32_Shdr *lireSecHeaTable(FILE* f, Elf32_Ehdr ehdr);
char *lireSection(FILE* f, Elf32_Shdr shdr);
Elf32_Sym lireSymbol(FILE* f);
Elf32_Sym *lireSymTable(FILE* f, Elf32_Shdr shdr);
Elf32_Rel lireRelocation(FILE* f);
Elf32_Rela lireRelocationA(FILE* f);
Elf32_Rel *lireRelocationTable(FILE *f,Elf32_Shdr shdr);
Elf32_Rela *lireRelocationATable(FILE *f,Elf32_Shdr shdr);
void lireRelocationTableComplete(FILE *f, Elf32_Ehdr ehdr, Elf32_Shdr *shdrtab, Elf32_Rel **rel, Elf32_Rela **rela);
ELF_FILE lireFichierELF(FILE *f);

#endif
