CC           := gcc
CFLAGS       := -Wall -std=gnu11 -g -Og -pedantic -MMD
SOURCE_DIRS   = src/lib/ src/astlang/ src/
SRC           = $(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)*.c))
PARSER        = src/astlang/ast.parser.c
LEXER         = src/astlang/ast.lexer.c

TARGET = ast

.PHONY: clean test format

$(TARGET): $(PARSER:.c=.o) $(LEXER:.c=.o) $(SRC:.c=.o)
	@echo "Linking executable: $@"
	@mkdir -p bin/
	@$(CC) -o bin/$@ $(PARSER:.c=.o) $(LEXER:.c=.o) $(SRC:.c=.o)

clean:
	@rm -f bin/$(TARGET) $(SRC:.c=.o) $(SRC:.c=.d) $(LEXER) $(LEXER:.c=.o) $(LEXER:.c=.d) \
		$(LEXER:.c=.h) $(PARSER) $(PARSER:.c=.output) $(PARSER:.c=.o) \
		$(PARSER:.c=.h) $(PARSER:.c=.d)

enum: $(TARGET)
	bin/$(TARGET) < test/pass/meta_ast.ast
	autogen -T src/templates/enum.tpl src/templates/enum.def

%.o: %.c
	@echo "Compiling source code: $(notdir $@)"
	@$(CC) $(CFLAGS) $(patsubst %,-I%,$(SOURCE_DIRS)) -o $@ -c $<

%.lexer.h: %.lexer.c
	@echo "Generating header from LEX specification $(notdir $@)"

%.lexer.c: $(LEXER:.lexer.c=.l)
	@echo "Generating source code from LEX specification: $(notdir $@)"
	@flex -o $@ --header-file=$(LEXER:.c=.h) $<

%.parser.c: $(PARSER:.parser.c=.y) $(LEXER:.c=.h)
	@echo "Generating source code from YACC specification: $(notdir $@)"
	@bison -dv -o $@ $<

test: $(TARGET)
	@test/test.sh test

format:
	@echo "Applying clang-format on all files"
	@find . -name "*.h" -o -name "*.c" | xargs -n1 clang-format -i

-include $(SRC:.c=.d)
