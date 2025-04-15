#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class DatabaseManager {
public:
    static DatabaseManager& instance();
    ~DatabaseManager();

    bool openDatabase(const QString& path);
    void closeDatabase();
    QSqlDatabase& database();

    // Schema Field Management
    int insertSchemaField(int parentId, const QString& key, const QString& value);
    bool insertSchemaFieldWithTransaction(int parentId, const QString& key, const QString& value);  // Transaction-based insert

    void printSchemaTable();

    // Export functions
    bool exportToJson(const QString& filePath);
    bool exportToXml(const QString& filePath);
    bool exportToGaml(const QString& filePath);
    bool exportToYaml(const QString& filePath);

    bool clearDatabase();

private:
    DatabaseManager();
    QSqlDatabase db;

    // Helper methods
    QVariantMap buildTreeFromDatabase();
    QVariantMap buildSubtree(int parentId, int depth = 0);
};

#endif // DATABASEMANAGER_H
