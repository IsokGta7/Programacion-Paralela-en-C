// Nombre: Ezequiel Isaac Rodriguez Tenorio

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Se necesitan al menos 2 procesos para ejecutar este programa.\n");
        }
        MPI_Finalize();
        return 0;
    }

    char message[100];

    if (rank == 0) {
        // Proceso 0 inicia el mensaje
        snprintf(message, sizeof(message), "Testeo de procesos locochones #%d", rank);
        MPI_Send(message, strlen(message) + 1, MPI_CHAR, (rank + 1) % size, 0, MPI_COMM_WORLD);
        printf("Soy el proceso %d y envié el mensaje: %s\n", rank, message);

        // Recibe el mensaje del último proceso
        MPI_Recv(message, sizeof(message), MPI_CHAR, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Soy el proceso %d y recibí el mensaje: %s\n", rank, message);
    } else {
        // Resto de procesos
        MPI_Recv(message, sizeof(message), MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Soy el proceso %d y recibí el mensaje: %s desde el proceso %d\n", rank, message, rank - 1);

        snprintf(message, sizeof(message), "Mas mensajes locochones #%d", rank);
        MPI_Send(message, strlen(message) + 1, MPI_CHAR, (rank + 1) % size, 0, MPI_COMM_WORLD);
        printf("Soy el proceso %d y envié el mensaje: %s\n", rank, message);
    }

    MPI_Finalize();
    return 0;
}
