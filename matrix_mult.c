#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void print_matrix(double *matrix, int rows, int cols, const char *name) {
    printf("\nMatrix %s (%d x %d):\n", name, rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%.2f ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int M, R, N;
    double *A = NULL, *B = NULL, *C = NULL;
    double *local_A = NULL, *local_C = NULL;
    int *sendcounts = NULL, *displs = NULL;
    int local_rows;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Check command line arguments
    if (argc != 4) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s M R N\n", argv[0]);
            fprintf(stderr, "  M: rows of matrix A\n");
            fprintf(stderr, "  R: columns of A / rows of B\n");
            fprintf(stderr, "  N: columns of matrix B\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Parse dimensions from command line
    M = atoi(argv[1]);
    R = atoi(argv[2]);
    N = atoi(argv[3]);
    
    if (M <= 0 || R <= 0 || N <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: All dimensions must be positive integers\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Process 0 initializes matrices A and B
    if (rank == 0) {
        printf("Matrix multiplication: C(%d x %d) = A(%d x %d) * B(%d x %d)\n", M, N, M, R, R, N);
        printf("Using %d MPI processes\n\n", size);
        
        // Allocate memory for A, B, and C
        A = (double *)malloc(M * R * sizeof(double));
        B = (double *)malloc(R * N * sizeof(double));
        C = (double *)malloc(M * N * sizeof(double));
        
        if (!A || !B || !C) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Initialize matrix A with sample values
        printf("Initializing matrix A...\n");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < R; j++) {
                A[i * R + j] = i + j + 1.0;
            }
        }
        
        // Initialize matrix B with sample values
        printf("Initializing matrix B...\n");
        for (int i = 0; i < R; i++) {
            for (int j = 0; j < N; j++) {
                B[i * N + j] = i * N + j + 1.0;
            }
        }
        
        // Print matrices if they are small enough
        if (M <= 10 && R <= 10 && N <= 10) {
            print_matrix(A, M, R, "A");
            print_matrix(B, R, N, "B");
        }
    }
    
    // Allocate B on all processes
    if (rank != 0) {
        B = (double *)malloc(R * N * sizeof(double));
        if (!B) {
            fprintf(stderr, "Process %d: Memory allocation for B failed\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Broadcast matrix B to all processes
    MPI_Bcast(B, R * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Calculate load balancing: distribute rows of A
    sendcounts = (int *)malloc(size * sizeof(int));
    displs = (int *)malloc(size * sizeof(int));
    
    if (!sendcounts || !displs) {
        fprintf(stderr, "Process %d: Memory allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Calculate how many rows each process gets
    int base_rows = M / size;
    int extra_rows = M % size;
    
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (base_rows + (i < extra_rows ? 1 : 0)) * R;
        displs[i] = (i == 0) ? 0 : displs[i-1] + sendcounts[i-1];
    }
    
    // Calculate local number of rows for this process
    local_rows = base_rows + (rank < extra_rows ? 1 : 0);
    
    // Allocate memory for local portion of A and C
    local_A = (double *)malloc(local_rows * R * sizeof(double));
    local_C = (double *)malloc(local_rows * N * sizeof(double));
    
    if (!local_A || !local_C) {
        fprintf(stderr, "Process %d: Memory allocation for local matrices failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Scatter rows of A to all processes
    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 local_A, local_rows * R, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);
    
    // Each process computes its portion of C
    // local_C = local_A * B
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < N; j++) {
            local_C[i * N + j] = 0.0;
            for (int k = 0; k < R; k++) {
                local_C[i * N + j] += local_A[i * R + k] * B[k * N + j];
            }
        }
    }
    
    // Prepare for gathering results
    int *recvcounts = NULL;
    int *recvdispls = NULL;
    
    if (rank == 0) {
        recvcounts = (int *)malloc(size * sizeof(int));
        recvdispls = (int *)malloc(size * sizeof(int));
        
        if (!recvcounts || !recvdispls) {
            fprintf(stderr, "Process 0: Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        for (int i = 0; i < size; i++) {
            int rows_for_process = base_rows + (i < extra_rows ? 1 : 0);
            recvcounts[i] = rows_for_process * N;
            recvdispls[i] = (i == 0) ? 0 : recvdispls[i-1] + recvcounts[i-1];
        }
    }
    
    // Gather results into C at process 0
    MPI_Gatherv(local_C, local_rows * N, MPI_DOUBLE,
                C, recvcounts, recvdispls, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    // Process 0 prints the result
    if (rank == 0) {
        printf("\nComputation completed!\n");
        
        if (M <= 10 && N <= 10) {
            print_matrix(C, M, N, "C (Result)");
        } else {
            printf("\nResult matrix C is too large to display (%d x %d)\n", M, N);
            printf("Sample of C (first element): C[0][0] = %.2f\n", C[0]);
        }
        
        // Cleanup
        free(A);
        free(C);
        free(recvcounts);
        free(recvdispls);
    }
    
    // Cleanup
    free(B);
    free(local_A);
    free(local_C);
    free(sendcounts);
    free(displs);
    
    MPI_Finalize();
    return 0;
}
