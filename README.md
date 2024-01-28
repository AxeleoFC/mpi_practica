"# MP2324"
## Intalación de MPI en Ubuntu
sudo apt-get update
sudo apt-get install libopenmpi-dev

## Compilación
mpicxx main.cpp -o main
## Ejecucion
mpiexec -n 4 ./ejemplo03deber