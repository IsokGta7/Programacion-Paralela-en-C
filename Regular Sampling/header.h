#include <stdlib.h>
#include "mpi.h"
#include <stdio.h>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n)(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

int compare(const void *_a, const void *_b) {
  int *a, *b;

  a = (int *) _a;
  b = (int *) _b;
  return (*a - *b);
}

//  ■■■■■■■■■■
int read_array(char* fname, int **arr, int np) {
  FILE *myFile;
  unsigned int i, n, chunksize, faltantes;

  myFile = fopen(fname, "r");
  if(!myFile){
    printf("ERROR: No se pudo abrir el archivo %s",fname);
    MPI_Abort(MPI_COMM_WORLD, 99);
  }

  fscanf(myFile, "%i\n", &n); // numero de datoss a leer

  *arr = (int *) malloc( n * sizeof(int) );
  if (*arr == NULL) {
    printf("Memoria insuficiente\n");
    MPI_Abort(MPI_COMM_WORLD, 99);
  }

  for ( i=0; i < n; i++)
    fscanf(myFile, "%i\n", (*arr)+i);

  return n;
}

int* mezcla( int *arr1, int n1, int *arr2, int n2 ){
  int *auxiliar;
  int i = 0, j = 0, k = 0;

  auxiliar = (int *) malloc( (n1+n2)*sizeof(int));
  if (auxiliar == NULL) {
    printf("Memoria insuficiente (auxiliar)\n");
    return NULL;
  }

  while (i < n1 && j < n2) {
    if (arr1[i] <= arr2[j]) {
      auxiliar[k] = arr1[i];
      i++;
    } else {
      auxiliar[k] = arr2[j];
      j++;
    }
    k++;
  }

  while (i < n1) {
    auxiliar[k] = arr1[i];
    i++;
    k++;
  }

  while (j < n2 ) {
    auxiliar[k] = arr2[j];
    j++;
    k++;
  }

  return auxiliar;
}

int verificacion_ordenamiento(int *arreglo, int n){
  int i;
  for (i = 0; i < n-1; i ++)
    if( arreglo[i] > arreglo[i+1])
        return 0;

  return 1;
}
