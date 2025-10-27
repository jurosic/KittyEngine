# Compiler and flags
CC = gcc
CC_FLAGS = -Wall -Wextra -O2


kit: main.c kittyengine.c
	$(CC) $(CC_FLAGS) -o kit.bin $^ -lm -lSDL2

# Clean up build files
clean:
	rm -f $(TARGET)