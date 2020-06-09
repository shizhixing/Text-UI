#CC=arm-unknown-linux-gnueabi-gcc
CC=gcc
CFLAGS=-Wall -pthread -lutil -g -std=c99

TARGET = minicoding
ROOT_PATH = $(CUR_PATH)
SRC_PATH = $(ROOT_PATH)src/
INC = -I./
OBJ_PATH = $(ROOT_PATH)obj/

SRCS = $(SRC_PATH)main.c\
	$(SRC_PATH)input.c\
	$(SRC_PATH)window.c\
	$(SRC_PATH)label.c\
	$(SRC_PATH)text.c\
	$(SRC_PATH)memo.c\
	$(SRC_PATH)user.c\
	$(SRC_PATH)cursor.c\
    $(SRC_PATH)memory.c\
    $(SRC_PATH)button.c\
    $(SRC_PATH)scroll.c\
    $(SRC_PATH)listview.c\

INCS = $(SRC_PATH)window.h\
	$(SRC_PATH)cursor.h\
	$(SRC_PATH)input.h\
	$(SRC_PATH)def.h\
	$(SRC_PATH)window_type.h\

OBJS = $(patsubst $(SRC_PATH)%,$(OBJ_PATH)%,$(SRCS:.c=.o))

$(TARGET):$(OBJS)
	@echo "-------------------Linking--------------";\
	$(CC) -o $@ $^ $(CFLAGS);\
	if [ -f $(TARGET) ]; then echo "Build $(TARGET) Successfully"; fi;

$(OBJ_PATH)%.o:$(SRC_PATH)%.c $(INCS) 
	@echo "-------------------Building--------------";\
	if [ ! -d $(OBJ_PATH) ]; then mkdir -p $(OBJ_PATH); fi;\
	if [ -f $(TARGET) ]; then rm $(TARGET); fi;\
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<;
	
	
clean:
	rm -rf $(TARGET) $(OBJ_PATH)*
	
all:
	@rm -rf $(TARGET) $(OBJ_PATH)*;\
	make
