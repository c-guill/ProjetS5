CC=clang 
CFLAGS=-g -Wall -Werror

# "make all" pour generer les executables de chaque etape de la phase 1
all: getELFHeader getSymbolTable

getELFHeader: getELFHeader.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

getSymbolTable: getSymbolTable.o bfile.o bit_operations.o
	$(CC) $^ -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $<

getELFHeader.o: bfile.h
getSymbolTable.o: bfile.h
bfile.o: bfile.h bit_operations.h
bit_operations.o: bit_operations.h

clean:
	rm -f *.o getELFHeader getSymbolTable
