# Programacion-Paralela-en-C

Colección de ejercicios y ejemplos para practicar programación paralela en C utilizando MPI y OpenMP. El repositorio reúne implementaciones orientadas a distribuir trabajo entre múltiples procesos, medir rendimientos y comparar enfoques de paralelización en distintos algoritmos.

## Objetivos del repositorio
- Mostrar plantillas mínimas para inicializar y finalizar programas con MPI y OpenMP.
- Ejemplificar patrones clásicos (saludos, anillo de mensajes, conteo de primos, ordenamientos paralelos).
- Servir como base para ejecutar y medir experimentos en entornos locales o con gestores de colas como Slurm.

## Mapa de directorios
- `Ejercicio1/`: "Hola mundo" distribuido con MPI.
- `Ejercicio1y2/`: Anillo de paso de mensajes y agregación de mensajes hacia el proceso raíz.
- `ComparacionParaleloRedFuncional/`: Conteo de primos paralelo con distribución por bloques.
- `PrimosParalelosCFOLDER/`: Variante de conteo de primos con envío de resultados al proceso raíz.
- `GAP/Entrega/`: Cálculo de máximos gaps entre números primos en paralelo.
- `Merge-Split/`: Generación de datos y ordenamiento paralelo mediante Merge Sort distribuido.
- `Regular Sampling/`: Implementación de ordenamiento paralelo con muestreo regular.

## Requisitos
- **Compilador de C** con soporte para C99 o superior (probado con `gcc 13.3.0`).
- **OpenMP** (opcional para los ejemplos que usen directivas de multihilo; `libgomp` incluido en GCC 13.3.0).
- **Implementación de MPI** (por ejemplo OpenMPI 4.1.x o MPICH 4.1.x) que proporcione `mpicc` y `mpirun`.
- **Slurm** (opcional) para ejecutar en clúster mediante scripts de envío (`sbatch`). Se han ejecutado ejemplos en entornos con Slurm 22.x.

## Compilación y ejecución
A continuación se listan ejemplos de compilación y ejecución para cada carpeta. Ajusta `-np` según la cantidad de procesos disponibles.

### Ejercicio1
```bash
cd Ejercicio1
mpicc Saludo.c -o saludo_mpi
mpirun -np 4 ./saludo_mpi
```

### Ejercicio1y2
```bash
cd Ejercicio1y2
mpicc anillo.c -o anillo
mpirun -np 4 ./anillo

mpicc mensajes.c -o mensajes
mpirun -np 4 ./mensajes
```

### ComparacionParaleloRedFuncional
```bash
cd ComparacionParaleloRedFuncional
mpicc PrimosParaleloRed.c -lm -o primos_red
mpirun -np 4 ./primos_red 100000
```

### PrimosParalelosCFOLDER
```bash
cd PrimosParalelosCFOLDER
mpicc PrimosParalelo.c -lm -o primos
mpirun -np 4 ./primos 100000
```

### GAP/Entrega
```bash
cd GAP/Entrega
mpicc GAP_p.c -lm -o gap_primos
mpirun -np 4 ./gap_primos 1000000
```

### Merge-Split
```bash
cd Merge-Split
# Generar datos de ejemplo (número de elementos = 100000)
gcc generar_datos.c -o generar_datos
./generar_datos datos.txt 100000

# Ordenar los datos generados
mpicc ms_bin.c -lm -o ms_bin
mpirun -np 4 ./ms_bin datos.txt
```

### Regular Sampling
```bash
cd "Regular Sampling"
# Ordenamiento de datos en texto
a) mpicc rs_dat.c -lm -o rs_dat
   mpirun -np 4 ./rs_dat dataset.txt

# Ordenamiento de datos en binario
b) mpicc rs_bin.c -lm -o rs_bin
   mpirun -np 4 ./rs_bin dataset.bin
```

### Ejecución con Slurm (opcional)
Ejemplo de script para enviar un trabajo MPI:
```bash
#!/bin/bash
#SBATCH --nodes=1                 # número de nodos (máximo 13)
#SBATCH --job-name=MPI_Anillo     # nombre del trabajo
#SBATCH --ntasks=5                # número de procesos
#SBATCH --time=1:00:00            # tiempo máximo
#SBATCH --partition=general       # cola de Slurm
#SBATCH --mem-per-cpu=3200        # memoria RAM por proceso

module load openmpi

mpicc programa.c -o NombreDePrograma
mpirun -np ${SLURM_NTASKS} ./NombreDePrograma > salida.out 2>&1
```

## Pruebas
- Ejecuta cada binario con un número pequeño de procesos (`-np 2`) para validar la configuración de MPI.
- Verifica que los conteos de primos y los ordenamientos sean coherentes comparándolos con ejecuciones secuenciales.
- Para `Merge-Split` y `Regular Sampling`, prueba con datasets pequeños antes de escalar para medir tiempos.

### Pruebas automatizadas
- Instala un entorno con MPI disponible (por ejemplo `sudo apt-get install -y mpich`). Si `mpicc` o `mpirun` no están presentes, el script de pruebas se omite de forma segura y avisa por consola.
- Ejecuta todas las comprobaciones locales con:
  ```bash
  make test
  ```
  El objetivo compila cada fuente principal con `mpicc`/`gcc -fopenmp`, genera datasets pequeños cuando es necesario y verifica salidas básicas (saludos, conteos de primos y ordenamientos).
- Limpia artefactos de compilación de pruebas con `make clean-test`.

## Buenas prácticas
- Compila siempre con flags de depuración cuando estés desarrollando: `mpicc -g -Wall -Wextra -lm fuente.c -o binario`.
- Documenta el número de procesos y el tamaño de entrada al registrar resultados experimentales.
- Revisa que las rutas a archivos de entrada sean accesibles desde todos los nodos cuando uses Slurm.
- Usa `MPI_Barrier` o mediciones con `MPI_Wtime` para aislar secciones críticas de rendimiento.

## Licencia/Autoría
El repositorio contiene material académico desarrollado por Ezequiel Isaac Rodriguez Tenorio y colaboradores (ver encabezados de los archivos). No se ha declarado una licencia explícita; asume uso educativo y revisa el historial de commits antes de redistribuir.
