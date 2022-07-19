CC		= gcc
CFLAGS	= -lpthread
RM		= rm -f

all: creadorr clientee reconstructorr estadisticas

creadorr: Creador/creador.c
	$(CC) Creador/creador.c -o compilado/creador  $(CFLAGS)

clientee: Cliente/cliente.c
	$(CC) Cliente/cliente.c -o compilado/cliente  $(CFLAGS)

reconstructorr: Reconstructor/reconstructor.c
	$(CC) Reconstructor/reconstructor.c -o compilado/reconstructor  $(CFLAGS)

estadisticas: Estadistico/estadisticas.c
	$(CC) Estadistico/estadisticas.c -o compilado/estadisticas  $(CFLAGS)

clean veryclean:
	$(RM) creador cliente reconstructor estadisticas
