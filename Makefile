CC = cc
CFLAGS = -Wall -Wextra
OBJ = cmd/server.o utils/channel.o
TARGET = server
LDLIBS = -lpthread

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

