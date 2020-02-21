CC     = gcc
INC    = -I.
CFLAGS = $(INC) -Wall -Wextra -Werror
CFILES = $(wildcard *.c)
OBJS   = my_shell.o
BIN    = my_shell

%.o:%.c
	$(info Compiling $<)
	@$(CC) $(CFLAGS) -o $@ -c $<

$(BIN):$(OBJS)
	$(CC) -o $@ $^

exec:$(BIN)
	./my_shell

clean:
	@rm -f $(BIN) $(OBJS) *~