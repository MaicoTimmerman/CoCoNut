# -------------------- Configurable options  ---------------------
CC           := gcc
CFLAGS       := -Wall -std=gnu11 -g -Og -pedantic -MMD \
				-Werror=implicit-function-declaration

AST_FILE	  			= test/pass/civic.ast
BIN_DIR 	  			= bin/
AST_GENERATED_SOURCES 	= src/generated/
AST_GENERATED_HEADERS	= include/generated/
SOURCES   	  			= src/core/
TARGET 	 	  			= civcc


# ----------------------- Other variables --------------------

LIB_SOURCES   			= src/lib/
LIB_SRC       = $(foreach dir,$(LIB_SOURCES),$(wildcard $(dir)*.c))

AST_GENERATED_SRC_GENFILE 		= $(AST_GENERATED_SOURCES).gen
AST_GENERATED_INC_GENFILE 		= $(AST_GENERATED_HEADERS).gen


AST_GEN_SOURCES = src/astgen/
AST_SOURCES   = $(LIB_SOURCES) $(AST_GEN_SOURCES)
AST_SRC       = $(foreach dir,$(AST_SOURCES),$(wildcard $(dir)*.c))
AST_PARSER    = src/astgen/ast.parser.c
AST_LEXER     = src/astgen/ast.lexer.c
AST_TARGET    = astgen

SRC           = $(foreach dir,$(SOURCES),$(wildcard $(dir)*.c))

COLOR_GREEN	  = "\\e[1m\\e[32m"
COLOR_RESET	  = "\\e[0m"

TARGET_BIN 		= $(BIN_DIR)$(TARGET)
AST_TARGET_BIN  = $(BIN_DIR)$(AST_TARGET)

# ----------------------- Compiler rules --------------------


.PHONY: all clean test format doc

all: $(TARGET_BIN) ;

$(TARGET_BIN): $(SRC:.c=.o) $(LIB_SRC:.c=.o)
	@echo -e "$(COLOR_GREEN) LINK$(COLOR_RESET)      $@"
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $(SRC:.c=.o) $(AST_GENERATED_SRC:.c=.o) $(LIB_SRC:.c=.o)

%.o: %.c $(AST_GENERATED_SRC_GENFILE)
	@echo -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	@$(CC) $(CFLAGS) -I include/ -o $@ -c $<

# ----------------------- AST-gen rules --------------------

$(AST_GENERATED_SRC_GENFILE): $(AST_GENERATED_INC_GENFILE) ;

$(AST_GENERATED_INC_GENFILE): $(AST_TARGET_BIN) $(AST_FILE)
	@echo -e "$(COLOR_GREEN) ASTGEN$(COLOR_RESET)    $(AST_FILE)"
	@$(AST_TARGET_BIN) --source-dir $(AST_GENERATED_SOURCES) \
		--header-dir $(AST_GENERATED_HEADERS) $(AST_FILE)
	@touch $(AST_GENERATED_INC_GENFILE) $(AST_GENERATED_SRC_GENFILE)

$(AST_TARGET_BIN): $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC:.c=.o)
	@echo -e "$(COLOR_GREEN) LINK$(COLOR_RESET)      $@"
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC:.c=.o)

$(LIB_SOURCES)%.o: $(LIB_SOURCES)%.c
	@echo -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	@$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_GEN_SOURCES)%.o: $(AST_GEN_SOURCES)%.c
	@echo -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	@$(CC) $(CFLAGS) -I include/ -o $@ -c $<

%.lexer.h: %.lexer.c
	@echo -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $@"

%.lexer.c: %.l
	@echo -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $@"
	@flex -o $@ --header-file=$(AST_LEXER:.c=.h) $<

%.parser.c: %.y %.lexer.h
	@echo -e "$(COLOR_GREEN) BISON$(COLOR_RESET)     $@"
	@bison -dv -o $@ $<


# ----------------------- Other rules --------------------
clean:
	@echo -e "$(COLOR_GREEN) CLEANING$(COLOR_RESET)"
	@rm -f $(AST_TARGET_BIN) $(TARGET_BIN)
	@rm -f $(AST_LEXER) $(AST_LEXER:.c=.h) \
		$(AST_PARSER) $(AST_PARSER:.c=.output) \
		$(AST_PARSER:.c=.h)
	@find . -type f -name '*.o' -exec rm {} \;
	@find . -type f -name '*.d' -exec rm {} \;
	@echo -e "$(COLOR_GREEN) DONE$(COLOR_RESET)"

doc:
	$(MAKE) --directory doc html

test: $(AST_TARGET_BIN)
	@test/test.sh test

format:
	@echo "Applying clang-format on all files"
	@find . -name "*.h" -o -name "*.c" | xargs -n1 clang-format -i

-include $(AST_SRC:.c=.d)
-include $(SRC:.c=.d)
