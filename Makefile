SERVER = agent
LDFLAGS = -g -DLINUX -lpthread
SERVER_SRC = $(wildcard *.c)
SERVER_OBJ = $(patsubst %.c,%.o,$(SERVER_SRC))

$(SERVER): $(SERVER_OBJ) 
	gcc -o $@ $^ $(LDFLAGS)

$(SERVER_OBJ): $(SERVER_SRC)
	@echo $(SERVER_SRC)
	gcc -c $^  $(LDFLAGS)

run:
	./$(SERVER) -h 192.168.1.246 -p 8080 -b 100

clean:
	rm -f *.o
	rm -f $(SERVER)
clean1:
	rm -f *.o