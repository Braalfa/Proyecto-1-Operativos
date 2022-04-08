CC		= gcc
CFLAGS	= -lpthread
RM		= rm -f

all: creador cliente reconstructor estadisticas

creador: creador.c
	$(CC) Creador/creador.c -o compilado/creador  $(CFLAGS)

cliente:
	$(CC) Cliente/cliente.c -o compilado/cliente  $(CFLAGS)

reconstructor:
	$(CC) Reconstructor/reconstructor.c -o compilado/reconstructor  $(CFLAGS)

estadisticas:
	$(CC) Estadistico/estadisticas.c -o compilado/estadisticas  $(CFLAGS)

clean veryclean:
	$(RM) creador cliente reconstructor estadisticas