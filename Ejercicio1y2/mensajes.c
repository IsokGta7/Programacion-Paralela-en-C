
#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include<string.h>
void main ( ) {
  MPI_Init(NULL, NULL);

  int rank,size,i, root=0;
  char mensaje[50];

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  sprintf ( mensaje,"Hola proceso %d, un saludo desde el proceso %d", root, rank );
  MPI_Send( mensaje, strlen(mensaje)+1, MPI_CHAR, root, 9, MPI_COMM_WORLD );

  if( rank == root ){
    printf( "Soy el proceso %d y recibí los mensajes:\n", root );
    for( i = 0 ; i < size ; i++ ){
      MPI_Recv( mensaje, 50, MPI_CHAR, i, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf( "<%s>\n", mensaje );
    }
  }

  MPI_Finalize();
  return ;
}