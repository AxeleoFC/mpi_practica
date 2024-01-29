#include <iostream>
#include <vector>
#include <cstdlib>
#include <mpi.h>

int maxNumero(int* lista, int tamanio) {
    int max = lista[0];
    for (int i = 1; i < tamanio; ++i) {
        if (max < lista[i]) {
            max = lista[i];
        }
    }
    return max;
}

int minNumero(int* lista, int tamanio) {
    int min = lista[0];
    for (int i = 1; i < tamanio; ++i) {
        if (min > lista[i]) {
            min = lista[i];
        }
    }
    return min;
}

std::vector<int> frecuenciaCalcular(const std::vector<int>& valores, const std::vector<int>& numeros, int block_size) {
    std::vector<int> frecuencia_p(block_size, 0);
    for (int i = 0; i < block_size; ++i) {
        for (int j = 0; j < block_size; ++j) {
            if (valores[i] == numeros[j]) {
                frecuencia_p[i] += 1;
            }
        }
    }
    return frecuencia_p;
}

std::vector<int> frecuenciaTotal(const std::vector<std::vector<int>>& frec, int tamanio) {
    std::vector<int> total(tamanio, 0);
    for (const auto& frec_par : frec) {
        for (int i = 0; i < tamanio; ++i) {
            total[i] += frec_par[i];
        }
    }
    return total;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int tamanioLista = 19;
    std::vector<int> numeros(tamanioLista);
    int block_size = (tamanioLista) / (nprocs);

    int max;
    int min;

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
            MPI_Send(&numeros[inicio], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&valor[inicio], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
            std::vector<int> numeros2=numeros;
            MPI_Send(&numeros2, tamanioLista, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int max_parciales[nprocs];
        int min_parciales[nprocs];
        std::vector<std::vector<int>> frec_parciales(nprocs);

        max_parciales[0] = maxNumero(numeros.data(), (tamanioLista) / (nprocs));
        min_parciales[0] = minNumero(numeros.data(), (tamanioLista) / (nprocs));

        frec_parciales[0] = frecuenciaCalcular(valor, numeros, (tamanioLista) / (nprocs));

        for (int i = 1; i < nprocs; ++i) {
            if (i == nprocs - 1) {
                int residuo = tamanioLista % nprocs;
                block_size = block_size + residuo;
            }
            MPI_Recv(&max_parciales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&min_parciales[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::vector<int> temp_frec(block_size);
            MPI_Recv(temp_frec.data(), block_size, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            frec_parciales[i] = temp_frec;
        }
        max = maxNumero(max_parciales, nprocs);
        min = minNumero(min_parciales, nprocs);
        std::printf("Maximo Total=%d\n", max);
        std::printf("Minimo Total=%d\n", min);

        std::vector<int> frecuenciaT = frecuenciaTotal(frec_parciales, tamanioLista);

        

    } else {
        if (rank == nprocs - 1) {
            int residuo = tamanioLista % nprocs;
            block_size = block_size + residuo;
        }
        MPI_Recv(numeros.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(valor.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> numeros2=numeros;
        MPI_Recv(numeros2.data(), tamanioLista, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::printf("Rank_%d =", rank);

        for (int i = 0; i < block_size; ++i) {
            std::printf("[%d],", numeros[i]);
        }
        std::printf("\n");
        int max_parcial = maxNumero(numeros.data(), block_size);
        MPI_Send(&max_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        int min_parcial = minNumero(numeros.data(), block_size);
        MPI_Send(&min_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        std::vector<int> frecuencia_parcial = frecuenciaCalcular(valor, numeros2, block_size);
        for (int i = 0; i < block_size; ++i) {
            std::printf("Valor=%d Frecuencia=%d\n", valor[i], frecuencia_parcial[i]);
        }
        MPI_Send(frecuencia_parcial.data(), block_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
