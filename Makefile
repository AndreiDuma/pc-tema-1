all: sender receiver

sender: sender.o lib.o link_emulator/lib.o
	gcc -g sender.o lib.o link_emulator/lib.o -o sender

receiver: receiver.o lib.o link_emulator/lib.o
	gcc -g receiver.o lib.o link_emulator/lib.o -o receiver

.c.o: 
	gcc -Wall -g -c $? 

clean:
	rm -f *.o sender receiver
