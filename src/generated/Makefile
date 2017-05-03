SRC  = $(wildcard *.c)

all: $(SRC:.c=.o) ;

%.o: %.c
	@echo -e "$(COLOR_GREEN) CC$(COLOR_RESET)        $@"
	@$(CC) $(CFLAGS) -I ../../include/ -o $@ -c $<
