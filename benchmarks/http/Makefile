BUILD_DIR = build
CL_FLAGS = -Wall 
all: client server

server: build_dir bhserver.c
	gcc $(CL_FLAGS) bhserver.c -o $(BUILD_DIR)/bhserver

client: build_dir bhclient.c	
	gcc $(CL_FLAGS) bhclient.c -o $(BUILD_DIR)/bhclient

build_dir:
	if ! [ -d "build" ]; then mkdir -p $(BUILD_DIR); fi 

clean: 
	rm -rf $(BUILD_DIR)/*

