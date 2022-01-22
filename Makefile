SRC_CLIENT := $(wildcard src/client/*.c)
SRC_SERVER := $(wildcard src/server/*.c)
SRC_COMMON := $(wildcard src/common/*.c)
SRC_TEST := $(wildcard test/*.c)

OBJ_CLIENT := $(SRC_CLIENT:.c=.o)
OBJ_SERVER := $(SRC_SERVER:.c=.o)
OBJ_COMMON := $(SRC_COMMON:.c=.o)

DEP_CLIENT := $(SRC_CLIENT:.c=.d)
DEP_SERVER := $(SRC_SERVER:.c=.d)
DEP_COMMON := $(SRC_COMMON:.c=.d)

SRC := $(SRC_CLIENT) $(SRC_SERVER) $(SRC_COMMON)
OBJ := $(OBJ_CLIENT) $(OBJ_SERVER) $(OBJ_COMMON)
DEP := $(DEP_CLIENT) $(DEP_SERVER) $(DEP_COMMON)

SRC_WITHOUT_MAIN := $(filter-out src/server/main.c,$(filter-out src/client/main.c,$(SRC)))

# -MMD -> Produce header dependency files to be included below
CFLAGS := -DDEBUG_ENABLED=1 -MMD -Iinclude/
CFLAGS_TEST := -DDEBUG_ENABLED=0 -Iinclude/ -Wall
COMPILER := gcc
LDFLAGS_SERVER := -lssl -lcrypto -lpthread
LDFLAGS_CLIENT := -lssl -lcrypto
LDFLAGS_TEST := -lpthread -lcriterion

EXECUTABLE_CLIENT := cow
EXECUTABLE_SERVER := cows
EXECUTABLE_TEST := utest

.PHONY: clean deploy

$(EXECUTABLE_CLIENT): $(OBJ_CLIENT) $(OBJ_COMMON)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS_CLIENT)

$(EXECUTABLE_SERVER): $(OBJ_SERVER) $(OBJ_COMMON)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS_SERVER)

$(EXECUTABLE_TEST): $(SRC_TEST) $(SRC_WITHOUT_MAIN)
	$(COMPILER) $(CFLAGS_TEST) -o $@ $^ $(LDFLAGS_TEST)

-include $(DEP)

clean:
	$(RM) $(OBJ) $(DEP) $(EXECUTABLE_CLIENT) $(EXECUTABLE_SERVER)

deploy:
	mkdir run_here || true
	mv cow run_here/ || true
	mv cows run_here/ || true
	mv utest run_here/ || true
	mv scripts/*.key run_here/ || true
	mv scripts/*.crt run_here/ || true
