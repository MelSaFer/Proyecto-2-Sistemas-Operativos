# Flags de compilación
CFLAGS = -Wall -g

# Bibliotecas necesarias
LIBS = -lpthread -lrt

# Nombres de los ejecutables
EXECUTABLES = starter_ finisher_

# Regla por defecto
all: $(EXECUTABLES)

# Reglas para compilar cada ejecutable
starter_: starter/starter.c
	gcc $(CFLAGS) -o starter_ starter/starter.c $(LIBS)

finisher_: finisher/finisher.c
	gcc $(CFLAGS) -o finisher_ finisher/finisher.c $(LIBS)

# Regla para limpiar archivos objeto y ejecutables
clean:
	rm -f $(EXECUTABLES)
