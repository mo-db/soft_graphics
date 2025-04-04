## Blueprint Makefile
SRC_FILES := templates
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

## create vars for target, src and object files
EXE := $(BIN_DIR)/a.out
# SRC := $(wildcard $(SRC_DIR)/*.c)
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SRC_FILES)))

# config for libraries, dont forget to edit upclangd target
BREW_PREFIX := /opt/homebrew/Cellar

SDL3_PREFIX:= $(BREW_PREFIX)/sdl3/3.2.10
SDL3_CFLAGS := -I$(SDL3_PREFIX)/include 
SDL3_LDFLAGS := -L$(SDL3_PREFIX)/lib -lsdl3 

FLAGS := -fsanitize=address -fsanitize=undefined
CFLAGS := -Wall -Wextra -g -MMD -MP $(SDL3_CFLAGS)
LDFLAGS := $(SDL3_LDFLAGS)

## select compiler
# CXX := clang
CXX := clang++

## Targets
# Phony targets aren't treated as files
.PHONY: all run clean upclangd

# Default target, executed with 'make' command
build: $(EXE)

# Execute immediatelly after building
run: $(EXE)
	./bin/a.out

# Link all the objectfiles into an exe
$(EXE): $(OBJ) | $(BIN_DIR) # 
	$(CXX) $(LDFLAGS) $^ -o $@
	dsymutil $@

# Only source files that have been changed get rebuilt
# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

# Clean for rebuilt - Using implicit variable RM (rm -f)
clean:
	@$(RM) -r $(OBJ_DIR) $(BIN_DIR)

# Recreate the .clangd file with the correct include paths
upclangd:
	echo "CompileFlags:" > .clangd
	echo "  Add: [" >> .clangd
	echo "    $(SRC_DIR), $(SDL3_CFLAGS)" >> .clangd
	echo "  ]" >> .clangd

# Make sure directories exist
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# check header files for changes
-include $(OBJ:.o=.d)


## Helper Legend
# normal-prerequisites | order-only-prerequisites (no out of date check)

## Automatic variables:
# $^: all prerequisites
# $<: first prerequisite
# $@: target

## Specifics
# -MDD, -MP: create .d files for header deps
# -g: additional debug info gets created
# dsymutil: extract debug info into seperate file, Mac thing I think..
