#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage ./prog <rows> <cols>\n");
        return 0;
    }

    MPI_Init(&argc, &argv);

    unsigned i, j;

    unsigned rows = atoi(argv[1]), cols = atoi(argv[2]);
    int m[rows][cols];
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            m[i][j] = i + j + 123;
        }
    }

    int mt[cols][rows];
    int procSize = 0, procNum = 0;
    double time;
    

    MPI_Comm_size(MPI_COMM_WORLD, &procSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &procNum);
    //MPI_Bcast(m, rows * cols, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    time = MPI_Wtime();

    int myCols_start = cols / procSize * procNum;
    int myCols_end = cols / procSize * (procNum + 1);

    if (myCols_end > cols) {
        myCols_end = cols;
    }

    for (i = 0; i < rows; ++i) {
        for (j = myCols_start; j < myCols_end; ++j) {
            mt[j][i] = m[i][j];
        }
    }

    if (procNum) {
        MPI_Send(mt[myCols_start], rows * (myCols_end - myCols_start), MPI_INT, 0, 1, MPI_COMM_WORLD);
    } else {
        MPI_Status status[procSize];
        for (i = 1; i < procSize; ++i) {
            int cols_st = cols / procSize * i, cols_en = cols / procSize * (i + 1);
            if (cols_en > cols) cols_en = cols;
            MPI_Recv(mt[cols_st], rows * (cols_en - cols_st), MPI_INT, i, 1, MPI_COMM_WORLD, &status[i]);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (!procNum) {
        time = MPI_Wtime() - time;
        printf("TIME: %lf\n", time);
    }

    for (i = 0; i < cols && rows <= 10 && procNum == 0; ++i) {
        for (j = 0; j < rows - 1; ++j) {
            printf("%d ", mt[i][j]);
        }
        printf("%d\n", mt[i][rows - 1]);
    }
    MPI_Finalize();
    return 0;
}
