CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -g
TARGET := sic_assembler
SRC := $(wildcard src/*.c)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.exe
