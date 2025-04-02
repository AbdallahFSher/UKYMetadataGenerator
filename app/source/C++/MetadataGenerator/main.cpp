#include <QApplication>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include "schemahandler.h"
#include "DatabaseManager.h"
#include "mainwindow.h"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);



    MainWindow *mw = new MainWindow();
    mw->show();

    return app.exec();
}
