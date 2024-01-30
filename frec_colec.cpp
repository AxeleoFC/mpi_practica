#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>

std::vector<int> frecuenciaCalcular(const std::vector<int>& valores, const std::vector<int>& numeros, int block_size) {
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
        MPI_Scatter(valor.data(),block_size,MPI_INT, MPI_IN_PLACE,0,MPI_INT, 0,MPI_COMM_WORLD);

        std::vector<int> frecuenciaTotal(tamanioLista);
        std::vector<int> frecuencia_parcial = frecuenciaCalcular(valor, numeros, block_size);

        MPI_Gather(MPI_IN_PLACE,0,MPI_INT, frecuenciaTotal.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

        for (int i = 0; i < frecuencia_parcial.size(); ++i) {
            if(frecuencia_parcial[i]!=0){
                std::printf("Valor=%d Frecuencia=%d\n", valor[i], frecuencia_parcial[i]);
            }
        }
        for (int i = 0; i < frecuenciaTotal.size(); ++i) {
            if(frecuenciaTotal[i]!=0){
                std::printf("Valor=%d Frecuencia=%d\n", valor[i], frecuenciaTotal[i]);
            }
        }

    } else {
        std::vector<int> numeros_locales(tamanioLista);
        MPI_Bcast(numeros_locales.data(), tamanioLista, MPI_INT, 0, MPI_COMM_WORLD);
        std::vector<int> valor_parcial(block_size);
        MPI_Scatter(nullptr,0, MPI_INT, valor_parcial.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

        std::vector<int> frecuencia_parcial = frecuenciaCalcular(valor_parcial, numeros_locales, block_size);
        MPI_Gather(frecuencia_parcial.data(), block_size, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
