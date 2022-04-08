CC		= gcc
CFLAGS	= -lpthread
RM		= rm -f

all: creador cliente reconstructor estadisticas

creador: Creador/creador.c
	$(CC) Creador/creador.c -o compilado/creador  $(CFLAGS)

cliente: Cliente/cliente.c
	$(CC) Cliente/cliente.c -o compilado/cliente  $(CFLAGS)

reconstructor: Reconstructor/reconstructor.c
	$(CC) Reconstructor/reconstructor.c -o compilado/reconstructor  $(CFLAGS)

estadisticas: Estadistico/estadisticas.c
	$(CC) Estadistico/estadisticas.c -o compilado/estadisticas  $(CFLAGS)

clean veryclean:
	$(RM) creador cliente reconstructor estadisticas
