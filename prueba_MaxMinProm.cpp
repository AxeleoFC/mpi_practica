#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>
#include <fstream>
#include <string>

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

int promNumero(int* lista, int tamanio){
    int prom=lista[0];
    for (int i = 1; i < tamanio; ++i) {
        prom=lista[i]+prom;
    }
    return prom/tamanio;
}

std::vector<int> read_file() {
    std::fstream fs("./datos.txt", std::ios::in );
    std::string line;
    std::vector<int> ret;
    while( std::getline(fs, line) ){
        ret.push_back( std::stoi(line) );
    }
    fs.close();
    return ret;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> numeros=read_file();
    int tamanioLista;
    if (rank == 0) {
        numeros=read_file();
        tamanioLista = numeros.size();
        MPI_Bcast(&tamanioLista, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    MPI_Bcast(&tamanioLista, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int max;
    int min;
    int prom;

    if (rank == 0) {
        for (int i = 1; i < nprocs; ++i) {
            int block_size= (tamanioLista)/(nprocs);
            int inicio=i*block_size;
            if(i==nprocs-1){
                int residuo = tamanioLista % nprocs;
                block_size=block_size+residuo;
            }
            MPI_Send(&numeros[inicio], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int max_parciales[2];
        int min_parciales[2];
        int prom_parciales[2];
        max_parciales[0] = maxNumero(numeros.data(),  (tamanioLista)/(nprocs));
        min_parciales[0] = minNumero(numeros.data(),  (tamanioLista)/(nprocs));
        prom_parciales[0] = promNumero(numeros.data(),  (tamanioLista)/(nprocs));

        MPI_Reduce(&max_parciales[0], &max_parciales[1], 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&min_parciales[0], &min_parciales[1], 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&prom_parciales[0], &prom_parciales[1], 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        std::printf("Maximo Total=%d\n",max_parciales[1]);
        std::printf("Minimo Total=%d\n",min_parciales[1]);
        std::printf("Promedio Total=%d\n",prom_parciales[1]/nprocs);

    } else {
        int block_size= (tamanioLista)/(nprocs);
        if (rank==nprocs-1){
            int residuo = tamanioLista % nprocs;
            block_size=block_size+residuo;
        }
        MPI_Recv(numeros.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int max_parcial = maxNumero(numeros.data(), block_size);
        int min_parcial = minNumero(numeros.data(), block_size);
        int prom_parcial = promNumero(numeros.data(), block_size);

        MPI_Reduce(&max_parcial, nullptr, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&min_parcial, nullptr, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&prom_parcial, nullptr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }


    MPI_Finalize();
    return 0;
}


