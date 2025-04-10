#include <stdlib.h>
#include "mpi.h"
#include <stdio.h>
#include <math.h>  // ceil()
#include <limits.h>  // INT_MAX

#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)

int compare(const void *_a, const void *_b) {
    int *a, *b;
    a = (int *)_a;
    b = (int *)_b;
    return (*a - *b);
}

//  ■■■■■■■■■■
// **✅ Nueva versión para leer archivos binarios**
int read_array_bin(char* fname, int **arr, int *n) {
    FILE *file = fopen(fname, "rb");
    if (!file) {
        printf("ERROR: No se pudo abrir el archivo %s\n", fname);
        MPI_Abort(MPI_COMM_WORLD, 99);
    }

    // **Leer el número total de datos (n)**
    fread(n, sizeof(int), 1, file);

    // **Reservar memoria para los datos**
    *arr = (int *)malloc(*n * sizeof(int));
    if (*arr == NULL) {
        printf("Memoria insuficiente para datos\n");
        MPI_Abort(MPI_COMM_WORLD, 99);
    }

    // **Leer los datos en memoria**
    fread(*arr, sizeof(int), *n, file);

    fclose(file);
    return *n;
}

// **✅ Función de mezcla sin cambios**
int* mezcla(int *arr1, int n1, int *arr2, int n2) {
    int *auxiliar;
    int i = 0, j = 0, k = 0;

    auxiliar = (int *)malloc((n1 + n2) * sizeof(int));
    if (auxiliar == NULL) {
        printf("Memoria insuficiente (auxiliar)\n");
        return NULL;
    }

    while (i < n1 && j < n2) {
        auxiliar[k++] = (arr1[i] <= arr2[j]) ? arr1[i++] : arr2[j++];
    }
    while (i < n1) auxiliar[k++] = arr1[i++];
    while (j < n2) auxiliar[k++] = arr2[j++];

    return auxiliar;
}

// **✅ Verificación de ordenamiento sin cambios**
int verificacion_ordenamiento(int *arreglo, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arreglo[i] > arreglo[i + 1]) return 0;
    }
    return 1;
}

