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

    int tamanioLista = 19;
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
        for (int i = 1; i < nprocs; ++i) {
            int inicio = i * block_size;
            if (i == nprocs - 1) {
                int residuo = tamanioLista % nprocs;
                block_size = block_size + residuo;
            }
            MPI_Send(numeros.data(), tamanioLista, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&valor[inicio], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        std::vector<std::vector<int>> frec_parciales(nprocs);

        frec_parciales[0] = frecuenciaCalcular(valor, numeros, (tamanioLista) / (nprocs));

        block_size = (tamanioLista) / (nprocs);
        for (int i = 1; i < nprocs; ++i) {
            if (i == nprocs - 1) {
                int residuo = tamanioLista % nprocs;
                block_size = block_size + residuo;
            }
            std::vector<int> temp_frec(block_size);
            MPI_Recv(temp_frec.data(), block_size, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            frec_parciales[i] = temp_frec;
        }

        int indice=0;
        for(std::vector<int> frec_par : frec_parciales){
            for (int i = 0; i < frec_par.size(); ++i) {
                if(frec_par[i]!=0){
                    std::printf("Valor=%d Frecuencia=%d\n", valor[indice], frec_par[i]);
                }
                indice+=1;
            }
        }

    } else {
        if (rank == nprocs - 1) {
            int residuo = tamanioLista % nprocs;
            block_size = block_size + residuo;
        }
        MPI_Recv(numeros.data(), tamanioLista, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(valor.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::vector<int> frecuencia_parcial = frecuenciaCalcular(valor, numeros, block_size);
        MPI_Send(frecuencia_parcial.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
