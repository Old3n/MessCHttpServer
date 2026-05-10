TARGET = bin/http-server
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

.PHONY: run default clean

run: clean default
	./$(TARGET) ./http-server

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*

$(TARGET): $(OBJ)
	gcc -o $@ $^

obj/%.o: src/%.c
	gcc -g -c $< -o $@ -Iinclude
