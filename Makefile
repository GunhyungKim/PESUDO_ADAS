XIMGPROC=1

LDLIBS=`pkg-config opencv --cflags --libs` -lboost_program_options -lboost_filesystem -lboost_system -lboost_thread -lv4l2 -ludev -pthread -lpthread -lcblas -L../darknet/ -ldarknet
CFLAG=g++-5 -g -std=c++11 -Wfatal-errors -Wno-write-strings
OBJFLAG=-c
SRC_DIR:=./src
DEPS=Makefile
ifeq ($(XIMGPROC),1)
	CFLAG += -DXIMGPROC
endif
INCLUDE= -Isrc/Location/

OBJ_DIR:=./obj
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp) 
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))



SRC_DIR2 =./src/Location
SRC2 := $(wildcard $(SRC_DIR2)/*.cpp)
SRC_FILES += $(SRC2)
OBJ_FILES += $(patsubst $(SRC_DIR2)/%.cpp,$(OBJ_DIR)/%.o,$(SRC2))


TARGET:=main

$(TARGET): $(OBJ_DIR) $(OBJ_FILES) $(DEPS)
	$(CFLAG) -o $@ $(OBJ_FILES)  $(LDLIBS) 
#$(CFLAG) -o $@ $^ $(LDLIBS)	

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CFLAG) -c -o $@ $< $(INCLUDE)

$(OBJ_DIR)/%.o: $(SRC_DIR2)/%.cpp $(DEPS)
	$(CFLAG) -c -o $@ $< $(INCLUDE)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	

clean:
	rm -f main
	rm -f $(OBJ_DIR)/*



test:
	$(CFLAG) -o src/yolo src/yolo.cpp $(LDLIBS)
	$(CFLAG) -o web_socket_server src/Location/web_socket_server.cpp $(LDLIBS)
	
	
