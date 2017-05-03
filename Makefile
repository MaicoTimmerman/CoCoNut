# -------------------- Configurable options  ---------------------
CC           := gcc
CFLAGS       := -Wall -std=gnu11 -g -Og -pedantic -MMD \
				-Werror=implicit-function-declaration

AST_FILE	  			= test/pass/civic.ast
BIN_DIR 	  			= bin/
DOC_DIR 	  			= doc/
AST_GENERATED_SOURCES 	= src/generated/
AST_GENERATED_HEADERS	= include/generated/
# TODO: Slashes
SOURCES   	  			= src/core/ src/traversal/ src/passes/
TARGET 	 	  			= civcc
# TODO HIDE


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

ECHO = $(shell which echo)

.PHONY: all compile_generated clean test format doc

all: $(TARGET_BIN) ;

$(TARGET_BIN): $(SRC:.c=.o) $(LIB_SRC:.c=.o) compile_generated
	@$(ECHO) -e "$(COLOR_GREEN) LINK$(COLOR_RESET)      $@"
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $(SRC:.c=.o) $(LIB_SRC:.c=.o) $(wildcard $(AST_GENERATED_SOURCES)*.o)

%.o: %.c compile_generated
	@$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	@$(CC) $(CFLAGS) -I include/ -o $@ -c $<

compile_generated: $(AST_GENERATED_SRC_GENFILE)
	@make -C $(AST_GENERATED_SOURCES) CC="$(CC)" CFLAGS="$(CFLAGS)" \
		COLOR_GREEN="$(COLOR_GREEN)" COLOR_RESET="$(COLOR_RESET)"


# ----------------------- AST-gen rules --------------------

$(AST_GENERATED_SRC_GENFILE): $(AST_GENERATED_INC_GENFILE) ;

$(AST_GENERATED_INC_GENFILE): $(AST_TARGET_BIN) $(AST_FILE)
	@$(ECHO) -e "$(COLOR_GREEN) ASTGEN$(COLOR_RESET)    $(AST_FILE)"
	@$(AST_TARGET_BIN) --source-dir $(AST_GENERATED_SOURCES) \
		--header-dir $(AST_GENERATED_HEADERS) $(AST_FILE)
	@touch $(AST_GENERATED_INC_GENFILE) $(AST_GENERATED_SRC_GENFILE)

$(AST_TARGET_BIN): $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC:.c=.o)
	@$(ECHO) -e "$(COLOR_GREEN) LINK$(COLOR_RESET)      $@"
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC:.c=.o)

$(LIB_SOURCES)%.o: $(LIB_SOURCES)%.c
	@$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	@$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_GEN_SOURCES)%.o: $(AST_GEN_SOURCES)%.c
	@$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	@$(CC) $(CFLAGS) -I include/ -o $@ -c $<

%.lexer.h: %.lexer.c
	@$(ECHO) -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $@"

%.lexer.c: %.l
	@$(ECHO) -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $@"
	@flex -o $@ --header-file=$(AST_LEXER:.c=.h) $<

%.parser.c: %.y %.lexer.h
	@$(ECHO) -e "$(COLOR_GREEN) BISON$(COLOR_RESET)     $@"
	@bison -dv -o $@ $<


# ----------------------- Other rules --------------------
clean:
	@$(ECHO) -e "$(COLOR_GREEN) CLEANING$(COLOR_RESET)"
	@rm -f $(AST_TARGET_BIN) $(TARGET_BIN)
	@rm -f $(AST_LEXER) $(AST_LEXER:.c=.h) \
		$(AST_PARSER) $(AST_PARSER:.c=.output) \
		$(AST_PARSER:.c=.h) \
		$(AST_GENERATED_SOURCES)*.c \
		$(AST_GENERATED_HEADERS)*.h
	@find . -type f -name '*.o' -exec rm {} \;
	@find . -type f -name '*.d' -exec rm {} \;
	@$(ECHO) -e "$(COLOR_GREEN) DONE$(COLOR_RESET)"

doc:
	$(MAKE) --directory doc html

png: $(AST_TARGET_BIN)
	@$(ECHO) -e "$(COLOR_GREEN) GEN$(COLOR_RESET)"
	@$(AST_TARGET_BIN) --dot $(DOC_DIR) $(AST_FILE)
	@$(ECHO) -e "$(COLOR_GREEN) DOT$(COLOR_RESET)"
	@dot -Tpng $(DOC_DIR)ast.dot > $(DOC_DIR)ast.png

test: $(AST_TARGET_BIN)
	@test/test.sh test

format:
	@$(ECHO) "Applying clang-format on all files"
	@find . -name "*.h" -o -name "*.c" | xargs -n1 clang-format -i

-include $(AST_SRC:.c=.d)
-include $(SRC:.c=.d)
