
TEMP_DIRS := $(BIN_DIR) $(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR)

NO_TBL_PROC := -DNO_TBL_PROC
NO_PRE_PROC := -DNO_PRE_PROC

SRCFILES := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES := $(INC_DIR)
OBJECTS  := $(SRCFILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPENDS  := $(OBJECTS:$(OBJ_DIR)/%=$(DEP_DIR)/%.d)
export OBJECTS

ifneq ($(NO_TBL_PROC),$(TBL_FLAG))
SUB1_MODULE := table_proc
SUB_MODULES := $(SUB1_MODULE)
endif
ifneq ($(NO_PRE_PROC),$(PRE_FLAG))
SUB2_MODULE := pre_proc
SUB_MODULES += $(SUB2_MODULE)
endif

.PHONY : all sub_modules
all : $(TEMP_DIRS) sub_modules $(BIN_DIR)/$(APP_NAME)

$(TEMP_DIRS) :
	mkdir -p $@

ifneq (,$(and $(findstring $(NO_TBL_PROC),$(TBL_FLAG)),$(findstring $(NO_PRE_PROC),$(PRE_FLAG))))

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS)
	@echo "\e[32m""Linking executable: $(BIN_DIR)/$(APP_NAME) without sub module included.""\e[0m"
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS)

else # ifneq (,$(and $(findstring $(NO_TBL_PROC),$(TBL_FLAG)),$(findstring $(NO_PRE_PROC),$(PRE_FLAG))))

sub_modules:
	@for m in $(SUB_MODULES); do cd $(SRC_DIR)/$$m; make SUB_MODULE=$$m || exit "$$?"; done

ifneq ($(NO_TBL_PROC),$(TBL_FLAG))
SUB1_SRC_FILES := $(wildcard $(SRC_DIR)/$(SUB1_MODULE)/*.cpp)
SUB_OBJECTS := $(SUB1_SRC_FILES:$(SRC_DIR)/$(SUB1_MODULE)/%.cpp=$(OBJ_DIR)/$(SUB1_MODULE)/%.o)
endif
ifneq ($(NO_PRE_PROC),$(PRE_FLAG))
SUB2_SRC_FILES := $(wildcard $(SRC_DIR)/$(SUB2_MODULE)/*.cpp)
SUB_OBJECTS += $(SUB2_SRC_FILES:$(SRC_DIR)/$(SUB2_MODULE)/%.cpp=$(OBJ_DIR)/$(SUB2_MODULE)/%.o)
endif

$(BIN_DIR)/$(APP_NAME) : $(OBJECTS) $(SUB_OBJECTS)
	@echo "\e[32m""Linking executable: $(BIN_DIR)/$(APP_NAME) with $(SUB_MODULES) included.""\e[0m"
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS) $(SUB_OBJECTS)

endif # ifneq (,$(and $(findstring $(NO_TBL_PROC),$(TBL_FLAG)),$(findstring $(NO_PRE_PROC),$(PRE_FLAG))))

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CCFLAGS) -I$(INCLUDES) -o $@ -c $< -MMD -MF $(patsubst $(OBJ_DIR)/%, $(DEP_DIR)/%.d, $@)

-include $(DEPENDS)

.PHONY : clean
clean :
	@for m in $(SUB_MODULES); do cd $(SRC_DIR)/$$m; make clean SUB_MODULE=$$m || exit "$$?"; done
	rm -rf $(BIN_DIR)/$(APP_NAME) $(OBJECTS) $(DEPENDS)