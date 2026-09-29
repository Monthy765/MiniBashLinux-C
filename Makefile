# Compilador
CC = gcc

# Flags de compilación
CFLAGS = -Wall -Wextra -Iinclude

# Archivos fuente
SRC = src/myshell.c src/jobs.c

# Archivos objeto
OBJ = $(SRC:.c=.o)

# Librerías
LIBS = lib/libparser.a

# Ejecutable final
OUT = build/myshell

# Regla principal
all: $(OUT)

# Cómo construir el ejecutable
$(OUT): $(OBJ)
    $(CC) $(CFLAGS) $(OBJ) $(LIBS) -o $(OUT) -static

# Cómo compilar cada .c a .o
src/%.o: src/%.c
    $(CC) $(CFLAGS) -c $< -o $@

# Limpiar binarios y objetos
clean:
    rm -f $(OBJ) $(OUT)

# Limpiar todo (incluye build/)
distclean: clean
    rm -rf build/*
