OS = Linux # Windows

SRC_DIR = src
UI_DIR = $(SRC_DIR)/ui
UI_FILES = main_window import_dialog

BUILD_DIR=bin

C_SRC = algo algo_ex algo_main
LIB_NAME = algo

CFLAGS = -DLIB_NAME="\"$(LIB_NAME)\"" -lpython3 -Wall -Werror -fmax-errors=5 -ggdb3 -fPIC -O0
LIB_POSTFIX =
CC =
PY_LIB_DIR =

ifeq ($(OS), Windows)
	CFLAGS += -DOS_WINDOWS
	LIB_POSTFIX = dll
	CC = gcc 				# ???
	CFLAGS += -Ilalala 		# Python.h file path
else
	CFLAGS += -DOS_LINUX
	LIB_POSTFIX = so
	CC = gcc
	CFLAGS += -I/usr/include/python3.4m/
endif

all: $(LIB_NAME).$(LIB_POSTFIX) $(UI_FILES:%=$(SRC_DIR)/%_ui.py)

$(LIB_NAME).$(LIB_POSTFIX): $(C_SRC:%=$(BUILD_DIR)/%.o)
	$(CC) -shared -o $@ $(C_SRC:%=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR) > /dev/null 2>&1
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%_ui.py: $(UI_DIR)/%.ui
	@echo "from .ecggraph import *" > $(SRC_DIR)/$*_ui.py
	pyuic4 $^ >> $(SRC_DIR)/$*_ui.py

clean:
	rm -f $(UI_FILES:%=$(SRC_DIR)/%_ui.py) $(C_SRC:%=$(BUILD_DIR)/%.o) $(LIB_NAME).$(LIB_POSTFIX)

m.PHONY: clean
