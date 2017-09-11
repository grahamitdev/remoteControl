CC:=gcc
CFLAGS:=-Iinclude 
CFLAGS+=-c
TARGET:=bin/main
DEPEND:=obj/main.o
DEPEND+=obj/serverview.o
DEPEND+=obj/servercontrol.o
DEPEND+=obj/serverlink.o
DEPEND+=obj/server.o

$(TARGET):$(DEPEND)
	$(CC) -o $@ $^ -lpthread 

obj/%.o:src/%.c
	$(CC) -o $@ $(CFLAGS) $^ -Wall

do:
	$(TARGET)

clean:
	rm -fr $(DEPEND)
	clear
