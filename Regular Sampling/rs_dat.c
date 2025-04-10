#include "header.h"
#include <string.h>

int main(int argc, char**argv) {
  int id, np, root, IND;
  int n, i,j, n_local, *displs, *sendcounts,*recvcounts,*sdispls,*rdispls, ndatos_recv;;
  double start, time;
  int *sendbuf, *recvbuf,  *muestras, *recvbuf_mezclas;
  int *todas_las_muestras, *pivotes;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
  MPI_Comm_rank(MPI_COMM_WORLD, &id); // Valor de nuestro identificador

  start = MPI_Wtime();

  // El root lee un dataset del archivo con n datos e inicia una dispersión de los datos.  <-----  (PASO 1)
  root = np - 1;
  
  if (id == root) { // Lee los datos del archivo
    n = read_array(argv[1], &sendbuf, np);
  }

  sendcounts = (int*)malloc(np*sizeof(int));
  if (sendcounts == NULL) {printf("Memoria insuficiente (sendcounts) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

  if(id==root){
    for(i=0;i<np;i++)
      sendcounts[i] = BLOCK_SIZE(i,np,n);


    displs = (int*)malloc(np*sizeof(int));
    if(displs==NULL){ printf("ERROR: Memoria insuficiente (displs)"); MPI_Abort(MPI_COMM_WORLD,99); }
    displs[0]=0;
    for(i=1;i<np;i++)
      displs[i] = displs[i-1]+sendcounts[i-1];
  }

  // Comunica longitud de los chunks
  MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);
  n_local = BLOCK_SIZE(id,np,n);

  recvbuf = (int *) malloc(n_local*sizeof(int));
  if (recvbuf == NULL) {printf("Memoria insuficiente (recvbuf)\n");MPI_Abort(MPI_COMM_WORLD, 99);}

  MPI_Scatterv(sendbuf, sendcounts, displs, MPI_INT, recvbuf, n_local, MPI_INT, root, MPI_COMM_WORLD);
  
  // Cada proceso ordena localmente sus datos con QuickSort  <-----  (PASO 2)
  qsort(recvbuf, n_local, sizeof(int), &compare);


  // Cada proceso selecciona los p índices 0, n/p^2, 2n/p^2, 3n/p^2,..., (p-1)n/p^2 como muestras regulares <-----  (PASO 3)
  muestras = (int*)malloc(np*sizeof(int));
  if (muestras == NULL) {printf("Memoria insuficiente (muestras)\n");MPI_Abort(MPI_COMM_WORLD, 99);}

  for(i=0;i<np;i++)
    muestras[i] = recvbuf[ i*n/(np*np) ];

  // if (id==0) { printf("\n[%d] muestras={",id);  for(i=0;i<np;i++) printf("%d, ", muestras[i]); printf("}\n"); }// <---- impresion

  // Un proceso recolecta y ordena las muestras regulares y selecciona p-1 valores pivote de la lista ordenada. <-----  (PASO 4)
  // Los valores pivote están en los índices p+p/2-1, 2p+p/2-1, 3p+p/2-1, ... , (p-1)p+p/2-1.
  // Los valores pivote son dados a conocer a todos los procesos.

  if(id==root){
    todas_las_muestras = (int*)malloc(   np*np*sizeof(int) );
    if (todas_las_muestras == NULL) {printf("Memoria insuficiente (todas_las_muestras)\n");MPI_Abort(MPI_COMM_WORLD, 99);}
  }

  MPI_Gather( muestras, np, MPI_INT, todas_las_muestras, np, MPI_INT, root, MPI_COMM_WORLD );

  if(id==root){
    qsort( todas_las_muestras, np*np, sizeof(int), &compare );

  //  printf("\n[%d] todas_las_muestras={",id);  for(i=0;i<np*np;i++) printf("%d, ", todas_las_muestras[i]); printf("}\n"); // <---- impresion
  }
  pivotes = (int*)malloc( (np-1)*sizeof(int)   );
  if (pivotes == NULL) {printf("Memoria insuficiente (pivotes) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

  if(id==root){
    for(i=0;i<np-1;i++)
      pivotes[i] = todas_las_muestras[ (i+1)*np + np/2 -1 ];
  }

  if (id==root) {
    //printf("\n[%d] pivotes={",id);  for(i=0;i<np-1;i++) printf("%d, ", pivotes[i]); printf("}\n");
  } // <---- impresion

  MPI_Bcast(pivotes, np-1, MPI_INT, root, MPI_COMM_WORLD);


  // Cada proceso particiona su sublista ordenada en p piezas disjuntas, usando los valores pivote como separadores. <----- (PASO 5)
  j=0;
  for(i=0;i<(np-1);i++){
    sendcounts[i]=0;
    while(recvbuf[j]<=pivotes[i] && j<n_local ){
      sendcounts[i]++;
      j++;
    }
  }
  sendcounts[np-1]=0;
  while( j < n_local ){
    sendcounts[np-1]++;
    j++;
  }
  //if(id==0){ printf("\n[%d] sendcounts={",id);  for(i=0;i<np;i++) printf("%d, ", sendcounts[i]); printf("}\n"); } // <---- impresion
  sdispls = (int*)malloc( np*sizeof(int) );
  if (sdispls == NULL) {printf("Memoria insuficiente (sdispls) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

  sdispls[0]=0;
  for(i=1;i<np;i++)
    sdispls[i] = sdispls[i-1]+sendcounts[i-1];
  //if(id==0){ printf("\n[%d] sdispls={",id);  for(i=0;i<np;i++) printf("%d, ", sdispls[i]); printf("}\n"); } // <---- impresion

  recvcounts =  (int*)malloc( np*sizeof(int) );
  if (recvcounts == NULL) {printf("Memoria insuficiente (recvcounts) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

  MPI_Alltoall( sendcounts, 1, MPI_INT, recvcounts, 1, MPI_INT, MPI_COMM_WORLD );

  rdispls = (int*)malloc( np*sizeof(int) );
  if (rdispls == NULL) {printf("Memoria insuficiente (rdispls) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

  rdispls[0]=0;
  for(i=1;i<np;i++)
    rdispls[i] = rdispls[i-1]+recvcounts[i-1];
 // if(id==0){ printf("\n[%d] rdispls={",id);  for(i=0;i<np;i++) printf("%d, ", rdispls[i]); printf("}\n"); } // <---- impresion

  ndatos_recv =0;
  for(i=0;i<np;i++)
    ndatos_recv += recvcounts[i];

  recvbuf_mezclas = (int*)malloc( ndatos_recv *sizeof(int) );
  if (recvbuf_mezclas == NULL) {printf("Memoria insuficiente (recvbuf_mezclas) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

  // Cada proceso Pi mantiene su iesima partición y envía la j-partición al proceso Pj (alltoallv)  <----- ( PASO 6)

  MPI_Alltoallv( recvbuf, sendcounts, sdispls, MPI_INT, recvbuf_mezclas, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD );

  //if(id==0){ printf("\n[%d] recvbuf_mezclas={",id);  for(i=0;i<ndatos_recv;i++) printf("%d, ", recvbuf_mezclas[i]); printf("}\n"); } // <---- impresion


  // Inicializar la primera parte
  int *mezclado = (int *)malloc(recvcounts[0] * sizeof(int));
  for (i = 0; i < recvcounts[0]; i++)
    mezclado[i] = recvbuf_mezclas[i];
  int tamano_mezclado = recvcounts[0];

  // Mezclar cada nueva parte en nuevo_mezclado
  for (i = 1; i < np; i++) {
    mezclado = mezcla(mezclado, tamano_mezclado, recvbuf_mezclas + rdispls[i], recvcounts[i]);
    tamano_mezclado += recvcounts[i];
  }

  free(recvbuf_mezclas);

  recvbuf_mezclas = mezclado;
  ndatos_recv = tamano_mezclado;

  time = MPI_Wtime() -start; // Tiempo de ejecución
  // Recolectar los elementos que tiene cada proceso en un solo proceso con fines de impresión <---- (PASO 8)

  MPI_Gather(&ndatos_recv, 1, MPI_INT, recvcounts, 1,MPI_INT, root, MPI_COMM_WORLD);
  if(id==root){
    rdispls[0]=0;
    for(i=1;i<np;i++)
      rdispls[i]= rdispls[i-1] + recvcounts[i-1];
  }

  MPI_Gatherv( recvbuf_mezclas, ndatos_recv, MPI_INT, sendbuf, recvcounts, rdispls, MPI_INT, root, MPI_COMM_WORLD );




  if(id==root){
    /*
     printf("\nDatos ordenados = {");
     for(i=0;i<n;i++){
      printf("%d, ", sendbuf[i]);
      }
       printf("}\n");
       printf("size = %d", n);
  */
       IND=verificacion_ordenamiento(sendbuf, n);
        printf("%d %d %f\n", np, IND, time);
     } // <---- impresion


  // Liberar memoria
  if(id==root){
    free(sendcounts);
    free(displs);
    free(todas_las_muestras);
  }
  free(mezclado);
  free(recvbuf);
  free(muestras);
  free(pivotes);

  MPI_Finalize();
  return 0;
}
