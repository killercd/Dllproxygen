ifeq ($(origin CC),default)
CC := x86_64-w64-mingw32-gcc
endif
CFLAGS_COMMON := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Iinclude
CFLAGS ?= $(CFLAGS_COMMON) -O2
SRC := src/main.c src/model.c src/pe_reader.c src/import_parser.c src/export_parser.c src/generator.c src/report.c
OBJ := $(SRC:.c=.o)

.PHONY: all debug clean test
all: dllproxygen.exe
dllproxygen.exe: $(OBJ)
	$(CC) $(OBJ) -o $@
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
debug: CFLAGS := $(CFLAGS_COMMON) -g -O0
debug: clean dllproxygen.exe
test:
	$(MAKE) -C tests CC="$(CC)"
clean:
ifeq ($(OS),Windows_NT)
	-cmd /C "del /Q src\\*.o dllproxygen.exe 2>NUL"
else
	$(RM) $(OBJ) dllproxygen.exe
endif
