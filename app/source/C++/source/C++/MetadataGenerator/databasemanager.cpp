#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
}

DatabaseManager::~DatabaseManager() {
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString& path) {
    db.setDatabaseName(path);
    if (!db.open()) {
        qDebug() << "Error: Could not open database:" << db.lastError().text();
        return false;
    }
    return true;
}

void DatabaseManager::closeDatabase() {
    if (db.isOpen()) {
        db.close();
    }
}

QSqlDatabase& DatabaseManager::database() {
    return db;
}
