# cpGlobalTema1 - Multiplicación de Matrices Paralela con MPI

TEMA 1: Diseñar un algoritmo paralelo que resuelva la multiplicación de matrices, con la forma C = A∗B, con A y B no necesariamente cuadradas.

## Descripción

Este proyecto implementa la multiplicación de matrices **C(MxN) = A(MxR) * B(RxN)** utilizando MPI (Message Passing Interface) con un modelo SPMD (Single Program Multiple Data) y descomposición de dominio 1D por filas de la matriz A.

### Características principales

- ✅ **Modelo SPMD**: Un solo programa ejecutado por múltiples procesos
- ✅ **Descomposición 1D**: Distribución de filas de A entre procesos
- ✅ **Balanceo de carga**: Distribución equitativa de filas con MPI_Scatterv
- ✅ **Comunicación eficiente**: Uso de MPI_Bcast, MPI_Scatterv y MPI_Gatherv
- ✅ **Matrices no cuadradas**: Soporte para dimensiones arbitrarias M, R, N
- ✅ **Código comentado**: Documentación completa en español

## Estructura del Proyecto

```
cpGlobalTema1/
├── matrix_mult_mpi.c    # Programa principal en C con MPI
├── Makefile             # Archivo de compilación
└── README.md            # Este archivo
```

## Requisitos

- **Compilador C**: gcc
- **MPI**: OpenMPI o MPICH
- **Sistema operativo**: Linux/Unix

### Instalación de dependencias (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y openmpi-bin libopenmpi-dev
```

## Compilación

Para compilar el programa, simplemente ejecute:

```bash
make
```

Para limpiar archivos compilados:

```bash
make clean
```

## Uso

### Sintaxis

```bash
mpirun -np <num_procesos> ./matrix_mult_mpi <M> <R> <N>
```

Donde:
- `num_procesos`: Número de procesos MPI a utilizar
- `M`: Número de filas de la matriz A
- `R`: Número de columnas de A / filas de B
- `N`: Número de columnas de la matriz B

### Ejemplos

**Ejemplo 1: Matrices pequeñas con 2 procesos**
```bash
mpirun -np 2 ./matrix_mult_mpi 8 6 4
```
Multiplica A(8x6) * B(6x4) = C(8x4) usando 2 procesos.

**Ejemplo 2: Matrices medianas con 4 procesos**
```bash
mpirun -np 4 ./matrix_mult_mpi 100 80 60
```
Multiplica A(100x80) * B(80x60) = C(100x60) usando 4 procesos.

**Ejemplo 3: Matrices grandes con 8 procesos**
```bash
mpirun -np 8 ./matrix_mult_mpi 1000 800 600
```
Multiplica A(1000x800) * B(800x600) = C(1000x600) usando 8 procesos.

### Pruebas automáticas

El Makefile incluye un objetivo `test` para ejecutar pruebas básicas:

```bash
make test
```

## Algoritmo

El programa sigue estos pasos:

1. **Inicialización**:
   - El proceso 0 lee los argumentos M, R, N
   - El proceso 0 inicializa las matrices A y B con valores aleatorios

2. **Distribución de datos**:
   - El proceso 0 distribuye las dimensiones M, R, N a todos los procesos con `MPI_Bcast`
   - El proceso 0 distribuye la matriz B completa a todos los procesos con `MPI_Bcast`
   - El proceso 0 distribuye las filas de A entre procesos con `MPI_Scatterv` (balanceo de carga)

3. **Cálculo paralelo**:
   - Cada proceso calcula su porción de C: `local_C = local_A * B`
   - Algoritmo estándar de multiplicación de matrices

4. **Recolección de resultados**:
   - El proceso 0 recolecta todas las porciones de C con `MPI_Gatherv`
   - El proceso 0 muestra los resultados y el tiempo de ejecución

### Balanceo de carga

La distribución de filas se realiza de forma equitativa:
- Si M es divisible por el número de procesos, cada proceso recibe M/n filas
- Si hay resto, los primeros procesos reciben una fila adicional

Ejemplo con M=10 y 3 procesos:
- Proceso 0: 4 filas
- Proceso 1: 3 filas
- Proceso 2: 3 filas

## Detalles de Implementación

### Funciones MPI utilizadas

- `MPI_Init()`: Inicializa el entorno MPI
- `MPI_Comm_rank()`: Obtiene el identificador del proceso
- `MPI_Comm_size()`: Obtiene el número total de procesos
- `MPI_Bcast()`: Difunde datos del proceso raíz a todos los procesos
- `MPI_Scatterv()`: Distribuye porciones de tamaño variable a cada proceso
- `MPI_Gatherv()`: Recolecta porciones de tamaño variable de todos los procesos
- `MPI_Barrier()`: Sincroniza todos los procesos
- `MPI_Wtime()`: Mide el tiempo de ejecución
- `MPI_Finalize()`: Finaliza el entorno MPI

### Almacenamiento de matrices

Las matrices se almacenan en formato row-major (filas consecutivas en memoria):
- Elemento `A[i][j]` se accede como `A[i * cols + j]`

## Verificación

Para matrices pequeñas (≤10x10), el programa imprime:
- La matriz A completa
- La matriz B completa
- La matriz resultado C completa

Para matrices grandes, el programa imprime:
- Los primeros elementos de C para verificación
- El tiempo de cálculo paralelo

## Rendimiento

El programa mide y reporta:
- Tiempo de cálculo paralelo (usando `MPI_Wtime()`)
- Distribución de carga entre procesos

### Optimizaciones implementadas

- Uso de `MPI_Scatterv` y `MPI_Gatherv` para distribución flexible
- Minimización de comunicación (broadcast de B una sola vez)
- Balanceo automático de carga

## Autor

Proyecto desarrollado como parte del curso de Computación Paralela Global.

## Licencia

Este proyecto es de código abierto y está disponible bajo los términos especificados por el curso.
