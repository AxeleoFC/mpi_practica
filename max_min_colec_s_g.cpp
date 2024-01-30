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

//
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
        for (int i = 0; i < tamanioLista; ++i) {
            int numero = rand() % 1000;
            std::printf("[%d],",numero);
            numeros[i]=numero;
        }
        std::printf("\n");
        int block_size=(tamanioLista/nprocs);
        // envio los datos desde rank 0 a los 4 datos
        MPI_Scatter(numeros.data(), block_size, MPI_INT, MPI_IN_PLACE, 0, MPI_INT, 0, MPI_COMM_WORLD);

        int max_parciales[nprocs];
        int min_parciales[nprocs];
        max_parciales[0]=maxNumero(numeros.data(), block_size);
        min_parciales[0]= minNumero(numeros.data(),block_size);

        MPI_Gather(MPI_IN_PLACE, 0, MPI_INT, max_parciales, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(MPI_IN_PLACE, 0, MPI_INT, min_parciales, 1, MPI_INT, 0, MPI_COMM_WORLD);

        max= maxNumero(max_parciales, nprocs);
        min= minNumero(min_parciales, nprocs);
        std::printf("Maximo Total=%d\n",max);
        std::printf("Minimo Total=%d\n",min);

    }else{
        int block_size=(tamanioLista/nprocs);
        std::vector<int> numeros_parciales(block_size);
        MPI_Scatter(nullptr, 0, MPI_INT, numeros_parciales.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);
        std::printf("Rank_%d =",rank);

        for (int i = 0; i < block_size; ++i) {
            std::printf("[%d],",numeros_parciales[i]);
        }
        std::printf("\n");
        int max_parcial= maxNumero(numeros_parciales.data(), block_size);
        int min_parcial= minNumero(numeros_parciales.data(),block_size);

        MPI_Gather(&max_parcial, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(&min_parcial, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);

    }

    MPI_Finalize();
    return  0;
}