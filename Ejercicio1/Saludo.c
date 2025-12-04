#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  /* Variables de estado */
  int init_status = MPI_SUCCESS;
  int size_status = MPI_SUCCESS;
  int rank_status = MPI_SUCCESS;
  int finalize_status = MPI_SUCCESS;
  int exit_code = EXIT_SUCCESS;

  /* Datos de proceso */
  int rank = 0;
  int size = 0;

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
  } else {
    /* Mostrar saludo cuando los datos son válidos */
    printf("Hola, mundo de C+MPI, desde el proceso %d de %d\n", rank, size);
  }

  /* Finalizar MPI garantizando una salida ordenada */
  finalize_status = MPI_Finalize();
  if (finalize_status != MPI_SUCCESS) {
    fprintf(stderr, "Advertencia: MPI_Finalize devolvió código %d.\n", finalize_status);
    exit_code = EXIT_FAILURE;
  }

  return exit_code;
}
