LIBS := -lrt -lpthread

all: print_server client

print_server: print_server.c
	gcc $^ -o $@ $(LIBS)

client: client.c
	gcc $^ -g -o $@ $(LIBS)

clean:
	-rm print_server client
