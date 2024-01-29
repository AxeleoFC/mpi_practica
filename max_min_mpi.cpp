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

//esto es de punto a punto
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int tamanioLista = 12;
    std::vector<int> numeros(tamanioLista);
    //int numeros[tamanioLista];

    int max;
    int min;

    if (rank == 0) {
        std::printf("Rank_%d =", rank);
        for (int i = 0; i < tamanioLista; ++i) {
            int numero = rand() % 1000;
            std::printf("[%d],", numero);
            numeros[i] = numero;
        }
        std::printf("\n");
        int block_size = (tamanioLista / nprocs);
        for (int i = 1; i < nprocs; ++i) {
            int inicio = i * block_size;
            MPI_Send(&numeros[inicio], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int max_parciales[nprocs];
        int min_parciales[nprocs];
        max_parciales[0] = maxNumero(numeros.data(), block_size);
        min_parciales[0] = minNumero(numeros.data(), block_size);
        for (int i = 1; i < nprocs; ++i) {
            MPI_Recv(&max_parciales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::printf("Rank=%d Max parcial=%d\n", i, max_parciales[i]);
            MPI_Recv(&min_parciales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::printf("Rank=%d Min parcial=%d\n", i, min_parciales[i]);
        }
        max = maxNumero(max_parciales, nprocs);
        min = minNumero(min_parciales, nprocs);
        std::printf("Maximo Total=%d\n", max);
        std::printf("Minimo Total=%d\n", min);

    } else {
        int block_size = (tamanioLista / nprocs);

        MPI_Recv(numeros.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::printf("Rank_%d =", rank);

        for (int i = 0; i < block_size; ++i) {
            std::printf("[%d],", numeros[i]);
        }
        std::printf("\n");
        int max_parcial = maxNumero(numeros.data(), block_size);
        std::printf("Max parcial=%d,", max_parcial);
        std::printf("\n");
        MPI_Send(&max_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        int min_parcial = minNumero(numeros.data(), block_size);
        MPI_Send(&min_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}