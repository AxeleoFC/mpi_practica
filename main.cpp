#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>
#include <cmath>

#define CANT 17

std::vector<int> frecuenciaCalcular(std::vector<int> valores, std::vector<int> numeros, int block_size, int resi) {
    std::vector<int> frecuencia_p(block_size, 0);
    for (int i = 0; i < block_size; ++i) {
        for (int j = 0; j < numeros.size()-resi; ++j) {
            if (valores[i] == numeros[j]) {
                frecuencia_p[i] += 1;
            }
        }
    }
    std::printf("\n");
    return frecuencia_p;
}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int rows_per_rank;
    int rows_alloc = CANT;
    int padding = 0;

    if (CANT % nprocs != 0) {
        rows_alloc = std::ceil((double) CANT / nprocs) * nprocs;
        padding = rows_alloc - CANT;
    }
    rows_per_rank = rows_alloc / nprocs;


    std::vector<int> numeros(rows_alloc);

    std::vector<int> valor(rows_alloc);
    std::vector<int> frecuencia(rows_alloc);

    if (rank == 0) {
        std::printf("Rank_%d =", rank);
        for (int i = 0; i <= CANT; ++i) {
            int numero = rand() % CANT;
            numeros[i] = numero;
            valor[i] = i;
        }
        numeros[0]=16;
        for (int i = 0; i <= CANT; ++i) {
            std::printf("[%d],", numeros[i]);
        }
        std::printf("\n");
        MPI_Bcast(numeros.data(), rows_alloc, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Bcast(numeros.data(), rows_alloc, MPI_INT, 0, MPI_COMM_WORLD);
    std::vector<int> valor_p(rows_per_rank);
    MPI_Scatter(valor.data(), rows_per_rank, MPI_INT, valor_p.data(), rows_per_rank, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> frecu_p(rows_per_rank);

    if(rank==nprocs-1){
        frecu_p= frecuenciaCalcular(valor_p, numeros, rows_per_rank-padding, padding);
    }else{
        frecu_p= frecuenciaCalcular(valor_p, numeros, rows_per_rank, padding);
    }

    MPI_Gather(frecu_p.data(), rows_per_rank, MPI_INT, frecuencia.data(), rows_per_rank, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank==0){
        for(int i=0;i<frecuencia.size()-padding;++i){
            if(frecuencia[i]!=0){
                std::printf("Valor: %d frecuencia: %d\n",valor[i],frecuencia[i]);
            }
        }
    }

    MPI_Finalize();
    return 0;
}
