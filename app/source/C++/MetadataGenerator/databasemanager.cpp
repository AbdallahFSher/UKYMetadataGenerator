#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QFile>
#include <QXmlStreamWriter>
#include <QString>

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

int DatabaseManager::insertSchemaField(int parentId, const QString& name) {
    // Check if parent is an array container
    bool isParentArray = false;
    if (parentId != 0) {
        QSqlQuery parentQuery(db);
        parentQuery.prepare("SELECT name FROM schema_fields WHERE id = :parent_id");
        parentQuery.bindValue(":parent_id", parentId);
        if (parentQuery.exec() && parentQuery.next()) {
            QString parentName = parentQuery.value(0).toString();
            isParentArray = parentName.endsWith("[]");
        }
    }

    // Skip duplicate check for array elements
    if (!isParentArray) {
        QSqlQuery checkQuery(db);
        checkQuery.prepare("SELECT COUNT(*) FROM schema_fields WHERE parent_id = :parent_id AND name = :name");
        checkQuery.bindValue(":parent_id", parentId);
        checkQuery.bindValue(":name", name);
        if (!checkQuery.exec()) {
            qDebug() << "Error checking for duplicates:" << checkQuery.lastError().text();
            return -1;
        }
        checkQuery.next();
        if (checkQuery.value(0).toInt() > 0) {
            qDebug() << "Duplicate schema field found.";
            return -1;
        }
    }

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


QVariantMap DatabaseManager::buildTreeFromDatabase() {
    return buildSubtree(0); // Start with root nodes (parent_id = 0)
}

// In DatabaseManager::buildSubtree, replace the array handling section with:
QVariantMap DatabaseManager::buildSubtree(int parentId) {
    QVariantMap tree;
    QSqlQuery query(db);

    query.prepare("SELECT id, name FROM schema_fields WHERE parent_id = :parent_id ORDER BY id");
    query.bindValue(":parent_id", parentId);

    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError().text();
        return tree;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();

        if (name.endsWith("[]")) {
            QString arrayName = name.left(name.length() - 2);
            QVariantList arrayItems;

            QSqlQuery arrayQuery(db);
            arrayQuery.prepare("SELECT id, name FROM schema_fields WHERE parent_id = :id ORDER BY id");
            arrayQuery.bindValue(":id", id);

            if (arrayQuery.exec()) {
                while (arrayQuery.next()) {
                    int elementId = arrayQuery.value(0).toInt();
                    QString elementName = arrayQuery.value(1).toString();

                    QVariantMap elementData = buildSubtree(elementId);

                    if (elementName.startsWith("value: ")) {
                        // Handle primitive values
                        QString value = elementName.mid(7);
                        bool ok;
                        double numValue = value.toDouble(&ok);
                        if (ok) {
                            arrayItems.append(numValue);
                        } else if (value == "true" || value == "false") {
                            arrayItems.append(value == "true");
                        } else {
                            arrayItems.append(value);
                        }
                    } else {
                        // Handle objects (even if empty)
                        arrayItems.append(elementData);
                    }
                }
            }
            tree[arrayName] = arrayItems.isEmpty() ? QVariantList() : arrayItems;
        }
        else if (name.contains(": ")) {
            QStringList parts = name.split(": ");
            if (parts.size() >= 2) {
                QString key = parts[0];
                QString value = parts[1];
                bool ok;
                double numValue = value.toDouble(&ok);
                if (ok) {
                    tree[key] = numValue;
                } else if (value == "true" || value == "false") {
                    tree[key] = (value == "true");
                } else {
                    tree[key] = value;
                }
            }
        }
        else if (!name.isEmpty()) {
            QVariantMap subtree = buildSubtree(id);
            if (!subtree.isEmpty()) {
                tree[name] = subtree;
            }
        }
    }
    return tree;
}


// Add this method to DatabaseManager class
bool DatabaseManager::clearDatabase()
{
    QSqlQuery query(db);
    return query.exec("DELETE FROM schema_fields");
}


bool DatabaseManager::exportToJson(const QString& filePath) {
    QVariantMap data = buildTreeFromDatabase();
    if (data.isEmpty()) {
        qDebug() << "Warning: No data to export";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(data);
    if (doc.isNull()) {
        qDebug() << "Error: Failed to create JSON document";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << filePath;
        return false;
    }

    if (file.write(doc.toJson(QJsonDocument::Indented)) == -1) {
        qDebug() << "Failed to write to file:" << file.errorString();
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool DatabaseManager::exportToXml(const QString& filePath) {
    QVariantMap data = buildTreeFromDatabase();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    // Convert QVariantMap to XML recursively
    std::function<void(const QVariantMap&)> writeMap = [&](const QVariantMap& map) {
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (it.value().type() == QVariant::Map) {
                xml.writeStartElement(it.key());
                writeMap(it.value().toMap());
                xml.writeEndElement();
            } else {
                xml.writeTextElement(it.key(), it.value().toString());
            }
        }
    };

    writeMap(data);
    xml.writeEndDocument();
    file.close();
    return true;
}

bool DatabaseManager::exportToGaml(const QString& filePath) {
    QVariantMap data = buildTreeFromDatabase();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream << "graph {\n";

    // Convert QVariantMap to GAML recursively
    std::function<void(const QVariantMap&, const QString&, int)> writeMap =
        [&](const QVariantMap& map, const QString& parentName, int depth) {
            QString indent(depth * 2, ' ');

            for (auto it = map.begin(); it != map.end(); ++it) {
                QString nodeName = parentName.isEmpty() ? it.key() : parentName + "." + it.key();

                if (it.value().type() == QVariant::Map) {
                    stream << indent << "node " << nodeName << " {\n";
                    writeMap(it.value().toMap(), nodeName, depth + 1);
                    stream << indent << "}\n";
                } else {
                    stream << indent << "attribute " << it.key() << " = \""
                           << it.value().toString() << "\";\n";
                }
            }
        };

    writeMap(data, "", 1);
    stream << "}\n";
    file.close();
    return true;
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
