from PyQt4.QtSql import QSqlQuery, QSqlDatabase

class Config:
    @staticmethod
    def db_name():
        return 'ecg_analizer'

    @staticmethod
    def db_user():
        return 'ecg_a_user'

    @staticmethod
    def db_pass():
        return '123456789'

    @staticmethod
    def db_host():
        return 'localhost'

    @staticmethod
    def diagram_step():
        return 0.1

class MySQL:
    def __init__(self):
        self.__db = QSqlDatabase.addDatabase('QMYSQL', Config.db_name())
        self.__db.setHostName(Config.db_host())
        self.__db.setDatabaseName(Config.db_name())
        self.__db.setUserName(Config.db_user())
        self.__db.setPassword(Config.db_pass())

        if not self.__db.open():
            raise Exception("Database open failure: %s" % self.__db.lastError().text())

    def close_connection(self):
        self.__db.close()
        self.__db = None
        QSqlDatabase.removeDatabase(Config.db_name())

class DBUser:
    @staticmethod
    def q():
        return QSqlQuery(QSqlDatabase.database(Config.db_name()))

    @staticmethod
    def last_id():
        q = DBUser.q()
        q.prepare("SELECT last_insert_id()")
        q.exec_()
        q.next()
        r = q.value(0)
        q.finish()
        return r
