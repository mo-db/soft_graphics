## Blueprint Makefile
SRC_FILES := tex_streaming_01
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

## create vars for target, src and object files
EXE := $(BIN_DIR)/a.out
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SRC_FILES)))

# config for libraries, dont forget to edit upclangd target
BREW_PREFIX := /opt/homebrew/Cellar

SDL3_PREFIX:= $(BREW_PREFIX)/sdl3/3.2.8
SDL3_CFLAGS := -I$(SDL3_PREFIX)/include 
SDL3_LDFLAGS := -L$(SDL3_PREFIX)/lib -lsdl3 

# SDL3_IMG_PREFIX:= $(BREW_PREFIX)/sdl3_image/3.2.0
# SDL3_IMG_CFLAGS := -I$(SDL3_IMG_PREFIX)/include
# SDL3_IMG_LDFLAGS := -L$(SDL3_IMG_PREFIX)/lib -lsdl3_image

# FFMPEG_PREFIX := $(BREW_PREFIX)/ffmpeg/7.1_4
# FFMPEG_CFLAGS := -I$(FFMPEG_PREFIX)/include
# FFMPEG_LDFLAGS := -L$(FFMPEG_PREFIX)/lib -lavcodec

FLAGS := -fsanitize=address -fsanitize=undefined
CFLAGS := $(FLAGS) -Wall -Wextra -g -MMD -MP $(SDL3_CFLAGS) $(FFMPEG_CFLAGS)
LDFLAGS := $(FLAGS) $(SDL3_LDFLAGS) $(FFMPEG_LDFLAGS)

## select compiler
# CC := gcc-14
CC := clang


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
	$(CC) $(LDFLAGS) $^ -o $@
	dsymutil $@

# Only source files that have been changed get rebuilt
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean for rebuilt - Using implicit variable RM (rm -f)
clean:
	@$(RM) -r $(OBJ_DIR) $(BIN_DIR)

# Recreate the .clangd file with the correct include paths
upclangd:
	echo "CompileFlags:" > .clangd
	echo "  Add: [" >> .clangd
	echo "    $(SRC_DIR), $(SDL3_CFLAGS)" >> .clangd
	# echo "    $(SRC_DIR), $(SDL3_CFLAGS), $(SDL3_IMG_CFLAGS), $(FFMPEG_CFLAGS)" >> .clangd
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
