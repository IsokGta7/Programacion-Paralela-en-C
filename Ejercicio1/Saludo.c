#include<stdio.h>
#include<mpi.h>

void main ( ){
  int rank, size, ind1,ind2;
  MPI_Init (NULL,NULL);

  ind1 = MPI_Comm_size (MPI_COMM_WORLD,&size ); // numero de procesos
  ind2 = MPI_Comm_rank (MPI_COMM_WORLD,&rank ); // Identificador de proceso

  if( ind1==MPI_SUCCESS && ind2==MPI_SUCCESS)
    printf( "Hola, mundo de C+MPI, desde el proceso %d de %d\n" ,rank, size);
  else
    printf("Ocurrió un error :O ");

  MPI_Finalize ( ) ;
  return;
}
