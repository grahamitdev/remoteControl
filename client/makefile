CC:=gcc
CFLAGS:=-Iinclude 
CFLAGS+=-c
TARGET:=bin/main
DEPEND:=obj/main.o
DEPEND+=obj/clientview.o
DEPEND+=obj/clientcontrol.o
DEPEND+=obj/clientlink.o
DEPEND+=obj/client.o

$(TARGET):$(DEPEND)
	$(CC) -o $@ $^ -lpthread 

obj/%.o:src/%.c
	$(CC) -o $@ $(CFLAGS) $^ -Wall

do:
	$(TARGET)

clean:
	rm -fr $(DEPEND)
	clear
