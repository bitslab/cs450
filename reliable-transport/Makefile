BINS=sender receiver
all: $(BINS)

sender: sender.c
	gcc -g $? -o $@

receiver: receiver.c
	gcc -g $? -o $@

clean:
	rm -rf *~ *.dSYM $(BINS)
