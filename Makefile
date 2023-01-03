CC=clang 
CFLAGS=-g -Wall -Werror

# "make all" pour generer les executables de chaque etape de la phase 1
all: fusionELF getELFHeader getRelocationTable getSymbolTable getSection getSectionHeaderTable

fusionELF: fusionELF.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

getELFHeader: getELFHeader.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

getRelocationTable: getRelocationTable.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

getSection: getSection.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

getSectionHeaderTable: getSectionHeaderTable.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

getSymbolTable: getSymbolTable.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $<

fusionELF.o: bfile.h
getELFHeader.o: bfile.h
getRelocationTable.o: bfile.h
getSection.o: bfile.h
getSymbolTable.o: bfile.h
getSectionHeaderTable.o: bfile.h
bfile.o: bfile.h bit_operations.h
bit_operations.o: bit_operations.h

clean:
	rm -f *.o fusionELF getELFHeader getRelocationTable getSection getSectionHeaderTable getSymbolTable
