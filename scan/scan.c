#include <mpi/mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MATRIX_SIZE 8

int size, rank;
double num, tmp, row_sum_i, partial_sum, ans;
MPI_Status status;
MPI_Request reqs[2];
MPI_Status st[2];

double GetUniformRand(double a, double b) {
    return (double)rand() / RAND_MAX * (b - a) + a;
}

void ForwardSending(int col) {
    switch (rank % MATRIX_SIZE) {
        case col:
            MPI_Send(&partial_sum, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            break;
        case col + 1:
            MPI_Recv(&tmp, 1, MPI_DOUBLE, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            row_sum_i += tmp;
            partial_sum += tmp;
            break;
        case MATRIX_SIZE - 1 - col - 1:
            MPI_Recv(&tmp, 1, MPI_DOUBLE, rank + 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            partial_sum += tmp;
            break;
        case MATRIX_SIZE - 1 - col:
            MPI_Send(&partial_sum, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
            break;
    }
}

void Exchange() {
    switch (rank % MATRIX_SIZE) {
        case 3:
            MPI_Isend(&partial_sum, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &reqs[0]);
            MPI_Irecv(&tmp, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &reqs[1]);
            MPI_Waitall(2, reqs, st);
            partial_sum += tmp;
            break;
        case 4:
            MPI_Isend(&partial_sum, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &reqs[0]);
            MPI_Irecv(&tmp, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &reqs[1]);
            MPI_Waitall(2, reqs, st);
            row_sum_i += tmp;
            partial_sum += tmp;
            break;

    }
}

void BackwardSending(int col) {
    switch (rank % MATRIX_SIZE) {
        case col + 1:
            MPI_Send(&partial_sum, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
            break;
        case col:
            MPI_Recv(&tmp, 1, MPI_DOUBLE, rank + 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            partial_sum = tmp;
            break;
        case MATRIX_SIZE - 1 - col:
            MPI_Recv(&tmp, 1, MPI_DOUBLE, rank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            partial_sum = tmp;
            row_sum_i = partial_sum;
            break;
        case MATRIX_SIZE - 1 - col - 1:
            MPI_Send(&partial_sum, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            break;
    }
}

void DownSending() {
    if (rank / MATRIX_SIZE != 0) {
        MPI_Recv(&tmp, 1, MPI_DOUBLE, rank - MATRIX_SIZE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        row_sum_i += tmp;
        partial_sum += tmp;
    }
    if (rank / MATRIX_SIZE != MATRIX_SIZE - 1) {
        MPI_Send(&partial_sum, 1, MPI_DOUBLE, rank + MATRIX_SIZE, 0, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    srand(time(NULL) * rank);

    num = GetUniformRand(0, 10);
    partial_sum = num;
    row_sum_i = num;

    MPI_Barrier(MPI_COMM_WORLD);

    ForwardSending(0);
    ForwardSending(1);
    ForwardSending(2);

    Exchange();

    BackwardSending(3);
    BackwardSending(2);
    BackwardSending(1);

    DownSending();

    printf("process #%d, initial_num = %.2f, sum(0..i) = %.2f\n", rank, num, row_sum_i);
    MPI_Barrier(MPI_COMM_WORLD);
}