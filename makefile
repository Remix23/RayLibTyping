EXE := Typing_stuff

C_VERSION := c99

HEADERS := $(shell find headers -maxdepth 1 -name "*.h")
SRC := $(shell find src -maxdepth 1 -name "*.c")

DEVFLAGS := -I ./headers -Wall -Wextra -g -std=$(C_VERSION) -fsanitize=address -lcurl
PRODFLAGS := -I ./headers -O3 -srd=${C_VERSION} -lcurl

$(EXE) : $(SRC) $(HEADERS)
	clang -o "$(EXE)" $(SRC)  $(shell pkg-config --cflags --libs raylib) $(DEVFLAGS)

.PHONY : build run clean

build : $(EXE)

clean :
	rm -f $(EXE)

run : $(EXE)
	./$(EXE)