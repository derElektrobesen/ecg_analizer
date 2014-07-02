import numpy as np
import re
from PyQt4.QtGui import QGroupBox, QWidget, QPainter
from PyQt4.QtCore import QPointF, QPoint, QRectF, Qt
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

    def set_graph(self, graph, points = None):
        plt = self.__axes
        plt.cla()
        plt.plot(*graph)
        self.__fig.subplots_adjust(left=0.07, right=0.95, top=0.9, bottom=0.2)
        plt.set_xlim(graph[0][0], graph[0][-1])

        self.__data = graph
        self.__points = points

        self.__axes.set_xlabel("Time, Sec")
        self.__axes.set_ylabel("Amplitude, mV")
        self.draw()

    def get_canvas(self):
        return self.__fig

    def paintEvent(self, e):
        FigureCanvas.paintEvent(self, e)
        if not self.__points:
            return

        points = []
        d_coords = []

        last_x = -1
        off = -1
        for pnt in self.__points:
            points.append((self.__data[0][pnt], self.__data[1][pnt]))
            if last_x < 0:
                last_x = self.__data[0][pnt]
            else:
                if off == -1:
                    off = (self.__data[0][pnt] - last_x) / 2.0
                d_coords.append((last_x + off, 0))
                last_x = self.__data[0][pnt]

        painter = QPainter(self)
        ptr = self.__axes.transData
        r = 4
        h = self.height()
        painter.setPen(Qt.red)
        b = painter.brush()
        b.setColor(Qt.white)
        b.setStyle(Qt.SolidPattern)
        painter.setBrush(b)
        
        bottom = ptr.transform([0,0])
        last_x = bottom[0]
        last_xx = 0
        for pnt in range(len(points) - 1):
            tmp = ptr.transform(points[pnt])
            painter.drawEllipse(tmp[0] - r / 2, h - tmp[1] - r / 2, r, r)
            name = "%1.2e" % points[pnt][1]
            painter.drawText(tmp[0], h - tmp[1], name)

            if last_xx != 0:
                painter.drawText(last_x + (tmp[0] - last_x) / 2 - 10, h - bottom[1], "%1.2f" % (points[pnt][0] - last_xx))
            last_x = tmp[0]
            last_xx = points[pnt][0]
        return
        for pnt in range(len(d_coords)):
            d_coords[pnt] = self.__axes.transAxes.inverted().transform(d_coords[pnt])


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
