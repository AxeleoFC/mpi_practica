#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>

std::vector<int> frecuenciaCalcular(std::vector<int> valores, std::vector<int> numeros, int block_size) {
    std::vector<int> frecuencia_p(block_size, 0);
    for (int i = 0; i < block_size; ++i) {
        for (int j = 0; j < numeros.size(); ++j) {
            if (valores[i] == numeros[j]) {
                frecuencia_p[i] += 1;
            }
        }
    }
    return frecuencia_p;
}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int tamanioLista = 16;
    std::vector<int> numeros(tamanioLista);
    int block_size = (tamanioLista) / (nprocs);

    std::vector<int> valor(tamanioLista);
    std::vector<int> frecuencia(tamanioLista);

    if (rank == 0) {
        std::printf("Rank_%d =", rank);
        for (int i = 0; i < tamanioLista; ++i) {
            int numero = rand() % 19;
            std::printf("[%d],", numero);
            numeros[i] = numero;
            valor[i] = i;
        }
        std::printf("\n");
        MPI_Bcast(numeros.data(), tamanioLista, MPI_INT, 0, MPI_COMM_WORLD);
    }
    MPI_Bcast(numeros.data(), tamanioLista, MPI_INT, 0, MPI_COMM_WORLD);
    std::vector<int> valor_p(block_size);
    MPI_Scatter(valor.data(), block_size, MPI_INT, valor_p.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> frecu_p(block_size);
    frecu_p= frecuenciaCalcular(valor_p, numeros, block_size);

    MPI_Gather(frecu_p.data(), block_size, MPI_INT, frecuencia.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank==0){
        for(int i=0;i<frecuencia.size();++i){
            if(frecuencia[i]!=0){
                std::printf("Valor: %d frecuencia: %d\n",valor[i],frecuencia[i]);
            }
        }
    }

    MPI_Finalize();
    return 0;
}
