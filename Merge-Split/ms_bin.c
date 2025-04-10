#include <stdlib.h>
#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)

// Función para mezclar dos arreglos ordenados
int* mezcla(int *arr1, int n1, int *arr2, int n2) {
    int *auxiliar = (int *)malloc((n1 + n2) * sizeof(int));
    if (auxiliar == NULL) {
        printf("Memoria insuficiente en mezcla (auxiliar)\n");
        return NULL;
    }

    int i = 0, j = 0, k = 0;
    while (i < n1 && j < n2) {
        auxiliar[k++] = (arr1[i] <= arr2[j]) ? arr1[i++] : arr2[j++];
    }
    while (i < n1) auxiliar[k++] = arr1[i++];
    while (j < n2) auxiliar[k++] = arr2[j++];

    return auxiliar;
}

// Función para verificar si un arreglo está ordenado
int verificacion_ordenamiento(int *arreglo, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arreglo[i] > arreglo[i + 1]) return 0;
    }
    return 1;
}

// Función de comparación para qsort
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main(int argc, char **argv) {
    int rank, np, root;
    unsigned int n, chunksize;
    float t1, t2;
    int *Arreglo = NULL, *S_i, *recv_buffer, *combined_array;
    MPI_Status status;

    // Inicializar MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Barrier(MPI_COMM_WORLD);

    root = 0;
    t1 = MPI_Wtime();

    // Leer el tamaño del archivo en el root y transmitir a todos los procesos
    if (rank == root) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            printf("Error al abrir el archivo.\n");
            MPI_Abort(MPI_COMM_WORLD, 99);
        }
        fscanf(file, "%u", &n);
        fclose(file);
    }
    MPI_Bcast(&n, 1, MPI_UNSIGNED, root, MPI_COMM_WORLD);

    chunksize = BLOCK_SIZE(rank, np, n);

    if (rank == root) {
        Arreglo = (int *)malloc(n * sizeof(int));
        if (!Arreglo) {
            printf("Error de memoria en Arreglo\n");
            MPI_Abort(MPI_COMM_WORLD, 99);
        }

        FILE *file = fopen(argv[1], "r");
        fscanf(file, "%u", &n);
        for (unsigned int i = 0; i < n; i++) {
            fscanf(file, "%d", &Arreglo[i]);
        }
        fclose(file);
    }

    S_i = (int *)malloc(chunksize * sizeof(int));
    if (!S_i) {
        printf("Error de memoria en S_i\n");
        MPI_Abort(MPI_COMM_WORLD, 99);
    }

    MPI_Scatter(Arreglo, chunksize, MPI_INT, S_i, chunksize, MPI_INT, root, MPI_COMM_WORLD);

    // Ordenar el chunk local
    qsort(S_i, chunksize, sizeof(int), compare);

    // Fase de combinación en Merge Sort Paralelo
    int step;
    for (step = 1; step < np; step *= 2) {
        if (rank % (2 * step) == 0) {
            if (rank + step < np) {
                int partner_size;
                MPI_Recv(&partner_size, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, &status);

                recv_buffer = (int *)malloc(partner_size * sizeof(int));
                if (!recv_buffer) {
                    printf("Error de memoria en recv_buffer\n");
                    MPI_Abort(MPI_COMM_WORLD, 99);
                }

                MPI_Recv(recv_buffer, partner_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, &status);

                combined_array = mezcla(S_i, chunksize, recv_buffer, partner_size);
                if (!combined_array) {
                    printf("Error en mezcla\n");
                    MPI_Abort(MPI_COMM_WORLD, 99);
                }

                free(S_i);
                free(recv_buffer);
                S_i = combined_array;
                chunksize += partner_size;
            }
        } else {
            int dest = rank - step;
            MPI_Send(&chunksize, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(S_i, chunksize, MPI_INT, dest, 0, MPI_COMM_WORLD);
            free(S_i);
            break;
        }
    }

    t2 = MPI_Wtime();

    // Imprimir resultados en el root
    if (rank == root) {
        int ordenado = verificacion_ordenamiento(S_i, n);
        printf("%d, %d, %f\n", np, ordenado, t2 - t1);
        free(S_i);
    }

    MPI_Finalize();
    return 0;
}

