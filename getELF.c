#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "getELF.h"
#include "util.h"


Elf32_Ehdr lireHeaderElf(FILE* f){
    Elf32_Ehdr ehdr;
    fread(&ehdr.e_ident,1,16,f);
    fread(&ehdr.e_type,2,1,f);
    fread(&ehdr.e_machine,2,1,f);
    fread(&ehdr.e_version,4,1,f);
    fread(&ehdr.e_entry,4,1,f);
    fread(&ehdr.e_phoff,4,1,f);
    fread(&ehdr.e_shoff,4,1,f);
    fread(&ehdr.e_flags,4,1,f);
    fread(&ehdr.e_ehsize,2,1,f);
    fread(&ehdr.e_phentsize,2,1,f);
    fread(&ehdr.e_phnum,2,1,f);
    fread(&ehdr.e_shentsize,2,1,f);
    fread(&ehdr.e_shnum,2,1,f);
    fread(&ehdr.e_shstrndx,2,1,f);
    if(!is_big_endian()){
        ehdr.e_type = reverse_2(ehdr.e_type);
        ehdr.e_machine = reverse_2(ehdr.e_machine);
        ehdr.e_version = reverse_4(ehdr.e_version);
        ehdr.e_entry = reverse_4(ehdr.e_entry);
        ehdr.e_phoff = reverse_4(ehdr.e_phoff);
        ehdr.e_shoff = reverse_4(ehdr.e_shoff);
        ehdr.e_flags = reverse_4(ehdr.e_flags);
        ehdr.e_ehsize = reverse_2(ehdr.e_ehsize);
        ehdr.e_phentsize = reverse_2(ehdr.e_phentsize);
        ehdr.e_phnum = reverse_2(ehdr.e_phnum);
        ehdr.e_shentsize = reverse_2(ehdr.e_shentsize);
        ehdr.e_shnum = reverse_2(ehdr.e_shnum);
        ehdr.e_shstrndx = reverse_2(ehdr.e_shstrndx);
    }
    return ehdr;
}

Elf32_Shdr lireSectionHeader(FILE* f){
    Elf32_Shdr shdr;
    fread(&shdr.sh_name,4,1,f);
    fread(&shdr.sh_type,4,1,f);
    fread(&shdr.sh_flags,4,1,f);
    fread(&shdr.sh_addr,4,1,f);
    fread(&shdr.sh_offset,4,1,f);
    fread(&shdr.sh_size,4,1,f);
    fread(&shdr.sh_link,4,1,f);
    fread(&shdr.sh_info,4,1,f);
    fread(&shdr.sh_addralign,4,1,f);
    fread(&shdr.sh_entsize,4,1,f);
    if(!is_big_endian()){
        shdr.sh_name = reverse_4(shdr.sh_name);
        shdr.sh_type = reverse_4(shdr.sh_type);
        shdr.sh_flags = reverse_4(shdr.sh_flags);
        shdr.sh_addr = reverse_4(shdr.sh_addr);
        shdr.sh_offset = reverse_4(shdr.sh_offset);
        shdr.sh_size = reverse_4(shdr.sh_size);
        shdr.sh_link = reverse_4(shdr.sh_link);
        shdr.sh_info = reverse_4(shdr.sh_info);
        shdr.sh_addralign = reverse_4(shdr.sh_addralign);
        shdr.sh_entsize = reverse_4(shdr.sh_entsize);

    }

    return shdr;
}
Elf32_Shdr *lireSecHeaTable(FILE *f,Elf32_Ehdr ehdr){
    Elf32_Shdr *shdr=malloc(sizeof(Elf32_Shdr)*ehdr.e_shnum);
    if(shdr==NULL){
        printf("Erreur d'allocation du tableau de section Header\n");
        exit(1);
    }
    for(int i=0;i<ehdr.e_shnum;i++){
        shdr[i]=lireSectionHeader(f);


    }
    return shdr;
}

char *lireSectionName(FILE *f, Elf32_Shdr shdr){
    char *c=malloc(sizeof(char)*shdr.sh_size);
    int i;
    if(c==NULL){
        printf("Erreur d'allocation du tableau de char\n");
        exit(1);
    }
    for (i = 0; i < shdr.sh_size; ++i) {
        fread(&c[i],1,1,f);
    }
    c[i] = '\0';
    return c;
}

Elf32_Sym lireSymbol(FILE* f){
    Elf32_Sym sym;
    fread(&sym.st_name,4,1,f);
    fread(&sym.st_value,4,1,f);
    fread(&sym.st_size,4,1,f);
    fread(&sym.st_info,1,1,f);
    fread(&sym.st_other,1,1,f);
    fread(&sym.st_shndx,2,1,f);

    return sym;
}

Elf32_Rel lireRelocation(FILE* f){
    Elf32_Rel rel;
    fread(&rel.r_offset,4,1,f);
    fread(&rel.r_info,4,1,f);

    return rel;
}

Elf32_Rela lireRelocationA(FILE* f){
    Elf32_Rela rela;
    fread(&rela.r_offset,4,1,f);
    fread(&rela.r_info,4,1,f);
    fread(&rela.r_addend,4,1,f);

    return rela;
}