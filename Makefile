CC           := gcc
CFLAGS       := -Wall -std=gnu11 -g -Og -pedantic -MMD
SOURCE_DIRS   = src/lib/ src/astlang/ src/
SRC           = $(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)*.c))
PARSER        = src/astlang/ast.parser.c
LEXER         = src/astlang/ast.lexer.c

TARGET = ast

.PHONY: clean test

$(TARGET): $(PARSER:.c=.o) $(LEXER:.c=.o) $(SRC:.c=.o)
	@echo "Linking executable: $@"
	@mkdir -p bin/
	@$(CC) -o bin/$@ $(PARSER:.c=.o) $(LEXER:.c=.o) $(SRC:.c=.o)

clean:
	@rm -f bin/$(TARGET) $(SRC:.c=.o) $(SRC:.c=.d) $(LEXER) $(LEXER:.c=.o) $(LEXER:.c=.d) \
		$(PARSER) $(PARSER:.c=.output) $(PARSER:.c=.o) $(PARSER:.c=.h) $(PARSER:.c=.d)

%.o: %.c
	@echo "Compiling source code: $(notdir $@)"
	@$(CC) $(CFLAGS) $(patsubst %,-I%,$(SOURCE_DIRS)) -o $@ -c $<

%.lexer.c: $(LEXER:.lexer.c=.l)
	@echo "Generating source code from LEX specification: $(notdir $@)"
	@flex -o $@ $<

%.parser.c: $(PARSER:.parser.c=.y)
	@echo "Generating source code from YACC specification: $(notdir $@)"
	@bison -dv -o $@ $<

test: $(TARGET)
	@test/test.sh

-include $(SRC:.c=.d)
