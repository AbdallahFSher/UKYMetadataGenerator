#include "nodemanager.h"
#include "colorhandler.h"
#include "databasemanager.h"
#include "node.h"
#include <iostream>

using namespace std;

NodeManager::NodeManager(QWidget* parent, ColorHandler colorHandler)
    : parent(parent), colorHandler(colorHandler), currentColumn(0) {
    // nodes is automatically initialized as a member of the class, no need to do it here
}

void NodeManager::processJson(const QVariantMap& json, int parentDatabaseId, QWidget* parentWidget) {
    DatabaseManager& db = DatabaseManager::instance();

    // Clear existing database entries for this parent
    db.deleteSchemaFieldsByParentId(parentDatabaseId);

    // Clear in-memory nodes
    qDeleteAll(nodes);
    nodes.clear();

    // Process new JSON
    createNodesFromJson(json, parentDatabaseId, parentWidget);
}
void NodeManager::createNodesFromJson(const QVariantMap& json, int parentDatabaseId, QWidget* parentWidget) {
    DatabaseManager& db = DatabaseManager::instance();

    for (auto it = json.begin(); it != json.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value();

        // Insert into database
        int newId = db.insertSchemaField(parentDatabaseId, key,
                                         value.type() == QVariant::Map ? "object" : "string");
        if (newId == -1) {
            qWarning() << "Failed to insert JSON node:" << key;
            continue;
        }

        // Create UI node
        Node* newNode = new Node(parentWidget, 0, nullptr);
        newNode->setDatabaseId(newId);
        newNode->setKey(key);
        newNode->setValue(value.toString());
        nodes.push_back(newNode);

        // Add node to parent's layout
        if (parentWidget) {
            if (!parentWidget->layout()) {
                parentWidget->setLayout(new QVBoxLayout());
            }
            parentWidget->layout()->addWidget(newNode);
        }

        // Recursive processing for nested objects
        if (value.type() == QVariant::Map) {
            createNodesFromJson(value.toMap(), newId, newNode);
        }
    }
}

void NodeManager::addNode(Node* node) {
    nodes.push_back(node);
    colorHandler.setColors(nodes);
}

std::vector<Node*> NodeManager::getNodes() const {
    return nodes;
}
