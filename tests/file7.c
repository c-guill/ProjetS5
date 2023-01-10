/*

  Question:
	Écrire un programme en C ou C++ qui copie le contenu d'un fichier dans un
	autre. Le programme prend deux paramètres : le fichier source et le fichier
	destination.

  Source:
	Object Systems Group (progsys), Université de Genève.

  Correction:
	Mohamed Lokbani
	de quelques warnings relatifs à fprintf, read, write.
	ajout de stdio et unistd
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  int fdSource, fdDestin;	/* Descripteurs de fichiers */
  char buffer[10000];	/* Un buffer pour contenir les copies */
  int nbRead;		/* Variable pour stocker le nombre d'octets lus */
  
  if (argc != 3) {
    printf("Usage: %s fichier_source fichier_destin\n", argv[0]);
    exit(0);
  }
  
  fdSource = open(argv[1], O_RDONLY);
  
  if (fdSource == -1) {
    char msg[100];
    sprintf(msg, "Impossible d'ouvrir le fichier '%s'", argv[1]);
    perror(msg);
    exit(0);
  }
  
  fdDestin = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0600);
  
  if (fdDestin == -1) {
    char reply;
    printf("Le fichier '%s' existe. Voulez-vous l'ecraser? (y/n) ", argv[2]);
    scanf("%c", &reply);
    if (reply != 'y') {
      exit(0);
    }
    
    fdDestin = open(argv[2], O_WRONLY | O_CREAT, 0600);
  }
  
  nbRead = read(fdSource, buffer, 10000);
  while (nbRead > 0) {
    write(fdDestin, buffer, nbRead);
    nbRead = read(fdSource, buffer, 10000);
  }
  return 0;
}
