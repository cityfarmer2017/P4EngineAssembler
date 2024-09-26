
TEMP_DIRS := $(BIN_DIR) $(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR)

SRCFILES := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES := $(INC_DIR)
OBJECTS  := $(SRCFILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPENDS  := $(OBJECTS:$(OBJ_DIR)/%=$(DEP_DIR)/%.d)
export OBJECTS

SUB1_MODULE := table_proc
SUB_MODULES := $(SUB1_MODULE)
# SUB2_MODULE := *_proc
# SUB_MODULES += $(SUB2_MODULE)

.PHONY : all sub_modules
all : $(TEMP_DIRS) sub_modules $(BIN_DIR)/$(APP_NAME)

$(TEMP_DIRS) :
	mkdir -p $@

ifeq (,$(SUB_FLAG))

sub_modules:
	@for m in $(SUB_MODULES); do cd $(SRC_DIR)/$$m; make SUB_MODULE=$$m || exit "$$?"; done

SUB1_SRC_FILES := $(wildcard $(SRC_DIR)/$(SUB1_MODULE)/*.cpp)
SUB_OBJECTS := $(SUB1_SRC_FILES:$(SRC_DIR)/$(SUB1_MODULE)/%.cpp=$(OBJ_DIR)/$(SUB1_MODULE)/%.o)
# SUB2_SRC_FILES := $(wildcard $(SRC_DIR)/$(SUB2_MODULE)/*.cpp)
# SUB_OBJECTS += $(SUB2_SRC_FILES:$(SRC_DIR)/$(SUB2_MODULE)/%.cpp=$(OBJ_DIR)/$(SUB2_MODULE)/%.o)

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS) $(SUB_OBJECTS)
	@echo "\e[32m""Linking executable: $(BIN_DIR)/$(APP_NAME) with sub modules included.""\e[0m"
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS) $(SUB_OBJECTS)

else  # ifeq (,$(SUB_FLAG))

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS)
	@echo "\e[32m""Linking executable: $(BIN_DIR)/$(APP_NAME) without sub modules included.""\e[0m"
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS)

endif  # ifeq (,$(SUB_FLAG))

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CCFLAGS) -I$(INCLUDES) -o $@ -c $< -MMD -MF $(patsubst $(OBJ_DIR)/%, $(DEP_DIR)/%.d, $@)

-include $(DEPENDS)

.PHONY : clean
clean :
	@for m in $(SUB_MODULES); do cd $(SRC_DIR)/$$m; make clean SUB_MODULE=$$m || exit "$$?"; done
	rm -rf $(BIN_DIR)/$(APP_NAME) $(OBJECTS) $(DEPENDS)