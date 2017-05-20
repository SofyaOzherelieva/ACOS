all: server client

server: server.c format_mes.c receive_mes.c 
	@gcc -o server server.c format_mes.c receive_mes.c -I -d -pthread

client: client.c
	@gcc -o client client.c -I -d -pthread

clean:
	-rm server client