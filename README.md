gcc reconstructor.c -o reconstructor -lpthread && ./reconstructor
gcc cliente.c -o cliente -lpthread && ./cliente
gcc creador.c -o creador -lpthread && ./creador
gcc estadisticas.c -o estadisticas -lpthread && ./estadisticas


Ver memoria compartida: ipcs
Matar memoria compartida: ipcrm shm memoryid