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

private:
    DatabaseManager();
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
