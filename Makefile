# Compiler and flags
CC = gcc
CC_FLAGS = -Wall -Wextra -O2
LIB_SRC = kittyengine.c
LIB_OBJ = $(LIB_SRC:.c=.o)
TEST_SRC = main.c
TEST_BIN = kit.bin

all: $(LIB_OBJ)

$(LIB_OBJ): ${LIB_SRC}
	$(CC) $(CC_FLAGS) -c $< -o $@ -lm -lSDL2

test: $(LIB_OBJ) $(TEST_SRC)
	$(CC) $(CC_FLAGS) -o $(TEST_BIN) $(TEST_SRC) $(LIB_OBJ) -lm -lSDL2 -lSDL2_ttf
	./$(TEST_BIN)

# Clean up build files
clean:
	rm -f $(LIB_OBJ) $(TEST_BIN)