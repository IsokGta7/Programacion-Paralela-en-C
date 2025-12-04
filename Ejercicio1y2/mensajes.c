#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  /* Variables de estado */
  int init_status = MPI_SUCCESS;
  int size_status = MPI_SUCCESS;
  int rank_status = MPI_SUCCESS;
  int send_status = MPI_SUCCESS;
  int recv_status = MPI_SUCCESS;
  int finalize_status = MPI_SUCCESS;
  int exit_code = EXIT_SUCCESS;

  /* Datos de proceso */
  int rank = 0;
  int size = 0;
  int i = 0;
  const int root = 0;
  char mensaje[50];

  /* Inicializar MPI y validar el resultado */
  init_status = MPI_Init(&argc, &argv);
  if (init_status != MPI_SUCCESS) {
    fprintf(stderr, "Error: MPI_Init falló (código %d).\n", init_status);
    return EXIT_FAILURE;
  }

  /* Consultar tamaño y rango del comunicador */
  size_status = MPI_Comm_size(MPI_COMM_WORLD, &size);
  rank_status = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (size_status != MPI_SUCCESS || rank_status != MPI_SUCCESS) {
    if (size_status != MPI_SUCCESS) {
      fprintf(stderr, "Error: MPI_Comm_size falló (código %d).\n", size_status);
    }
    if (rank_status != MPI_SUCCESS) {
      fprintf(stderr, "Error: MPI_Comm_rank falló (código %d).\n", rank_status);
    }
    exit_code = EXIT_FAILURE;
    goto finalize;
  }

  /* Preparar y enviar el mensaje al proceso raíz */
  snprintf(mensaje, sizeof(mensaje),
           "Hola proceso %d, un saludo desde el proceso %d", root, rank);
  send_status = MPI_Send(mensaje, strlen(mensaje) + 1, MPI_CHAR, root, 9,
                         MPI_COMM_WORLD);
  if (send_status != MPI_SUCCESS) {
    fprintf(stderr, "Error: MPI_Send falló en el proceso %d (código %d).\n",
            rank, send_status);
    exit_code = EXIT_FAILURE;
    goto finalize;
  }

  /* Recibir y mostrar los mensajes en el proceso raíz */
  if (rank == root) {
    printf("Soy el proceso %d y recibí los mensajes:\n", root);
    for (i = 0; i < size; i++) {
      recv_status = MPI_Recv(mensaje, (int)sizeof(mensaje), MPI_CHAR, i, 9,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if (recv_status != MPI_SUCCESS) {
        fprintf(stderr,
                "Error: MPI_Recv falló al recibir del proceso %d (código %d).\n",
                i, recv_status);
        exit_code = EXIT_FAILURE;
        break;
      }
      printf("<%s>\n", mensaje);
    }
  }

finalize:
  /* Finalizar MPI garantizando una salida ordenada */
  finalize_status = MPI_Finalize();
  if (finalize_status != MPI_SUCCESS) {
    fprintf(stderr, "Advertencia: MPI_Finalize devolvió código %d.\n",
            finalize_status);
    exit_code = EXIT_FAILURE;
  }

  return exit_code;
}
