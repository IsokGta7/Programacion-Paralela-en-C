#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/tests/build"
TMP_DIR="$ROOT_DIR/tests/tmp"
export TMP_DIR
MPICC_BIN=${MPICC:-mpicc}
CC_BIN=${CC:-gcc}
MPIEXEC=${MPIEXEC:-mpirun}

if ! command -v "$MPICC_BIN" >/dev/null 2>&1; then
  echo "mpicc no está disponible; instala una implementación de MPI (p. ej., MPICH u OpenMPI)." >&2
  exit 1
fi
if ! command -v "$MPIEXEC" >/dev/null 2>&1; then
  echo "mpirun/mpiexec no está disponible." >&2
  exit 1
fi

if [[ -z "${MPIRUN_FLAGS:-}" ]]; then
  if "$MPIEXEC" --help 2>&1 | grep -q -- "--allow-run-as-root"; then
    MPIRUN_FLAGS="--allow-run-as-root --oversubscribe"
  else
    MPIRUN_FLAGS=""
  fi
fi

run_mpi() {
  local np="$1"; shift
  "$MPIEXEC" ${MPIRUN_FLAGS} -np "$np" "$@"
}

mkdir -p "$BUILD_DIR" "$TMP_DIR"

# 1) Saludo MPI
$MPICC_BIN "$ROOT_DIR/Ejercicio1/Saludo.c" -o "$BUILD_DIR/saludo_mpi"
SALUDO_OUTPUT=$(run_mpi 2 "$BUILD_DIR/saludo_mpi")
[[ "$SALUDO_OUTPUT" == *"Hola"* ]]

# 2) Anillo
$MPICC_BIN "$ROOT_DIR/Ejercicio1y2/anillo.c" -o "$BUILD_DIR/anillo_mpi"
ANILLO_OUTPUT=$(run_mpi 3 "$BUILD_DIR/anillo_mpi")
[[ "$ANILLO_OUTPUT" == *"envié el mensaje"* ]]

# 3) Mensajes
$MPICC_BIN "$ROOT_DIR/Ejercicio1y2/mensajes.c" -o "$BUILD_DIR/mensajes_mpi"
MENSAJES_OUTPUT=$(run_mpi 3 "$BUILD_DIR/mensajes_mpi")
[[ "$MENSAJES_OUTPUT" == *"recibí los mensajes"* ]]

# 4) Conteo de primos con reducción
$MPICC_BIN "$ROOT_DIR/ComparacionParaleloRedFuncional/PrimosParaleloRed.c" -lm -o "$BUILD_DIR/primos_red"
PRIMOS_RED_OUTPUT=$(run_mpi 2 "$BUILD_DIR/primos_red" 30)
[[ "$PRIMOS_RED_OUTPUT" == *, 10,* ]]

# 5) Conteo de primos con envío al root
$MPICC_BIN "$ROOT_DIR/PrimosParalelosCFOLDER/PrimosParalelo.c" -lm -o "$BUILD_DIR/primos_envio"
PRIMOS_ENVIO_OUTPUT=$(run_mpi 2 "$BUILD_DIR/primos_envio" 30)
[[ "$PRIMOS_ENVIO_OUTPUT" == *, 10,* ]]

# 6) Máximo gap entre primos
$MPICC_BIN "$ROOT_DIR/GAP/Entrega/GAP_p.c" -lm -o "$BUILD_DIR/gap_primos"
GAP_OUTPUT=$(run_mpi 2 "$BUILD_DIR/gap_primos" 30)
[[ "$GAP_OUTPUT" == *, 4,* ]]

# 7) Merge-Split: generar datos y ordenar
$CC_BIN -fopenmp "$ROOT_DIR/Merge-Split/generar_datos.c" -o "$BUILD_DIR/generar_datos"
python - <<'PY'
import os, pathlib
nums = [8, 3, 5, 7, 2, 11, 9, 4]
path = pathlib.Path(os.environ["TMP_DIR"]) / "datos_ms.txt"
path.write_text(f"{len(nums)}\n" + "\n".join(map(str, nums)) + "\n")
PY
$MPICC_BIN "$ROOT_DIR/Merge-Split/ms_bin.c" -lm -o "$BUILD_DIR/ms_bin"
MS_OUTPUT=$(run_mpi 2 "$BUILD_DIR/ms_bin" "$TMP_DIR/datos_ms.txt")
[[ "$MS_OUTPUT" == *, 1,* ]]

# 8) Regular Sampling (texto)
python - <<'PY'
import os, pathlib
nums = [15, 4, 9, 1, 7, 2]
path = pathlib.Path(os.environ["TMP_DIR"]) / "rs_datos.txt"
path.write_text(f"{len(nums)}\n" + "\n".join(map(str, nums)) + "\n")
PY
$MPICC_BIN -I"$ROOT_DIR/Regular Sampling" "$ROOT_DIR/Regular Sampling/rs_dat.c" -lm -o "$BUILD_DIR/rs_dat"
RS_DAT_OUTPUT=$(run_mpi 2 "$BUILD_DIR/rs_dat" "$TMP_DIR/rs_datos.txt")
[[ "$RS_DAT_OUTPUT" == *" 1 "* || "$RS_DAT_OUTPUT" == *", 1,"* ]]

# 9) Regular Sampling (binario)
python - <<'PY'
import os, struct, pathlib
nums = [6, 5, 3, 1, 4, 2]
path = pathlib.Path(os.environ["TMP_DIR"]) / "rs_datos.bin"
with path.open('wb') as f:
    f.write(struct.pack('i', len(nums)))
    f.write(struct.pack(f'{len(nums)}i', *nums))
PY
$MPICC_BIN -I"$ROOT_DIR/Regular Sampling" "$ROOT_DIR/Regular Sampling/rs_bin.c" -lm -o "$BUILD_DIR/rs_bin"
RS_BIN_OUTPUT=$(run_mpi 2 "$BUILD_DIR/rs_bin" "$TMP_DIR/rs_datos.bin")
[[ "$RS_BIN_OUTPUT" == *" 1 "* || "$RS_BIN_OUTPUT" == *", 1,"* ]]

echo "Todas las pruebas finalizaron correctamente."
