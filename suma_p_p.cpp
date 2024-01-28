#include <iostream>
#include <mpi.h>

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    const int tamanioLista=12;
    int numeros[tamanioLista];
    int sumaLocal=0;
    int sumaGlobal=0;

    for (int i = 0; i < tamanioLista; ++i) {
        numeros[i]=i+1;
    }

    //
    if(rank!=0){
        int inicio=rank*(tamanioLista/nprocs);
        int fin=(rank+1)*(tamanioLista/nprocs);
        for (int i = inicio; i < fin; ++i) {
            sumaLocal+=numeros[i];
        }
        MPI_Send(&sumaLocal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }else{
        std::printf("Numero de proccesos ===> %d \n", nprocs);
        int inicio=0;
        int fin=(rank+1)*(tamanioLista/nprocs);
        for (int i = inicio; i < fin; ++i) {
            sumaLocal+=numeros[i];
        }
        sumaGlobal+=sumaLocal;
        for (int i = 1; i < nprocs; ++i) {
            MPI_Recv(&sumaLocal, 1, MPI_INT, i,0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            sumaGlobal+=sumaLocal;
        }
        std::printf("Rank_%d suma de numero 1...%d  global===>%d \n",rank,tamanioLista,sumaGlobal);
    }

    MPI_Finalize();
    return  0;
}