CC=g++ --std=c++14
	SRC_FILE=src
	OBJ_FILE=obj
	BIN_FILE=bin
	INC_FILE=h
	INCLUDES=-I $(INC_FILE)
CFLAGS=
LDFLAGS=
	SOURCES=$(wildcard $(SRC_FILE)/*.cpp)
	OBJECTS=$(patsubst $(SRC_FILE)/%.cpp, $(OBJ_FILE)/%.o, $(SOURCES))
EXECUTABLE=$(BIN_FILE)/as
all: make_dirs $(SOURCES) $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -g3 -gdwarf-2 -o $@
$(OBJ_FILE)/%.o: $(SRC_FILE)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -g3 -gdwarf-2 -c $< -o $@
.PHONY: clean clean_obj clean_exe clean_~ make_dirs
make_dirs:
	mkdir -p $(BIN_FILE)
	mkdir -p $(OBJ_FILE)
clean: clean_obj clean_exe clean_~
clean_obj:
	rm -rf $(OBJ_FILE)/*.o
clean_exe:
	rm -rf $(EXECUTABLE)
clean_~:
	rm -rf *~
	rm -rf $(SRC_FILE)/*~
	rm -rf $(INC_FILE)/*~