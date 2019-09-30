TARGET_CLIENT = client
TARGET_SERVER = server

CC = g++
SRC_CLIENT = client.cpp
SRC_SERVER = server.cpp
OBJ_CLIENT = $(patsubst %.cpp,%.o,$(SRC_CLIENT))
OBJ_SERVER = $(patsubst %.cpp,%.o,$(SRC_SERVER))

CPPFLAGS += -g -Wall -std=c++11 -I./
LDFLAGS += -lpthread

all: $(OBJ_CLIENT) $(OBJ_SERVER)
	$(CC) $(LDFLAGS) -o $(TARGET_CLIENT) $(OBJ_CLIENT)
	$(CC) $(LDFLAGS) -o $(TARGET_SERVER) $(OBJ_SERVER)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $<

.PHONY: all clean

clean:
	rm -vf $(TARGET_CLIENT) $(TARGET_SERVER) $(OBJ_CLIENT) $(OBJ_SERVER)
