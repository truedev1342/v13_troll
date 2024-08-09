# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -ansi -pedantic -I include -g

# Source files
SRC = src/main.c src/code.c src/command.c src/extra.c src/compiler.c src/template.c

# Output executable
TARGET = assembler

# Default target
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Clean up build files
clean:
	rm -f $(TARGET) *.ent *.ext *.ob *.am
