#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>

int max(std::vector<int> lista){
    int max=lista[0];
    for (int i = 1; i < lista.size(); ++i) {
        if(max<lista[i]){
            max=lista[i];
        }
    }
    return max;
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int tamanioLista=12;
    int numeros[tamanioLista];

    int max;

    if(rank==0){
        std::printf("Rank_%d =",rank);
        for (int i = 0; i < tamanioLista; ++i) {
            int numero = rand() % 1000;
            std::printf("[%d],",numero);
            numeros[i]=numero;
        }
        std::printf("\n");
        int block_size=(tamanioLista/nprocs);
        for (int i = 1; i < nprocs; ++i) {
            int inicio=i*block_size;
            MPI_Send(&numeros[inicio], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

    }else{
        int block_size=(tamanioLista/nprocs);
        MPI_Recv(&numeros,block_size, MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        std::printf("Rank_%d =",rank);

        for (int i = 0; i < block_size; ++i) {
            std::printf("[%d],",numeros[i]);
        }
        std::printf("\n");

    }

    MPI_Finalize();
    return  0;
}