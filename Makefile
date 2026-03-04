CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include
TARGET = ping
SRCDIR = src
SOURCES = $(SRCDIR)/ping.c
HEADERS = include/ping.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) *.o

install:
	sudo chown root:root $(TARGET)
	sudo chmod u+s $(TARGET)

test: $(TARGET)
	@echo "🧪 Testing ping command..."
	@sudo ./$(TARGET) -c 3 127.0.0.1

.PHONY: all clean install test
