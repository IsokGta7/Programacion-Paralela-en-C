//Isaac Rodríguez

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Función para verificar si un número es primo
int es_primo(int n) {
    if (n < 2) return 0;
    if (n == 2 || n == 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    
    int i;
    for (i = 5; i * i <= n; i += 6){
        if (n % i == 0 || n % (i + 2) == 0){
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int id, np, n;
    double t1, t2;

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (argc != 2) {
        if (id == 0) printf("Se requiere un argumento (número límite)\n");
        MPI_Finalize();
        return 1;
    }

    t1 = MPI_Wtime();
    n = atoi(argv[1]);
    int bloque = (n - 1) / np; // Distribuir carga más equitativamente
    int inicio = id * bloque + 2;
    int fin = (id == np - 1) ? n : (inicio + bloque - 1);

    int ultimo_primo_local = -1;
    int primer_primo_local = -1;
    int max_gap_local = 0;
    int gap_entre_procesos = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    // Buscar primos en el rango
    int i;
    for (i = inicio; i <= fin; i++) {
        if (es_primo(i)) {
            if (ultimo_primo_local != -1) {
                int gap_actual = i - ultimo_primo_local;
                if (gap_actual > max_gap_local) {
                    max_gap_local = gap_actual;
                }
            }
            if (primer_primo_local == -1) {
                primer_primo_local = i;
            }
            ultimo_primo_local = i;
        }
    }

    // Comunicación eficiente para conectar gaps entre procesos
    int ultimo_primo_anterior;
    if (id > 0) {
        MPI_Recv(&ultimo_primo_anterior, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (ultimo_primo_anterior != -1 && primer_primo_local != -1) {
            gap_entre_procesos = primer_primo_local - ultimo_primo_anterior;
            if (gap_entre_procesos > max_gap_local) {
                max_gap_local = gap_entre_procesos;
            }
        }
    }

    if (id < np - 1) {
        MPI_Send(&ultimo_primo_local, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
    }

    // Reducir para encontrar el gap máximo global
    int max_gap_global;
    MPI_Reduce(&max_gap_local, &max_gap_global, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    t2 = MPI_Wtime();
    if (id == 0) {
        printf("%d, %d, %f\n", np, max_gap_global, t2 - t1);
    }

    MPI_Finalize();
    return 0;
}

