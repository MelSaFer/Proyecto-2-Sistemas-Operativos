# Flags de compilación
CFLAGS = -Wall -g

# Bibliotecas necesarias
LIBS = -lpthread -lrt

# Nombres de los ejecutables
EXECUTABLES = starter_ finisher_ producer_ spy_

# Regla por defecto
all: $(EXECUTABLES)

# Reglas para compilar cada ejecutable
starter_: starter/starter.c include/thread.h include/mempartition.h include/sharedMem.h
	gcc $(CFLAGS) -o starter_ starter/starter.c $(LIBS)

finisher_: finisher/finisher.c
	gcc $(CFLAGS) -o finisher_ finisher/finisher.c $(LIBS)

producer_: producer/producer.c include/thread.h include/mempartition.h include/sharedMem.h
	gcc $(CFLAGS) -o producer_ producer/producer.c $(LIBS)

spy_: spy/spy.c include/thread.h include/mempartition.h include/sharedMem.h
	gcc $(CFLAGS) -o spy_ spy/spy.c $(LIBS)

# Regla para limpiar archivos objeto y ejecutables
clean:
	rm -f $(EXECUTABLES)
