#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    static DatabaseManager& instance();
    ~DatabaseManager();

    bool openDatabase(const QString& path);
    void closeDatabase();
    QSqlDatabase& database();
    int insertSchemaField(int parentId, const QString& name);
    void printSchemaTable();

private:
    DatabaseManager();
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
