CC=clang 
CFLAGS= -g -Wall -Werror

test: getELFHeader.o bfile.o 
	$(CC) $^ -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $<

getELFHeader.o: bfile.h
bfile.o: bfile.h

clean:
	rm -f *.o essai_pile