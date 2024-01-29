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

    int tamanioLista = 15;
    std::vector<int> numeros(tamanioLista);

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
        for (int i = 1; i < nprocs; ++i) {
            int block_size= (tamanioLista)/(nprocs);
            int inicio=i*block_size;
            if(i==nprocs-1){
                int residuo = tamanioLista % nprocs;
                block_size=block_size+residuo;
            }
            MPI_Send(&numeros[inicio], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int max_parciales[nprocs];
        int min_parciales[nprocs];
        max_parciales[0] = maxNumero(numeros.data(),  (tamanioLista)/(nprocs));
        min_parciales[0] = minNumero(numeros.data(),  (tamanioLista)/(nprocs));
        for (int i = 1; i < nprocs; ++i) {
            MPI_Recv(&max_parciales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&min_parciales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        max = maxNumero(max_parciales, nprocs);
        min = minNumero(min_parciales, nprocs);
        std::printf("Maximo Total=%d\n", max);
        std::printf("Minimo Total=%d\n", min);

    } else {
        int block_size= (tamanioLista)/(nprocs);
        if (rank==nprocs-1){
            int residuo = tamanioLista % nprocs;
            block_size=block_size+residuo;
        }
        MPI_Recv(numeros.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::printf("Rank_%d =", rank);

        for (int i = 0; i < block_size; ++i) {
            std::printf("[%d],", numeros[i]);
        }
        std::printf("\n");
        int max_parcial = maxNumero(numeros.data(), block_size);
        MPI_Send(&max_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        int min_parcial = minNumero(numeros.data(), block_size);
        MPI_Send(&min_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}