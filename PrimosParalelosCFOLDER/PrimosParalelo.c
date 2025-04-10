#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// -----------------------------------------------------------------------------
int BLOCK_LOW(int id, int p, int n){
  return (id * n / p);
}
int BLOCK_HIGH(int id, int p, int n){
  return BLOCK_LOW(id + 1, p, n) - 1;
}
int BLOCK_SIZE(int id, int p, int n){
  return BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1;
}
// -----------------------------------------------------------------------------
float time_diff(struct timeval *start, struct timeval *end){
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}
// -----------------------------------------------------------------------------
int EsPrimo(int n)
{
    int i;
    // Corner cases
    if (n == 1)  return 0;
    if (n <= 3)  return 1;

    // This is checked so that we can skip
    // middle five numbers in below loop
    if (n%2 == 0 || n%3 == 0) return 0;

    for (i=5; i*i<=n; i+=6)
        if (n%i == 0 || n%(i+2) == 0)
           return 0;

    return 1;
}
// -----------------------------------------------------------------------------
int main(int argc, char **argv){
  MPI_Init(&argc, &argv);
  int id, i,  np, n, aux, inicio, fin, num_elems_rango, root = 0, total;
  double t1, t2;

  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  if(argc!=2){
    if(id==0) printf("se requiere que se envien 2 argumentos\n");
    MPI_Finalize();
    return 1;
  }

  t1 = MPI_Wtime(); // <-------------------- tiempo1
  n = atoi(argv[1]);

  inicio = BLOCK_LOW(id, np, n) + 1; // para que empiece en 1
  if(id==0) inicio=2;

  fin    = BLOCK_HIGH(id, np, n) + 1;
  num_elems_rango = BLOCK_SIZE(id, np, n);

  int cuantos = 0;
  MPI_Barrier(MPI_COMM_WORLD);


  for (i = inicio; i <= fin; i++)
      cuantos += EsPrimo(i);

  MPI_Send(&cuantos, 1, MPI_INT, root, 9, MPI_COMM_WORLD);

  if (id == root) {
    total = 0;
    for (i = 0; i < np; i++) {
      MPI_Recv(&cuantos, 1, MPI_INT, i, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      total = total + cuantos;
    }
    t2 = MPI_Wtime(); // <----------------------- tiempo2

    float tiempo = t2-t1;
    printf("%d, %d, %f\n", np, total, tiempo);
  }

  MPI_Finalize();
  return 0;
}
