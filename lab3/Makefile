all: shell

shell: mini_shell.c error.c init_destroy.c parse.c commands.c
	@gcc -o shell mini_shell.c error.c init_destroy.c parse.c commands.c -I -d

clean:
	-rm shell