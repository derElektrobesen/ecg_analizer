from .main_window_ui import Ui_MainWindow
from .import_dialog_ui import Ui_ImportDialog

import os.path
from .config import Config, DBUser

from PyQt4.QtGui import QMainWindow, QFileDialog, QDialog, QMessageBox, QAction, QActionGroup
from PyQt4.QtCore import *

from algo import *

class ImportDialog(Ui_ImportDialog, QDialog):
    def __init__(self, parent = None, callback = None):
        QDialog.__init__(self, parent)
        self.setupUi(self)
        self.__callback = callback

    @pyqtSlot()
    def on_toolButton_clicked(self):
        filename = QFileDialog.getOpenFileName(self, "Import ECG", "", "Txt files (*.txt)")
        self.file_name_edt.setText(filename)

    @pyqtSlot()
    def on_import_btn_clicked(self):
        name = self.ecg_name_edt.text()
        filename = self.file_name_edt.text()

        if name.strip() == "" or filename.strip() == "":
            QMessageBox.warning(self, "Error", "All fiels must be filled")
            return

        if not os.path.isfile(filename):
            QMessageBox.warning(self, "Error", "File `" + filename + "` not exists")
            return

        self.close()

        if self.__callback:
            self.__callback(name, filename)

class MainWindow(Ui_MainWindow, QMainWindow):
    def __init__(self, parent = None):
        QMainWindow.__init__(self, parent)
        self.setupUi(self)
        self.__checked_etalon = None
        self.set_etalons()

    @pyqtSlot()
    def on_act_import_ecg_triggered(self):
        filedialog = ImportDialog(self, self.start_import)
        filedialog.show()

    def start_import(self, name, filename):
        f = open(filename, 'r')
        q = DBUser.q()
        q.prepare("INSERT INTO etalons(name) values (?) ON DUPLICATE KEY UPDATE id = last_insert_id(id)")
        q.bindValue(0, name)
        q.exec_()
        q.exec_("BEGIN TRANSACTION")
        last_id = DBUser.last_id()
        query = "INSERT INTO data(etalons_id, x, y) values "
        t = 0
        i = 0
        qstr = ""
        data = [[], []]
        for line in f:
            if i:
                qstr += ", "
            i += 1
            n = float(line)
            qstr += "(%d, %f, %f)" % (last_id, t, n)
            data[0].append(t)
            data[1].append(n)
            if i % 300 == 0:
                q.exec_(query + qstr)
                qstr = ""
                i = 0
            t += Config.diagram_step()
        if qstr:
            q.exec_(query + qstr)
        q.exec_("COMMIT")
        self.set_etalons()
        self.__etalons_data[name] = { 'id': last_id, 'data': data }

    def set_etalons(self):
        q = DBUser.q()
        q.exec_("SELECT name, id FROM etalons ORDER BY NAME ASC")
        self.acts_group = QActionGroup(self)
        self.etalon_menu.clear()
        first_act = None
        self.__etalons_data = {}
        while q.next():
            name = q.value(0)
            act = QAction(self)
            act.setObjectName("etalon_" + name + "_act")
            act.setText(name)
            if name not in self.__etalons_data:
                self.__etalons_data[name] = { 'id': q.value(1), 'data': [] }
            self.etalon_menu.addAction(act)
            act.setCheckable(True)
            act.setActionGroup(self.acts_group)
            if not self.__checked_etalon:
                if not first_act:
                    act.setChecked(True)
                    first_act = act
            elif name == self.__checked_etalon:
                act.setChecked(True)
                first_act = act

        self.etalon_menu.addSeparator()
        self.act_import_ecg = QAction(self)
        self.act_import_ecg.setObjectName("act_import_ecg")
        self.etalon_menu.addAction(self.act_import_ecg)
        self.act_import_ecg.setText("Import ECG")
        QObject.connect(self.act_import_ecg, SIGNAL("triggered()"), self.on_act_import_ecg_triggered)
        QObject.connect(self.acts_group, SIGNAL("triggered(QAction*)"), self.on_etalon_changed)
        if first_act:
            self.on_etalon_changed(first_act)

    @pyqtSlot(QAction)
    def on_etalon_changed(self, act):
        self.__checked_etalon = act.text()
        self.load_etalon_data(act.text())
        self.ecg_graph_etalon.set_graph(self.__etalons_data[act.text()]['data'])
        self.count_graph(act.text())

    def count_graph(self, gr_text):
        data = self.__etalons_data[gr_text]
        if 'counted' not in data:
            gr = data['data']
            data['counted'] = band_filter(gr)
        self.ecg_graph_real.set_graph(data['counted'])

    def load_etalon_data(self, name):
        if not self.__etalons_data[name]['data']:
            q = DBUser.q()
            q.prepare("SELECT x, y FROM data WHERE etalons_id = ?")
            q.bindValue(0, self.__etalons_data[name]['id'])
            q.exec_()
            data = ([], [])
            while q.next():
                data[0].append(q.value(0))
                data[1].append(q.value(1))
            self.__etalons_data[name]['data'] = (tuple(data[0]), tuple(data[1]))
