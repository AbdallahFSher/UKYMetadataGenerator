#include <QApplication>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include "DatabaseManager.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Open the database
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.openDatabase("example.db")) {
        return -1;
    }

    // Create a table
    QSqlQuery query(dbManager.database());
    if (!query.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)")) {
        qDebug() << "Error: Could not create table:" << query.lastError().text();
        return -1;
    }

    // Insert data
    query.prepare("INSERT INTO users (name) VALUES (:name)");
    query.bindValue(":name", "John Doe");
    if (!query.exec()) {
        qDebug() << "Error: Could not insert data:" << query.lastError().text();
        return -1;
    }

    // Query data
    if (!query.exec("SELECT id, name FROM users")) {
        qDebug() << "Error: Could not query data:" << query.lastError().text();
        return -1;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        qDebug() << "ID:" << id << "Name:" << name;
    }

    MainWindow mw = new MainWindow();
    mw.show();

    return app.exec();
}
