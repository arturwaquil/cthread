#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
#

CC=gcc -c
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
CFLAGS=-Wall -g

#LIB=$(LIB_DIR)/libcthread.a

all: $(BIN_DIR)/cthread.o
	ar -crs $(LIB) $^ $(BIN_DIR)/support.o

$(BIN_DIR)/cthread.o: $(SRC_DIR)/cthread.c
	$(CC) -o $@ $< -I$(INC_DIR) $(CFLAGS)


tar:
	@cd .. && tar -zcvf 274693.tar.gz cthread

clean:
	mv $(BIN_DIR)/support.o ../
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~
	mv ../support.o $(BIN_DIR)
