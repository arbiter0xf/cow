SRC_CLIENT := $(wildcard src/client/*.c)
SRC_SERVER := $(wildcard src/server/*.c)

OBJ_CLIENT := $(SRC_CLIENT:.c=.o)
OBJ_SERVER := $(SRC_SERVER:.c=.o)

DEP_CLIENT := $(SRC_CLIENT:.c=.d)
DEP_SERVER := $(SRC_SERVER:.c=.d)

# Produce header dependency files to be included below
CFLAGS := -MMD
COMPILER := gcc
# LDFLAGS :=

EXECUTABLE_CLIENT := cow
EXECUTABLE_SERVER := cows

.PHONY: clean deploy

$(EXECUTABLE_CLIENT): $(OBJ_CLIENT)
	$(COMPILER) -Wall -o $@ $^
#	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS)

$(EXECUTABLE_SERVER): $(OBJ_SERVER)
	$(COMPILER) -Wall -o $@ $^
#	$(COMPILER) -Wall -o $@ $^ $(LDFLAGS)

-include $(DEP)

clean:
	$(RM) $(OBJ) $(DEP) $(EXECUTABLE)

deploy:
	mkdir run_here || true
	mv cow run_here/
	mv cows run_here/
