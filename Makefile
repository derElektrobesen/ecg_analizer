SRC_DIR = src
UI_DIR = $(SRC_DIR)/ui
UI_FILES = main_window import_dialog

$(SRC_DIR)/%_ui.py: $(UI_DIR)/%.ui
	echo "from .ecggraph import *" > $(SRC_DIR)/$*_ui.py
	pyuic4 $^ >> $(SRC_DIR)/$*_ui.py

all: $(UI_FILES:%=$(SRC_DIR)/%_ui.py)
clean:
	rm -f $(UI_FILES:%=$(SRC_DIR)/%_ui.py)
