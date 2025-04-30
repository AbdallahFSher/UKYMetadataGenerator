#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QFile>
#include <QXmlStreamWriter>
#include <QRegularExpression>
#include <QTextStream>
#include <QDebug>
#include <functional>

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

// Opens the SQLite database and enables foreign keys
bool DatabaseManager::openDatabase(const QString& path) {
    db.setDatabaseName(path);
    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return false;
    }

    // enable foreign-key support so ON DELETE CASCADE actually works
    {
        QSqlQuery pragma(db);
        if (!pragma.exec("PRAGMA foreign_keys = ON")) {
            qDebug() << "Could not enable foreign keys:" << pragma.lastError().text();
        } else {
            qDebug() << "Foreign keys enabled";
        }
    }

    qDebug() << "Database opened:" << path;
    return true;
}

// Closes the database connection
void DatabaseManager::closeDatabase() {
    if (db.isOpen()) {
        db.close();
    }
}

// Provides reference to internal QSqlDatabase
QSqlDatabase& DatabaseManager::database() {
    return db;
}

// Inserts a new field into the schema_fields table, avoiding duplicates under the same parent
int DatabaseManager::insertSchemaField(int parentId, const QString& name, const QString& type)
{
    // check for duplicate under same parent (NULL means root)
    QSqlQuery checkQuery(db);
    if (parentId > 0) {
        checkQuery.prepare("SELECT COUNT(*) FROM schema_fields WHERE parent_id = ? AND name = ?");
        checkQuery.addBindValue(parentId);
        checkQuery.addBindValue(name);
    }
    else {
        checkQuery.prepare("SELECT COUNT(*) FROM schema_fields WHERE parent_id IS NULL AND name = ?");
        checkQuery.addBindValue(name);
    }
    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "Duplicate key under the same parent:" << name;
        return -1;
    }

    // insert, binding NULL for root-level entries
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO schema_fields (parent_id, name, type) VALUES (?, ?, ?)");
    if (parentId > 0)
        insertQuery.addBindValue(parentId);
    else
        insertQuery.addBindValue(QVariant());    // bind NULL for parent_id

    insertQuery.addBindValue(name);
    insertQuery.addBindValue(type);

    if (!insertQuery.exec()) {
        qDebug() << "Insertion failed:" << insertQuery.lastError().text();
        return -1;
    }
    return insertQuery.lastInsertId().toInt();
}

// Inserts a field within a transaction for rollback safety
bool DatabaseManager::insertSchemaFieldWithTransaction(int parentId, const QString& key, const QString& value)
{
    if (!db.transaction()) {
        qDebug() << "Transaction start failed.";
        return false;
    }

    int id = insertSchemaField(parentId, key, value);
    if (id == -1) {
        db.rollback();
        return false;
    }

    return db.commit();
}

// Constructs a full hierarchical tree from schema_fields
QVariantMap DatabaseManager::buildTreeFromDatabase() {
    return buildSubtree(0); // Root parent_id = 0
}

// Recursive helper to build tree structure from schema_fields
QVariantMap DatabaseManager::buildSubtree(int parentId, int depth) {
    QVariantMap tree;
    if (depth > 100) {
        qDebug() << "Maximum recursion depth reached!";
        return tree;
    }

    QSqlQuery query(db);
    if (parentId == 0) {
        // fetch top‐level nodes
        query.prepare("SELECT id, name, type FROM schema_fields WHERE parent_id IS NULL");
    } else {
        query.prepare("SELECT id, name, type FROM schema_fields WHERE parent_id = ?");
        query.addBindValue(parentId);
    }

    if (!query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return tree;
    }

    while (query.next()) {
        int id      = query.value(0).toInt();
        QString key = query.value(1).toString();
        QString type= query.value(2).toString();

        // arrays end with "[]"
        if (key.endsWith("[]")) {
            QString arrayName = key.left(key.length() - 2);
            QVariantList arrayItems;

            QSqlQuery arrayQuery(db);
            arrayQuery.prepare("SELECT id FROM schema_fields WHERE parent_id = ?");
            arrayQuery.addBindValue(id);

            if (arrayQuery.exec()) {
                while (arrayQuery.next()) {
                    int childId = arrayQuery.value(0).toInt();
                    arrayItems.append(buildSubtree(childId, depth + 1));
                }
            }
            tree[arrayName] = arrayItems;

        } else {
            // does this node have children?
            QSqlQuery childQuery(db);
            childQuery.prepare("SELECT COUNT(*) FROM schema_fields WHERE parent_id = ?");
            childQuery.addBindValue(id);

            bool hasChildren = false;
            if (childQuery.exec() && childQuery.next())
                hasChildren = (childQuery.value(0).toInt() > 0);

            if (hasChildren) {
                // recurse into object
                tree[key] = buildSubtree(id, depth + 1);
            } else {
                // leaf: use the stored type as value
                tree[key] = type;
            }
        }
    }

    return tree;
}

// Deletes all rows from the schema_fields table
bool DatabaseManager::clearDatabase() {
    QSqlQuery query(db);
    bool success = query.exec("DELETE FROM schema_fields");
    qDebug() << (success ? "Database cleared." : "Failed to clear database:" + query.lastError().text());
    return success;
}

// Recursively deletes a node and its subtree from schema_fields
void DatabaseManager::deleteSubtree(int id)
{
    // first delete all children
    QSqlQuery q(db);
    q.prepare("SELECT id FROM schema_fields WHERE parent_id = ?");
    q.addBindValue(id);
    if (q.exec()) {
        while (q.next()) {
            deleteSubtree(q.value(0).toInt());
        }
    }
    // then delete this node
    QSqlQuery del(db);
    del.prepare("DELETE FROM schema_fields WHERE id = ?");
    del.addBindValue(id);
    del.exec();
}

// -------- Export Helpers -------- //

// Exports schema tree to a JSON file
bool DatabaseManager::exportToJson(const QString& filePath) {
    QVariantMap data = buildTreeFromDatabase();
    if (data.isEmpty()) {
        qDebug() << "No data to export.";
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(data);
    if (doc.isNull()) {
        qDebug() << "Failed to create JSON document.";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return false;
    }

    qint64 bytesWritten = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    if (bytesWritten == -1) {
        qDebug() << "Failed to write JSON to file:" << file.errorString();
        return false;
    }

    qDebug() << "JSON exported to:" << filePath;
    return true;
}

// Exports schema tree to an XML file
bool DatabaseManager::exportToXml(const QString& filePath) {
    QVariantMap data = buildTreeFromDatabase();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("root"); // Root element for the XML

    auto sanitize = [](const QString& name) {
        QString sanitized = name;
        sanitized.replace(" ", "_")
            .replace("/", "_")
            .remove(QRegularExpression("[^a-zA-Z0-9_\\-]"));
        return sanitized;
    };

    std::function<void(const QVariant&, const QString&)> writeVariant;
    writeVariant = [&](const QVariant& var, const QString& tag) {
        QString elementName = sanitize(tag);

        if (var.type() == QVariant::Map) {
            // Handle objects (nested elements)
            xml.writeStartElement(elementName);
            QVariantMap map = var.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                writeVariant(it.value(), it.key());
            }
            xml.writeEndElement();
        } else if (var.type() == QVariant::List) {
            // Handle arrays (use parent's tag name for each element)
            QVariantList list = var.toList();
            if (list.isEmpty()) return;

            xml.writeStartElement(elementName);
            for (const auto& item : list) {
                writeVariant(item, elementName); // Use parent's tag name for array elements
            }
            xml.writeEndElement();
        } else {
            // Handle leaf nodes (text content)
            xml.writeTextElement(elementName, var.toString());
        }
    };

    // Start writing from the root of the data
    for (auto it = data.begin(); it != data.end(); ++it) {
        writeVariant(it.value(), it.key());
    }

    xml.writeEndElement(); // Close root
    xml.writeEndDocument();
    return true;
}

// Exports schema tree to an GAML file
bool DatabaseManager::exportToGaml(const QString& filePath) {
    QVariantMap data = buildTreeFromDatabase();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out << "graph {\n";

    std::function<void(const QVariant&, const QString&, int)> writeGaml;
    writeGaml = [&](const QVariant& var, const QString& parentPath, int depth) {
        QString indent(depth * 2, ' ');

        if (var.type() == QVariant::Map) {
            QVariantMap map = var.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                QString key = it.key();
                QString sanitizedKey = key.replace(" ", "").replace("&", "And");
                QString currentPath = parentPath.isEmpty()
                                          ? sanitizedKey
                                          : parentPath + "." + sanitizedKey;

                // Write attributes directly for leaf nodes
                if (it.value().type() != QVariant::Map &&
                    it.value().type() != QVariant::List) {
                    out << indent << "attribute " << sanitizedKey
                        << " = \"" << it.value().toString() << "\";\n";
                }
                // Handle nested objects/arrays
                else {
                    out << indent << "node " << sanitizedKey << " {\n";
                    writeGaml(it.value(), currentPath, depth + 1);
                    out << indent << "}\n";
                }
            }
        }
        else if (var.type() == QVariant::List) {
            QVariantList list = var.toList();
            for (int i = 0; i < list.size(); ++i) {
                QString elementPath = parentPath + "[" + QString::number(i) + "]";
                const QVariant& element = list[i];

                out << indent << "node [" << i << "] {\n";
                writeGaml(element, elementPath, depth + 1);
                out << indent << "}\n";
            }
        }
    };

    writeGaml(data, "", 1);
    out << "}\n";
    return true;
}

// Exports schema tree to an Yaml file
bool DatabaseManager::exportToYaml(const QString& filePath) {
    QVariantMap data = buildTreeFromDatabase();
    if (data.isEmpty()) {
        qDebug() << "No data to export.";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    std::function<void(const QVariant&, int)> writeYaml;
    writeYaml = [&out, &writeYaml](const QVariant& var, int indentLevel) {
        QString indent(indentLevel * 2, ' '); // 2 spaces per indent level

        if (var.type() == QVariant::Map) {
            QVariantMap map = var.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                QString key = it.key();
                const QVariant& value = it.value();

                out << indent << key << ":";
                if (value.type() == QVariant::Map || value.type() == QVariant::List) {
                    out << "\n";
                    writeYaml(value, indentLevel + 1);
                } else {
                    out << " " << value.toString() << "\n";
                }
            }
        } else if (var.type() == QVariant::List) {
            QVariantList list = var.toList();
            for (const QVariant& item : list) {
                out << indent << "-";
                if (item.type() == QVariant::Map || item.type() == QVariant::List) {
                    out << "\n";
                    writeYaml(item, indentLevel + 1);
                } else {
                    out << " " << item.toString() << "\n";
                }
            }
        } else {
            out << indent << var.toString() << "\n";
        }
    };

    writeYaml(data, 0); // Start with indentation level 0 (root)

    file.close();
    qDebug() << "YAML exported to:" << filePath;
    return true;
}

// Prints the database, used in debugging
void DatabaseManager::printSchemaTable() {
    QSqlQuery query(db);
    if (!query.exec("SELECT id, parent_id, name, type FROM schema_fields ORDER BY id")) {
        qDebug() << "Failed to print schema table:" << query.lastError().text();
        return;
    }

    qDebug() << "\nSchema Table:";
    qDebug() << "ID\tParent ID\tKey\tValue";
    while (query.next()) {
        qDebug() << query.value(0).toInt()
        << "\t" << query.value(1).toInt()
        << "\t" << query.value(2).toString()
        << "\t" << query.value(3).toString();
    }
}
