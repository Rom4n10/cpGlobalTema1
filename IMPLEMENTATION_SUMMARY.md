# Implementation Summary: MPI Matrix Multiplication

## Overview
This project implements parallel matrix multiplication C(M×N) = A(M×R) × B(R×N) using MPI with SPMD model and 1D domain decomposition.

## Requirements Implemented

All requirements from the problem statement have been successfully implemented:

1. ✅ **Process 0 Initialization**: Process 0 initializes matrices A, B and reads dimensions M, R, N from command line arguments
2. ✅ **MPI_Bcast**: Process 0 broadcasts matrix B to all processes
3. ✅ **MPI_Scatterv**: Process 0 distributes rows of A with load balancing
4. ✅ **Local Computation**: Each process computes its portion of result matrix C
5. ✅ **MPI_Gatherv**: Process 0 collects the final result matrix C
6. ✅ **Command Line Arguments**: Dimensions M, R, N are passed via argv

## Architecture

### SPMD Model
- All processes execute the same program
- Different behavior based on process rank
- Process 0 has additional responsibilities (initialization, I/O)

### 1D Domain Decomposition
- Matrix A is partitioned by rows
- Matrix B is replicated on all processes
- Load balancing: rows distributed evenly with remainder handled

### Load Balancing Strategy
- Base rows per process: M / num_processes
- Extra rows: M % num_processes
- First (M % num_processes) processes get one extra row

## File Structure

```
cpGlobalTema1/
├── matrix_mult.c      # Main implementation
├── Makefile           # Build configuration
├── README.md          # User documentation
└── .gitignore         # Git ignore rules
```

## Code Quality Features

1. **Robust Input Validation**: Uses `strtol()` instead of `atoi()` for proper error detection
2. **Helper Functions**: Eliminates code duplication
   - `parse_positive_int()`: Input validation
   - `calculate_distribution()`: Sendcounts/displacements calculation
   - `get_local_item_count()`: Local row count
3. **Memory Management**: Proper allocation and cleanup
4. **Error Handling**: Comprehensive error messages
5. **No Compiler Warnings**: Clean compilation with `-Wall`

## Testing Results

All tests pass successfully:
- ✅ Square matrices (4×4, 3×3, 2×2)
- ✅ Non-square matrices (10×8 × 8×6, 5×3 × 3×7, 7×2 × 2×2)
- ✅ Various process counts (1, 2, 3, 4)
- ✅ Load balancing scenarios (7 rows, 3 processes)
- ✅ Error handling (invalid inputs, missing arguments)

## Performance Characteristics

- **Communication**: O(R×N) broadcast + O(M×R/P) scatter + O(M×N/P) gather
- **Computation**: O(M×R×N/P) per process
- **Memory**: O(M×R/P + R×N + M×N/P) per process
- **Scalability**: Good speedup for large matrices with P ≤ M

## Usage

```bash
# Compile
make

# Run
mpirun -np <num_processes> ./matrix_mult M R N

# Example
mpirun -np 4 --oversubscribe ./matrix_mult 10 8 6

# Clean
make clean
```

## Verification

Sample verification for 2×2 matrices:
- A = [[1,2], [2,3]]
- B = [[1,2], [3,4]]
- C = [[7,10], [11,16]] ✓

Calculation: C[0][0] = 1×1 + 2×3 = 7 ✓
