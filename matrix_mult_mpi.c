/**
 * Multiplicación de Matrices Paralela con MPI
 * ============================================
 * 
 * Programa: Multiplicación de matrices C(MxN) = A(MxR) * B(RxN)
 * Modelo: SPMD (Single Program Multiple Data)
 * Descomposición: 1D por filas de la matriz A
 * 
 * Características:
 * - El proceso 0 inicializa las matrices A y B
 * - Se usa MPI_Bcast para distribuir la matriz B y las dimensiones
 * - Se usa MPI_Scatterv para distribuir las filas de A con balanceo de carga
 * - Cada proceso calcula su porción de la matriz resultado C
 * - Se usa MPI_Gatherv para recolectar el resultado final en proceso 0
 * 
 * Compilación: mpicc -o matrix_mult_mpi matrix_mult_mpi.c -lm
 * Ejecución: mpirun -np <num_procesos> ./matrix_mult_mpi <M> <R> <N>
 * 
 * Ejemplo: mpirun -np 4 ./matrix_mult_mpi 100 50 80
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Función para inicializar una matriz con valores aleatorios
 * 
 * @param matrix Puntero a la matriz (arreglo 1D)
 * @param rows Número de filas
 * @param cols Número de columnas
 */
void init_matrix(double *matrix, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = ((double)rand() / RAND_MAX) * 10.0;
    }
}

/**
 * Función para imprimir una matriz (para depuración con matrices pequeñas)
 * 
 * @param matrix Puntero a la matriz
 * @param rows Número de filas
 * @param cols Número de columnas
 * @param name Nombre de la matriz para mostrar
 */
void print_matrix(double *matrix, int rows, int cols, const char *name) {
    printf("\nMatriz %s (%dx%d):\n", name, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%8.2f ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

/**
 * Función principal del programa
 */
int main(int argc, char *argv[]) {
    int rank;           // Rango del proceso actual
    int size;           // Número total de procesos
    int M, R, N;        // Dimensiones de las matrices: A(MxR), B(RxN), C(MxN)
    
    double *A = NULL;   // Matriz A completa (solo en proceso 0)
    double *B = NULL;   // Matriz B (en todos los procesos)
    double *C = NULL;   // Matriz C resultado completa (solo en proceso 0)
    
    double *local_A;    // Porción de filas de A asignadas a este proceso
    double *local_C;    // Porción de filas de C calculadas por este proceso
    
    int *sendcounts;    // Número de elementos a enviar a cada proceso (para Scatterv)
    int *displs;        // Desplazamientos para cada proceso (para Scatterv)
    int local_rows;     // Número de filas asignadas a este proceso
    
    double start_time = 0.0, end_time;  // Para medir el tiempo de ejecución
    
    // ========================================================================
    // INICIALIZACIÓN DE MPI
    // ========================================================================
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // ========================================================================
    // VALIDACIÓN DE ARGUMENTOS
    // ========================================================================
    if (argc != 4) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <M> <R> <N>\n", argv[0]);
            fprintf(stderr, "  M: Número de filas de A\n");
            fprintf(stderr, "  R: Número de columnas de A / filas de B\n");
            fprintf(stderr, "  N: Número de columnas de B\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Leer dimensiones de los argumentos
    M = atoi(argv[1]);
    R = atoi(argv[2]);
    N = atoi(argv[3]);
    
    // Validar que las dimensiones sean positivas
    if (M <= 0 || R <= 0 || N <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: Las dimensiones deben ser positivas\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    if (rank == 0) {
        printf("==========================================================\n");
        printf("Multiplicación de Matrices Paralela con MPI\n");
        printf("==========================================================\n");
        printf("Dimensiones: C(%dx%d) = A(%dx%d) * B(%dx%d)\n", M, N, M, R, R, N);
        printf("Número de procesos: %d\n", size);
        printf("==========================================================\n\n");
    }
    
    // ========================================================================
    // PROCESO 0: INICIALIZACIÓN DE MATRICES Y CÁLCULO DE DISTRIBUCIÓN
    // ========================================================================
    if (rank == 0) {
        // Asignar memoria para las matrices completas
        A = (double *)malloc(M * R * sizeof(double));
        B = (double *)malloc(R * N * sizeof(double));
        C = (double *)malloc(M * N * sizeof(double));
        
        if (!A || !B || !C) {
            fprintf(stderr, "Error: No se pudo asignar memoria para las matrices\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Inicializar matrices con valores aleatorios
        srand(time(NULL));
        init_matrix(A, M, R);
        init_matrix(B, R, N);
        
        // Imprimir matrices si son pequeñas (para verificación)
        if (M <= 10 && R <= 10 && N <= 10) {
            print_matrix(A, M, R, "A");
            print_matrix(B, R, N, "B");
        }
        
        printf("Matrices inicializadas correctamente.\n\n");
    }
    
    // ========================================================================
    // BROADCAST DE LA MATRIZ B Y LAS DIMENSIONES A TODOS LOS PROCESOS
    // ========================================================================
    // Todos los procesos necesitan conocer las dimensiones
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&R, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Asignar memoria para B en todos los procesos (excepto el 0 que ya la tiene)
    if (rank != 0) {
        B = (double *)malloc(R * N * sizeof(double));
        if (!B) {
            fprintf(stderr, "Proceso %d: Error al asignar memoria para B\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Broadcast de la matriz B completa a todos los procesos
    MPI_Bcast(B, R * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Matriz B distribuida a todos los procesos.\n\n");
    }
    
    // ========================================================================
    // CÁLCULO DE BALANCEO DE CARGA (DISTRIBUCIÓN DE FILAS)
    // ========================================================================
    sendcounts = (int *)malloc(size * sizeof(int));
    displs = (int *)malloc(size * sizeof(int));
    
    if (!sendcounts || !displs) {
        fprintf(stderr, "Proceso %d: Error al asignar memoria para sendcounts/displs\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Calcular cuántas filas le corresponden a cada proceso
    // Distribuimos de forma equitativa, con los primeros procesos recibiendo
    // una fila extra si M no es divisible exactamente por size
    int base_rows = M / size;           // Filas base para cada proceso
    int extra_rows = M % size;          // Filas extras a distribuir
    
    int offset = 0;
    for (int i = 0; i < size; i++) {
        // Los primeros 'extra_rows' procesos reciben una fila adicional
        int rows_for_process = base_rows + (i < extra_rows ? 1 : 0);
        
        sendcounts[i] = rows_for_process * R;  // Elementos de A para proceso i
        displs[i] = offset;                     // Desplazamiento en A
        offset += sendcounts[i];
    }
    
    // Determinar cuántas filas recibirá este proceso
    local_rows = base_rows + (rank < extra_rows ? 1 : 0);
    
    if (rank == 0) {
        printf("Distribución de filas de A entre procesos:\n");
        for (int i = 0; i < size; i++) {
            int rows = sendcounts[i] / R;
            printf("  Proceso %d: %d filas (%d elementos)\n", i, rows, sendcounts[i]);
        }
        printf("\n");
    }
    
    // ========================================================================
    // SCATTER DE FILAS DE A A TODOS LOS PROCESOS
    // ========================================================================
    // Asignar memoria para la porción local de A
    local_A = (double *)malloc(local_rows * R * sizeof(double));
    if (!local_A) {
        fprintf(stderr, "Proceso %d: Error al asignar memoria para local_A\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Distribuir las filas de A usando Scatterv
    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 local_A, local_rows * R, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Filas de A distribuidas a todos los procesos.\n\n");
        start_time = MPI_Wtime();  // Iniciar medición de tiempo
    }
    
    // ========================================================================
    // CÁLCULO LOCAL: CADA PROCESO MULTIPLICA SU PORCIÓN
    // ========================================================================
    // Asignar memoria para la porción local de C
    local_C = (double *)malloc(local_rows * N * sizeof(double));
    if (!local_C) {
        fprintf(stderr, "Proceso %d: Error al asignar memoria para local_C\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Realizar la multiplicación de matrices: local_C = local_A * B
    // Para cada fila i de local_A
    for (int i = 0; i < local_rows; i++) {
        // Para cada columna j de B (y de local_C)
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            // Producto punto de la fila i de local_A con la columna j de B
            for (int k = 0; k < R; k++) {
                sum += local_A[i * R + k] * B[k * N + j];
            }
            local_C[i * N + j] = sum;
        }
    }
    
    // Barrera para asegurar que todos los procesos terminaron el cálculo
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Cálculo local completado en todos los procesos.\n\n");
    }
    
    // ========================================================================
    // GATHER DEL RESULTADO EN EL PROCESO 0
    // ========================================================================
    // Preparar sendcounts y displs para la recolección de C
    // (misma distribución pero con N columnas en lugar de R)
    for (int i = 0; i < size; i++) {
        int rows_for_process = base_rows + (i < extra_rows ? 1 : 0);
        sendcounts[i] = rows_for_process * N;
        displs[i] = (i == 0) ? 0 : displs[i-1] + sendcounts[i-1];
    }
    
    // Recolectar todas las porciones de C en el proceso 0
    MPI_Gatherv(local_C, local_rows * N, MPI_DOUBLE,
                C, sendcounts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    // ========================================================================
    // PROCESO 0: MOSTRAR RESULTADOS
    // ========================================================================
    if (rank == 0) {
        end_time = MPI_Wtime();  // Finalizar medición de tiempo
        
        printf("Resultado recolectado en proceso 0.\n\n");
        
        // Imprimir matriz resultado si es pequeña
        if (M <= 10 && N <= 10) {
            print_matrix(C, M, N, "C = A * B");
        }
        
        printf("==========================================================\n");
        printf("Tiempo de cálculo paralelo: %.6f segundos\n", end_time - start_time);
        printf("==========================================================\n");
        
        // Verificación básica: imprimir algunos valores de C
        if (M > 10 || N > 10) {
            printf("\nPrimeros elementos de C (verificación):\n");
            int max_show = (M < 3 ? M : 3);
            for (int i = 0; i < max_show; i++) {
                printf("  C[%d][0] = %.4f\n", i, C[i * N]);
            }
        }
        
        printf("\nMultiplicación completada exitosamente.\n");
    }
    
    // ========================================================================
    // LIBERACIÓN DE MEMORIA
    // ========================================================================
    if (rank == 0) {
        free(A);
        free(C);
    }
    free(B);
    free(local_A);
    free(local_C);
    free(sendcounts);
    free(displs);
    
    // ========================================================================
    // FINALIZACIÓN DE MPI
    // ========================================================================
    MPI_Finalize();
    
    return 0;
}
