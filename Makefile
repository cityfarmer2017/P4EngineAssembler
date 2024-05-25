APP_NAME := P4eAsm
SRC_DIR := src
INC_DIR := inc
OBJ_DIR := obj
BIN_DIR := bin
DEP_DIR := dep

CC := g++
CCFLAGS := -g -Wall

SRCFILES := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES := $(INC_DIR)
OBJECTS  := $(SRCFILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPENDS  := $(OBJECTS:$(OBJ_DIR)/%=$(DEP_DIR)/%.d)

all : $(BIN_DIR)/$(APP_NAME)

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS)
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CCFLAGS) -I$(INCLUDES) -o $@ -c $< -MMD -MF $(patsubst $(OBJ_DIR)/%, $(DEP_DIR)/%.d, $@)

-include $(DEPENDS)

.PHONY : clean
clean :
	rm -f $(BIN_DIR)/$(APP_NAME) $(OBJECTS) $(DEPENDS)
# objects = main.o p4e_assembler.o parser_operation.o

# p4easm : $(objects)
# 	g++ -o p4easm $(objects)

# main.o : p4e_assembler.h
# p4e_assembler.o : p4e_assembler.h
# parser_operation.o : global_def.h parser_def.h parser_operation.h

# .PHONY : clean
# clean :
# 	rm p4easm $(objects)