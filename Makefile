# Makefile para la Multiplicación de Matrices Paralela con MPI
# ============================================================

# Compilador MPI
CC = mpicc

# Flags de compilación
CFLAGS = -Wall -O2 -lm

# Nombre del ejecutable
TARGET = matrix_mult_mpi

# Archivo fuente
SRC = matrix_mult_mpi.c

# Regla por defecto: compilar el programa
all: $(TARGET)

# Regla para compilar el programa
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
	@echo "Compilación exitosa: $(TARGET)"
	@echo ""
	@echo "Ejemplos de uso:"
	@echo "  mpirun -np 4 ./$(TARGET) 100 50 80"
	@echo "  mpirun -np 2 ./$(TARGET) 8 6 4"
	@echo ""

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET)
	@echo "Archivos limpiados"

# Regla para ejecutar un test básico
test: $(TARGET)
	@echo "Ejecutando test con matriz pequeña (8x6 * 6x4)..."
	mpirun -np 2 --allow-run-as-root ./$(TARGET) 8 6 4
	@echo ""
	@echo "Ejecutando test con matriz mediana (100x80 * 80x60)..."
	mpirun -np 4 --allow-run-as-root --oversubscribe ./$(TARGET) 100 80 60

# Regla de ayuda
help:
	@echo "Makefile para Multiplicación de Matrices MPI"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  all    - Compilar el programa (objetivo por defecto)"
	@echo "  clean  - Eliminar archivos compilados"
	@echo "  test   - Ejecutar pruebas básicas"
	@echo "  help   - Mostrar esta ayuda"
	@echo ""
	@echo "Uso manual:"
	@echo "  mpirun -np <num_procesos> ./$(TARGET) <M> <R> <N>"
	@echo ""
	@echo "Donde:"
	@echo "  num_procesos - Número de procesos MPI a usar"
	@echo "  M - Número de filas de la matriz A"
	@echo "  R - Número de columnas de A / filas de B"
	@echo "  N - Número de columnas de la matriz B"

.PHONY: all clean test help
