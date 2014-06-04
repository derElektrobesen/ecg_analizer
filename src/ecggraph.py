import numpy as np
import re
from PyQt4.QtGui import QGroupBox, QWidget
from PyQt4.QtCore import QPointF, QPoint, QRectF
from matplotlib.figure import Figure
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.legend_handler import HandlerLine2D
from matplotlib.lines import Line2D

class EcgGraph(FigureCanvas):
    def __init__(self, parent = None):
        self.__fig = Figure()
        FigureCanvas.__init__(self, self.__fig)
        self.setParent(parent)

        parent.set_ecg_gr(self)

        self.__axes = self.__fig.add_subplot(111)
        self.__axes_coords_ref = self.__axes.transAxes
        self.__data_coords_ref = self.__axes.transData

    def set_graph(self, graph):
        plt = self.__axes
        plt.cla()
        plt.plot(*graph)
        self.__fig.subplots_adjust(left=0.07, right=0.95, top=0.9, bottom=0.1)
        plt.set_xlim(graph[0][0], graph[0][-1])
        self.draw()

    def get_canvas(self):
        return self.__fig

class EcgGraphWrapper(QGroupBox):
    def __init__(self, parent = None):
        QGroupBox.__init__(self, parent)
        self.__ecg_graph = None

    def set_ecg_gr(self, widget):
        self.__ecg_gr = widget

    def get_ecg_gr(self):
        return self.__ecg_gr

class NavBar(NavigationToolbar):
    def __init__(self, group_box_parent = None):
        self.__ecg_gr = group_box_parent.get_ecg_gr()
        QWidget.__init__(self, group_box_parent)
        NavigationToolbar.__init__(self, self.__ecg_gr, self)
