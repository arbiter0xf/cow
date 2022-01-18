SRC_CLIENT := $(wildcard src/client/*.c)
SRC_SERVER := $(wildcard src/server/*.c)

OBJ_CLIENT := $(SRC_CLIENT:.c=.o)
OBJ_SERVER := $(SRC_SERVER:.c=.o)

DEP_CLIENT := $(SRC_CLIENT:.c=.d)
DEP_SERVER := $(SRC_SERVER:.c=.d)

# Produce header dependency files to be included below
CFLAGS := -MMD
COMPILER := gcc
LDFLAGS_SERVER := -lssl -lcrypto
LDFLAGS_CLIENT := -lssl -lcrypto

EXECUTABLE_CLIENT := cow
EXECUTABLE_SERVER := cows

.PHONY: clean deploy

$(EXECUTABLE_CLIENT): $(OBJ_CLIENT)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS_CLIENT)

$(EXECUTABLE_SERVER): $(OBJ_SERVER)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS_SERVER)

-include $(DEP)

clean:
	$(RM) $(OBJ) $(DEP) $(EXECUTABLE)

deploy:
	mkdir run_here || true
	mv cow run_here/ || true
	mv cows run_here/ || true
	mv scripts/*.key run_here/ || true
	mv scripts/*.crt run_here/ || true
