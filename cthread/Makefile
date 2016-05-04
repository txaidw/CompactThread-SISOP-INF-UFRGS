#
# Makefile ESQUELETO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "fila2.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: fila2.o cdata.o cthread.o libcthread

fila2.o:
	$(CC) -c $(SRC_DIR)/fila2.c -o $(BIN_DIR)/fila2.o -Wall

cdata.o:
	$(CC) -c $(SRC_DIR)/cdata.c -o $(BIN_DIR)/cdata.o -Wall

cthread.o:
	$(CC) -c $(SRC_DIR)/cthread.c -o $(BIN_DIR)/cthread.o -Wall

libcthread: fila2.o cdata.o cthread.o
	ar crs $(LIB_DIR)/libcthread.a $(BIN_DIR)/fila2.o $(BIN_DIR)/cdata.o $(BIN_DIR)/cthread.o

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~
