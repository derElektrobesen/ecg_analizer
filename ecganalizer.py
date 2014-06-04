#!/usr/bin/python3
# encoding: utf-8

import sys
import os

sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from src import MainWindow, MySQL
from PyQt4.QtGui import QApplication

if __name__ == '__main__':
    app = QApplication(sys.argv)
    mysql = MySQL()
    wnd = MainWindow()
    wnd.show()
    app.exec_()
    mysql.close_connection()
