#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>

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

// TODO: Update to not add duplicate schema fields
int DatabaseManager::insertSchemaField(int parentId, const QString& name) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO schema_fields (parent_id, name) VALUES (:parent_id, :name)");
    query.bindValue(":parent_id", parentId);
    query.bindValue(":name", name);
    if (!query.exec()) {
        qDebug() << "Error inserting schema field:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

void DatabaseManager::printSchemaTable() {
    QSqlQuery query(db);
    if (!query.exec("SELECT id, parent_id, name FROM schema_fields ORDER BY id")) {
        qDebug() << "Error printing schema table:" << query.lastError().text();
        return;
    }

    qDebug() << "\nSchema Table Contents:";
    qDebug() << "ID\tParent\tName";
    qDebug() << "---------------------";

    while (query.next()) {
        int id = query.value(0).toInt();
        int parentId = query.value(1).toInt();
        QString name = query.value(2).toString();
        qDebug() << id << "\t" << parentId << "\t" << name;
    }
}

void DatabaseManager::closeDatabase() {
    if (db.isOpen()) {
        db.close();
    }
}

QSqlDatabase& DatabaseManager::database() {
    return db;
}
