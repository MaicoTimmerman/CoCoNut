# -------------------- Configurable options  ---------------------
include Makefile.config
# ----------------------- Other variables --------------------

LIB_SOURCES   			= src/lib
LIB_SRC       = $(foreach dir,$(LIB_SOURCES),$(wildcard $(dir)/*.c))

AST_GENERATED_SRC_GENFILE 		= $(AST_GENERATED_SOURCES).gen
AST_GENERATED_INC_GENFILE 		= $(AST_GENERATED_HEADERS).gen


AST_GEN_SOURCES = src/cocogen
AST_SOURCES   = $(LIB_SOURCES) $(AST_GEN_SOURCES)
AST_SRC       = $(foreach dir,$(AST_SOURCES),$(wildcard $(dir)/*.c))
AST_PARSER    = src/cocogen/ast.parser.c
AST_LEXER     = src/cocogen/ast.lexer.c
AST_TARGET    = cocogen

AST_SRC_FILTERED = $(filter-out $(AST_LEXER) $(AST_PARSER),$(AST_SRC))

AST_TXT_PARSER 	= src/framework/serialization-txt.parser.c
AST_TXT_LEXER 	= src/framework/serialization-txt.lexer.c

SOURCES       += src/framework
SRC           = $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
SRC_FILTERED  = $(filter-out $(AST_TXT_LEXER) $(AST_TXT_PARSER),$(SRC))

COLOR_GREEN	  = "\\e[1m\\e[32m"
COLOR_RESET	  = "\\e[0m"

TARGET_BIN 		= $(BIN_DIR)$(TARGET)
AST_TARGET_BIN  = $(BIN_DIR)$(AST_TARGET)

# ----------------------- Compiler rules --------------------

ECHO = $(shell which echo)

.PHONY: all compile_generated clean test format doc

all: $(TARGET_BIN) ;

$(TARGET_BIN): $(SRC_FILTERED:.c=.o) $(LIB_SRC:.c=.o) $(AST_TXT_LEXER:.c=.o) $(AST_TXT_PARSER:.c=.o) compile_generated
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) LINK$(COLOR_RESET)      $@"
	$(DEBUG)mkdir -p $(BIN_DIR)
	$(DEBUG)$(CC) -o $@ $(LDFLAGS) $(SRC_FILTERED:.c=.o) $(LIB_SRC:.c=.o) \
		$(AST_TXT_PARSER:.c=.o) $(AST_TXT_LEXER:.c=.o) \
		$(wildcard $(AST_GENERATED_SOURCES)*.o)

%.o: %.c compile_generated
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	$(DEBUG)$(CC) $(CFLAGS) -I include/ -o $@ -c $<

compile_generated: $(AST_GENERATED_SRC_GENFILE)
	$(DEBUG)make -C $(AST_GENERATED_SOURCES) -j4 CC="$(CC)" CFLAGS="$(CFLAGS)" \
		COLOR_GREEN="$(COLOR_GREEN)" COLOR_RESET="$(COLOR_RESET)"

$(AST_TXT_LEXER:.c=.o): $(AST_TXT_LEXER) $(AST_TXT_PARSER:.c=.h)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	$(DEBUG)$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_TXT_PARSER:.c=.o): $(AST_TXT_PARSER) $(AST_TXT_LEXER:.c=.h)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	$(DEBUG)$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_TXT_PARSER:.c=.h): $(AST_TXT_PARSER)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) BISON$(COLOR_RESET)     $@"

$(AST_TXT_LEXER:.c=.h): $(AST_TXT_LEXER)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $@"


# ----------------------- AST-gen rules --------------------

$(AST_GENERATED_SRC_GENFILE): $(AST_GENERATED_INC_GENFILE) ;

$(AST_GENERATED_INC_GENFILE): $(AST_TARGET_BIN) $(AST_FILE)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) COCOGEN$(COLOR_RESET)    $(AST_FILE)"
	$(DEBUG)$(AST_TARGET_BIN) --source-dir $(AST_GENERATED_SOURCES) \
		--header-dir $(AST_GENERATED_HEADERS) $(AST_FILE)
	$(DEBUG)touch $(AST_GENERATED_INC_GENFILE) $(AST_GENERATED_SRC_GENFILE)

$(AST_TARGET_BIN): $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC_FILTERED:.c=.o)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) LINK$(COLOR_RESET)      $@"
	$(DEBUG)mkdir -p $(BIN_DIR)
	$(DEBUG)$(CC) -o $@ $(LDFLAGS) $(AST_PARSER:.c=.o) $(AST_LEXER:.c=.o) $(AST_SRC_FILTERED:.c=.o)

$(LIB_SOURCES)/%.o: $(LIB_SOURCES)/%.c
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	$(DEBUG)$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_GEN_SOURCES)/%.o: $(AST_GEN_SOURCES)/%.c
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	$(DEBUG)$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_LEXER:.c=.o): $(AST_LEXER) $(AST_PARSER:.c=.h)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	$(DEBUG)$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_PARSER:.c=.o): $(AST_PARSER) $(AST_LEXER:.c=.h)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	$(DEBUG)$(CC) $(CFLAGS) -I include/ -o $@ -c $<

$(AST_PARSER:.c=.h): $(AST_PARSER)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) BISON$(COLOR_RESET)     $@"

$(AST_LEXER:.c=.h): $(AST_LEXER)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $@"

%.lexer.c: %.l
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) FLEX$(COLOR_RESET)      $@"
	$(DEBUG)flex -o $@ --header-file=$(@:.c=.h) $<

%.parser.c: %.y
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) BISON$(COLOR_RESET)     $@ $%"
	$(DEBUG)bison -dv -o $@ $<


# ----------------------- Other rules --------------------
clean:
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) CLEANING$(COLOR_RESET)"
	$(DEBUG)rm -f $(AST_TARGET_BIN) $(TARGET_BIN)
	$(DEBUG)rm -f $(AST_LEXER) $(AST_LEXER:.c=.h) \
		$(AST_PARSER) $(AST_PARSER:.c=.output) \
		$(AST_PARSER:.c=.h) \
		$(AST_TXT_LEXER) $(AST_TXT_LEXER:.c=.h) \
		$(AST_TXT_PARSER) $(AST_TXT_PARSER:.c=.output) \
		$(AST_TXT_PARSER:.c=.h) \
		$(AST_GENERATED_SOURCES)*.c \
		$(AST_GENERATED_HEADERS)*.h
	$(DEBUG)find . -type f -name '*.o' -exec rm {} \;
	$(DEBUG)find . -type f -name '*.d' -exec rm {} \;
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) DONE$(COLOR_RESET)"

doc:
	$(MAKE) --directory doc html

png: $(AST_TARGET_BIN)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) GEN$(COLOR_RESET)"
	$(DEBUG)$(AST_TARGET_BIN) --dot $(DOC_DIR) $(AST_FILE)
	$(DEBUG)$(ECHO) -e "$(COLOR_GREEN) DOT$(COLOR_RESET)"
	$(DEBUG)dot -Tpng $(DOC_DIR)ast.dot > $(DOC_DIR)ast.png

test: $(AST_TARGET_BIN)
	$(DEBUG)test/test.sh test

format:
	$(DEBUG)$(ECHO) "Applying clang-format on all files"
	$(DEBUG)find . -name "*.h" -o -name "*.c" | egrep  -v "^(./src/generated/|./include/generated)" | xargs -n1 clang-format -i

-include $(AST_SRC:.c=.d)
-include $(SRC:.c=.d)
