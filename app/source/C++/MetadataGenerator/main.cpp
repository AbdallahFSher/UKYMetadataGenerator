#include <QApplication>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include "schemahandler.h"
#include "DatabaseManager.h"
#include "mainwindow.h"

void insertFieldTree(const std::shared_ptr<Field>& node, int parentId, DatabaseManager& dbManager) {
    int currentId = dbManager.insertSchemaField(parentId, QString::fromStdString(node->name));
    if (currentId == -1) return;
    for (const auto& child : node->children) {
        insertFieldTree(child, currentId, dbManager);
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Open the database FIRST
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.openDatabase("example.db")) {
        return -1;
    }

    // Create the table BEFORE inserting data
    QSqlQuery query(dbManager.database());
    if (!query.exec("CREATE TABLE IF NOT EXISTS schema_fields ("
                    "id INTEGER PRIMARY KEY, "
                    "parent_id INTEGER, "
                    "name TEXT, "
                    "FOREIGN KEY(parent_id) REFERENCES schema_fields(id))")) {
        qDebug() << "Error creating schema table:" << query.lastError().text();
        return -1;
    }

    // Then handle the schema
    SchemaHandler schemaHandler;
    Schema* currentSchema = schemaHandler.addSchema("C:/Users/Caleb Fields/Downloads/UKYMetadataGenerator/app/examples/exampleSchema.txt");

    if (currentSchema) {
        auto rootField = currentSchema->getRoot();
        if (rootField) {
            insertFieldTree(rootField, 0, dbManager); // 0 indicates no parent
        }
    }

    // Print the table contents
    qDebug() << "\nDatabase Contents:";
    if (!query.exec("SELECT id, parent_id, name FROM schema_fields ORDER BY id")) {
        qDebug() << "Error reading schema table:" << query.lastError().text();
    } else {
        qDebug() << "ID\tParent\tName";
        qDebug() << "---------------------";
        while (query.next()) {
            qDebug() << query.value(0).toInt() << "\t"
                     << query.value(1).toInt() << "\t"
                     << query.value(2).toString();
        }
    }

    MainWindow *mw = new MainWindow();
    mw->show();

    return app.exec();
}
