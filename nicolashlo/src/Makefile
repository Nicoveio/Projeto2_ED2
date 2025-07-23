PROJ_NAME = ted

CC = gcc
CFLAGS = -g -O0 -std=c99 -fstack-protector-all -Werror=implicit-function-declaration -Wall
LDFLAGS = 
LIBS = -lm

SRCS = main.c graph.c hash.c priority_queue.c lista.c fila.c smutreap.c utils.c via.c geo.c svg.c

OBJETOS = $(SRCS:.c=.o)

# Regra principal
$(PROJ_NAME): $(OBJETOS)
	@echo "--- Linkando o executável de teste: $(PROJ_NAME) ---"
	$(CC) -o $(PROJ_NAME) $(OBJETOS) $(LIBS) $(LDFLAGS)
	@echo "--- Compilação concluída com sucesso! ---"


%.o: %.c
	@echo "Compilando $<..."
	$(CC) -c $(CFLAGS) $< -o $@


lista.o: lista.c lista.h
fila.o: fila.c fila.h
utils.o: utils.c utils.h

hash.o: hash.c hash.h utils.h
priority_queue.o: priority_queue.c priority_queue.h
smutreap.o: smutreap.c smutreap.h fila.h lista.h

graph.o: graph.c graph.h lista.h hash.h priority_queue.h smutreap.h utils.h
via.o: via.c via.h graph.h utils.h
geo.o: geo.c geo.h lista.h
svg.o: svg.c svg.h graph.h lista.h

main.o: main.c utils.h via.h graph.h lista.h geo.h svg.h

clean:
	rm -f $(OBJETOS) $(PROJ_NAME)