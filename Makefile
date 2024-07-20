APP_NAME := P4eAsm
SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
BIN_DIR := bin
DEP_DIR := dep
TEMP_DIRS := $(OBJ_DIR) $(BIN_DIR) $(DEP_DIR)

CC := g++
CCFLAGS := -g -Wall# -DDEBUG

SRCFILES := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES := $(INC_DIR)
OBJECTS  := $(SRCFILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPENDS  := $(OBJECTS:$(OBJ_DIR)/%=$(DEP_DIR)/%.d)

all : $(TEMP_DIRS) $(BIN_DIR)/$(APP_NAME)

$(TEMP_DIRS) :
	mkdir -p $@

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS)
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CCFLAGS) -I$(INCLUDES) -o $@ -c $< -MMD -MF $(patsubst $(OBJ_DIR)/%, $(DEP_DIR)/%.d, $@)

-include $(DEPENDS)

.PHONY : clean
clean :
	rm -rf $(BIN_DIR)/$(APP_NAME) $(OBJECTS) $(DEPENDS)

.PHONY : dist-clean
dist-clean :
	rm -rf $(TEMP_DIRS)