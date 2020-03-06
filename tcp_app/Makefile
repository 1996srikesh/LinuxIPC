CC=gcc
CSOURCES1=tcp_server.c
CSOURCES2=tcp_client.c
CFLAGS=-c -pthread -Wall -I../
TARGET1=tcp_server
TARGET2=tcp_client

.PHONY : all clean release debug

BUILD_DIR=release

debug: BUILD_DIR=debug
release: BUILD_DIR=release

all: $(TARGET1) $(TARGET2)

clean:
	-rm -rf debug
	-rm -rf release
	
release: $(TARGET1) $(TARGET2)

debug: CFLAGS += -std=c99 -DDEBUG_ON -g -O0
debug: $(TARGET1) $(TARGET2)

A_OBJS=$(CSOURCES1:.c=.o)
B_OBJS=$(CSOURCES2:.c=.o)

$(TARGET1): $(A_OBJS)
	cd $(BUILD_DIR); $(CC) -pthread $^ -o $@ 

$(TARGET2): $(B_OBJS)
	cd $(BUILD_DIR); $(CC) -pthread $^ -o $@ 

%.o : %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@
