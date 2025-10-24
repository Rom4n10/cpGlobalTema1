# Makefile for MPI Matrix Multiplication

CC = mpicc
CFLAGS = -Wall -O2
TARGET = matrix_mult
SOURCE = matrix_mult.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

# Run with default parameters (4x4 matrices, 2 processes)
test: $(TARGET)
	mpirun -np 2 --allow-run-as-root --oversubscribe ./$(TARGET) 4 4 4

# Run with larger matrices (10x8 * 8x6, 4 processes)
test-large: $(TARGET)
	mpirun -np 4 --allow-run-as-root --oversubscribe ./$(TARGET) 10 8 6

# Run with non-square matrices (5x3 * 3x7, 3 processes)
test-nonsquare: $(TARGET)
	mpirun -np 3 --allow-run-as-root --oversubscribe ./$(TARGET) 5 3 7

.PHONY: all clean test test-large test-nonsquare
