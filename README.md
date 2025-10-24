# cpGlobalTema1
TEMA 1: Diseñar un algoritmo paralelo que resuelva la multiplicación de matrices, con la forma C = A∗B, con A y B no necesariamente cuadradas.

## Descripción

Este proyecto implementa la multiplicación de matrices paralela usando MPI con el modelo SPMD (Single Program Multiple Data) y descomposición de dominio 1D por filas de la matriz A.

### Características

- **Modelo SPMD**: Todos los procesos ejecutan el mismo programa
- **Descomposición 1D**: Las filas de la matriz A se distribuyen entre los procesos
- **Balanceo de carga**: Distribución equitativa de filas usando `MPI_Scatterv` y `MPI_Gatherv`
- **Broadcast eficiente**: La matriz B se transmite a todos los procesos con `MPI_Bcast`
- **Dimensiones flexibles**: Soporta matrices no cuadradas de cualquier tamaño compatible

## Algoritmo

1. **Proceso 0** lee las dimensiones M, R, N de los argumentos de línea de comandos
2. **Proceso 0** inicializa las matrices A(M×R) y B(R×N)
3. **Proceso 0** usa `MPI_Bcast` para enviar la matriz B y las dimensiones a todos los procesos
4. **Proceso 0** usa `MPI_Scatterv` para distribuir las filas de A entre todos los procesos (con balanceo de carga)
5. **Cada proceso** calcula su porción de la matriz resultado C
6. **Proceso 0** usa `MPI_Gatherv` para recolectar todas las porciones y formar la matriz C final

## Compilación

```bash
make
```

Esto generará el ejecutable `matrix_mult`.

## Uso

```bash
mpirun -np <num_procesos> ./matrix_mult M R N
```

Donde:
- `num_procesos`: Número de procesos MPI a utilizar
- `M`: Número de filas de la matriz A
- `R`: Número de columnas de A / filas de B
- `N`: Número de columnas de la matriz B

### Ejemplos

```bash
# Multiplicación de matrices 4×4 con 2 procesos
mpirun -np 2 --oversubscribe ./matrix_mult 4 4 4

# Multiplicación de matrices no cuadradas 10×8 * 8×6 con 4 procesos
mpirun -np 4 --oversubscribe ./matrix_mult 10 8 6

# Multiplicación 5×3 * 3×7 con 3 procesos
mpirun -np 3 --oversubscribe ./matrix_mult 5 3 7
```

## Pruebas

El Makefile incluye varios objetivos de prueba:

```bash
# Prueba básica (4×4 matrices, 2 procesos)
make test

# Prueba con matrices grandes (10×8 * 8×6, 4 procesos)
make test-large

# Prueba con matrices no cuadradas (5×3 * 3×7, 3 procesos)
make test-nonsquare
```

## Implementación

El programa implementa todos los requisitos especificados:

1. ✅ Proceso 0 inicializa A, B y dimensiones M, R, N
2. ✅ Proceso 0 usa `MPI_Bcast` para enviar B y las dimensiones a todos
3. ✅ Proceso 0 usa `MPI_Scatterv` para distribuir las filas de A (balanceo de carga)
4. ✅ Cada proceso calcula su porción de C
5. ✅ Proceso 0 usa `MPI_Gatherv` para recolectar el C final
6. ✅ M, R, N se pasan por argumentos argv

### Detalles de la Implementación

- **Balanceo de carga**: Las filas se distribuyen de manera que los primeros procesos obtengan una fila extra si M no es divisible entre el número de procesos
- **Inicialización**: Las matrices se inicializan con valores de prueba para facilitar la verificación
- **Salida**: Si las matrices son pequeñas (≤10×10), se imprimen completas; de lo contrario, solo se muestra información resumida

## Requisitos

- OpenMPI (o cualquier implementación de MPI)
- GCC o compilador compatible con C
- Sistema operativo compatible con MPI (Linux, macOS, etc.)

## Limpieza

Para eliminar los archivos compilados:

```bash
make clean
```
