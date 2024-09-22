SUB_OBJ_DIR := $(OBJ_DIR)/$(SUB_MODULE)
SUB_DEP_DIR := $(DEP_DIR)/$(SUB_MODULE)

SUB_TEMP_DIRS := $(SUB_OBJ_DIR) $(SUB_DEP_DIR)

SUB_SRC_DIR := $(SRC_DIR)/$(SUB_MODULE)
SUB_INC_DIR := $(INC_DIR)/$(SUB_MODULE)

SUB_SRCFILES := $(wildcard $(SUB_SRC_DIR)/*.cpp)
SUB_INCLUDES := $(SUB_INC_DIR)
SUB_OBJECTS  := $(SUB_SRCFILES:$(SUB_SRC_DIR)/%.cpp=$(SUB_OBJ_DIR)/%.o)
SUB_DEPENDS  := $(SUB_OBJECTS:$(SUB_OBJ_DIR)/%=$(SUB_DEP_DIR)/%.d)

.PHONY : all
all : $(SUB_TEMP_DIRS) $(SUB_OBJECTS)

$(SUB_TEMP_DIRS) :
	mkdir -p $@

$(SUB_OBJ_DIR)/%.o : $(SUB_SRC_DIR)/%.cpp
	$(CC) $(CCFLAGS) -I$(SUB_INCLUDES) -o $@ -c $< -MMD -MF $(patsubst $(SUB_OBJ_DIR)/%, $(SUB_DEP_DIR)/%.d, $@)

-include $(SUB_DEPENDS)

.PHONY : clean
clean :
	rm -rf $(SUB_OBJECTS) $(SUB_DEPENDS)