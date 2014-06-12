OS = Linux # Windows

SRC_DIR = src
UI_DIR = $(SRC_DIR)/ui
UI_FILES = main_window import_dialog

C_SRC = algo algo_ex algo_main
LIB_NAME = algo

CFLAGS = -DLIB_NAME="\"$(LIB_NAME)\"" -lpython3 -Wall -Werror -fmax-errors=5
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

$(LIB_NAME).$(LIB_POSTFIX): $(C_SRC:%=%.o)
	$(CC) -shared -o $@ $(C_SRC:%=%.o)

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -fPIC

$(SRC_DIR)/%_ui.py: $(UI_DIR)/%.ui
	echo "from .ecggraph import *" > $(SRC_DIR)/$*_ui.py
	pyuic4 $^ >> $(SRC_DIR)/$*_ui.py

clean:
	rm -f $(UI_FILES:%=$(SRC_DIR)/%_ui.py) $(C_SRC:%=%.o) $(LIB_NAME).$(LIB_POSTFIX)

m.PHONY: clean
