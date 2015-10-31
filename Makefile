BIN=m2gServer
LIB=BufferQueue/BufferQueue.a ./m2gnetwork/network/network.a ./m2gnetwork/rdr/rdr.a ./m2gnetwork/rfb/rfb.a ./h264enc/h264enc.a

CC=/usr/bin/g++
FLAGS= -g -o3 -pthread -IBufferQueue/inc


all: $(BIN)
	@echo
	@echo "$(BIN) created successful"
	@echo "use \"make check\" to execute this example program"
	@echo

check: $(BIN)
	./$(BIN)
	@diff out.264 test.264
	@echo "check output OK"

$(BIN): *.cpp ${LIB}
	@echo "create $(BIN)"
	$(CC) $(FLAGS) *.cpp $(LIB) -o $(BIN)

clean:
	cd BufferQueue; make clean
	@rm -f $(BIN) *.o out*.264 $(LIB)
	@rm -f m2gServer
	@echo "clean m2gServer complete"

BufferQueue/BufferQueue.a:
	cd BufferQueue;make;
	ranlib $(LIB)
