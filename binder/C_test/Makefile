APPS = service_manager test_client test_server
all : $(APPS)

service_manager : service_manager.o binder.o
	arm-linux-gcc -o $@ $^ -lpthread
test_client : test_client.o binder.o
	arm-linux-gcc -o $@ $^ -lpthread
test_server : test_server.o binder.o
	arm-linux-gcc -o $@ $^ -lpthread

%.o : %.c
	arm-linux-gcc -DBINDER_IPC_32BIT=1 -I include -c -o $@ $<

clean:
	rm $(APPS) -f; rm -f *.o
