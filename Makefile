SRC_CLIENT := $(wildcard src/client/*.c)
SRC_SERVER := $(wildcard src/server/*.c)
SRC_COMMON := $(wildcard src/common/*.c)
SRC_UTEST := $(wildcard test/utest/*.c)
SRC_ITEST := $(wildcard test/itest/*.c)

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
CFLAGS := -DLOCAL_SERVER_CONFIG=1 -DDEBUG_ENABLED=1 -MMD -Iinclude/
CFLAGS_UTEST := -DLOCAL_SERVER_CONFIG=1 -DDEBUG_ENABLED=0 -Iinclude/ -Wall
CFLAGS_ITEST := -DLOCAL_SERVER_CONFIG=1 -DDEBUG_ENABLED=0 -Iinclude/ -Wall
COMPILER := gcc
LDFLAGS_SERVER := -lssl -lcrypto -lpthread
LDFLAGS_CLIENT := -lssl -lcrypto
LDFLAGS_UTEST := -lpthread -lcriterion
LDFLAGS_ITEST := -lssl -lcrypto -lpthread -lcriterion

EXECUTABLE_CLIENT := run_here/cow
EXECUTABLE_SERVER := run_here/cows
EXECUTABLE_UTEST := run_here/utest
EXECUTABLE_ITEST := run_here/itest

OUT_DIR := run_here/

.PHONY: clean client deploy prepare_out server utest

client: $(OUT_DIR) $(EXECUTABLE_CLIENT)

server: $(OUT_DIR) $(EXECUTABLE_SERVER)

utest: $(OUT_DIR) $(EXECUTABLE_UTEST)

itest: $(OUT_DIR) $(EXECUTABLE_ITEST)

$(EXECUTABLE_CLIENT): $(OBJ_CLIENT) $(OBJ_COMMON)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS_CLIENT)

$(EXECUTABLE_SERVER): $(OBJ_SERVER) $(OBJ_COMMON)
	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS_SERVER)

$(EXECUTABLE_UTEST): $(SRC_UTEST) $(SRC_WITHOUT_MAIN)
	$(COMPILER) $(CFLAGS_UTEST) -o $@ $^ $(LDFLAGS_UTEST)

$(EXECUTABLE_ITEST): $(SRC_ITEST) $(SRC_WITHOUT_MAIN)
	$(COMPILER) $(CFLAGS_ITEST) -o $@ $^ $(LDFLAGS_ITEST)

$(OUT_DIR):
	test -d run_here/ || mkdir run_here/

-include $(DEP)

clean:
	$(RM) $(OBJ) $(DEP) $(EXECUTABLE_CLIENT) $(EXECUTABLE_SERVER)
	$(RM) $(EXECUTABLE_UTEST) $(EXECUTABLE_ITEST)

deploy:
	test -d run_here/ || mkdir run_here/
	mv scripts/*.key run_here/ || true
	mv scripts/*.crt run_here/ || true
