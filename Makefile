all: client server

client:
	gcc ./src/forum.c ./src/util.c -o client

server:
	gcc ./src/server.c ./src/fdb.c ./src/util.c ./lib/sqlite3.c -o server

clean:
	rm -rf client server
