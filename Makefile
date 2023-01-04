CC=clang 
CFLAGS=-g -Wall

# "make all" pour generer les executables de chaque etape de la phase 1
all: getELFHeader getSectionHeaderTable #getSymbolTable getSection getRelocationTable


getELFHeader: getELFHeader.o util.o getELF.o
	$(CC) $^ -o $@

#getRelocationTable: getRelocationTable.o  util.o getELF.o
	#$(CC) $^ -o $@

# getSection: getSection.o  util.o getELF.o
# 	$(CC) $^ -o $@

getSectionHeaderTable: getSectionHeaderTable.o  util.o getELF.o
	$(CC) $^ -o $@

# getSymbolTable: getSymbolTable.o  util.o getELF.o
# 	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

getELFHeader.o: getELF.h util.h
#getRelocationTable.o: getELF.h util.h
#getSection.o:  getELF.h util.h
#getSymbolTable.o:  getELF.h util.h
getSectionHeaderTable.o: getELF.h util.h
getELF.o:util.h



clean:
	rm -f *.o getELFHeader getRelocationTable getSection getSectionHeaderTable getSymbolTable
