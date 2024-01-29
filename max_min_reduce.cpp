#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>

int maxNumero(int* lista, int tamanio){
    int max=lista[0];
    for (int i = 1; i < tamanio; ++i) {
        if(max<lista[i]){
            max=lista[i];
        }
    }
    return max;
}

int minNumero(int* lista, int tamanio){
    int min=lista[0];
    for (int i = 1; i < tamanio; ++i) {
        if(min>lista[i]){
            min=lista[i];
        }
    }
    return min;
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int tamanioLista=12;
    std::vector<int> numeros(tamanioLista);

    int max;
    int min;

    if(rank==0){
        std::printf("Rank_%d =",rank);
        numeros[0]=1;
        for (int i = 1; i < tamanioLista; ++i) {
            int numero = rand() % 1000;
            numeros[i]=numero;
        }
        for (int i = 0; i < tamanioLista; ++i) {
            std::printf("[%d],",numeros[i]);
        }
        std::printf("\n");
    }

    int block_size=(tamanioLista/nprocs);
    std::vector<int> numeros_locales(block_size);
    MPI_Scatter(numeros.data(), block_size, MPI_INT, numeros_locales.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    int max_parcial= maxNumero(numeros_locales.data(),block_size);
    int min_parcial= minNumero(numeros_locales.data(),block_size);

    MPI_Reduce(&max_parcial, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&min_parcial, &min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if(rank==0){
        std::printf("Maximo Total=%d\n",max);
        std::printf("Minimo Total=%d\n",min);
    }

    MPI_Finalize();
    return  0;
}