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

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./tests
EXP_DIR=./exemplos

all: e4

# test_all: ccreate
# 	$(CC) -o $(BIN_DIR)/ccreate_test $(SRC_DIR)/cthread.c -Wall
e1:
	$(CC) -o ./exec ${EXP_DIR}/exemplo.c $(SRC_DIR)/cthread.c $(BIN_DIR)/support.o -Wall
e2:
	$(CC) -o ./exec ${EXP_DIR}/mandel.c $(SRC_DIR)/cthread.c $(BIN_DIR)/support.o -Wall
e3:
	$(CC) -o ./exec ${EXP_DIR}/series.c $(SRC_DIR)/cthread.c $(BIN_DIR)/support.o -Wall
e4:
	$(CC) -o ./exec ${EXP_DIR}/teste_vetor.c $(SRC_DIR)/cthread.c $(BIN_DIR)/support.o -Wall
ccreate_test:
	$(CC) -o ./exec ${TST_DIR}/ccreate.c $(SRC_DIR)/cthread.c $(BIN_DIR)/support.o -Wall
cyield_test:
	$(CC) -o ./exec ${TST_DIR}/cyield.c $(SRC_DIR)/cthread.c $(BIN_DIR)/support.o -Wall

# ccreate_test:
# 	$(CC) -o ./ccreate_test ${TST_DIR}/ccreate.c $(SRC_DIR)/cthread.c $(BIN_DIR)/support.o -Wall

# ccreate: cthread
# 	$(CC) -o $(BIN_DIR)/ccreate $(TST_DIR)/ccreate.c -Wall
#
# cthread: #dependências para a regran
# 	$(CC) -o $(BIN_DIR)/cthread $(SRC_DIR)/cthread.c -Wall

clean:
	rm -rf $(LIB_DIR)/*.a ./exec $(SRC_DIR)/*~ $(INC_DIR)/*~ *~
