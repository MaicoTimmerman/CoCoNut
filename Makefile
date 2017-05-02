# -------------------- Compiler flags ---------------------
CC           := gcc
CFLAGS       := -Wall -std=gnu11 -g -Og -pedantic -MMD \
				-Werror=implicit-function-declaration


# ----------------------- AST-gen variables ---------------
AST_SOURCES   = src/lib/ src/astgen/
AST_SRC       = $(foreach dir,$(AST_SOURCES),$(wildcard $(dir)*.c))
AST_PARSER    = src/astgen/ast.parser.c
AST_LEXER     = src/astgen/ast.lexer.c
AST_TARGET    = astgen

# ----------------------- Compiler variables --------------------
SOURCES   	  = src/lib/ src/core/
SRC           = $(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)*.c))
TARGET 	 	  = civcc

# ----------------------- Other variables --------------------
COLOR_GREEN	  = "\\e[1m\\e[32m"
COLOR_RESET	  = "\\e[0m"

.PHONY: clean test format doc

# ----------------------- AST-gen rules --------------------
$(AST_TARGET): $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC:.c=.o)
	@echo -e "$(COLOR_GREEN) LINK$(COLOR_RESET)      $@"
	@mkdir -p bin/
	@$(CC) -o bin/$@ $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC:.c=.o)

%.o: %.c
	@echo -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $(notdir $@)"
	@$(CC) $(CFLAGS) -I include/ -o $@ -c $<

%.lexer.h: %.lexer.c
	@echo -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $(notdir $@)"

%.lexer.c: %.l
	@echo -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $(notdir $@)"
	@flex -o $@ --header-file=$(AST_LEXER:.c=.h) $<

%.parser.c: %.y %.lexer.h
	@echo -e "$(COLOR_GREEN) BISON$(COLOR_RESET)     $(notdir $@)"
	@bison -dv -o $@ $<


# ----------------------- Other rules --------------------
clean:
	@rm -f bin/$(AST_TARGET)  $(AST_SRC:.c=.o) $(AST_SRC:.c=.d) \
		$(AST_LEXER) $(AST_LEXER:.c=.o) $(AST_LEXER:.c=.d) \
		$(AST_LEXER:.c=.h) $(AST_PARSER) $(AST_PARSER:.c=.output) \
		$(AST_PARSER:.c=.o) $(AST_PARSER:.c=.h) $(AST_PARSER:.c=.d)

doc:
	$(MAKE) --directory doc html

test: $(AST_TARGET)
	@test/test.sh test

format:
	@echo "Applying clang-format on all files"
	@find . -name "*.h" -o -name "*.c" | xargs -n1 clang-format -i

-include $(AST_SRC:.c=.d)
