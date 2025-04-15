#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include "node.h"
#include "colorhandler.h"
#include <QWidget>
#include <vector>
#include <QVariantMap>

class NodeManager {
public:
    NodeManager(QWidget* parent, ColorHandler colorHandler);
    void processJson(const QVariantMap& json, int parentDatabaseId, QWidget* parentWidget = nullptr);
    void addNode(Node* node);
    std::vector<Node*> getNodes() const; // Return std::vector

private:
    void createNodesFromJson(const QVariantMap& json, int parentDatabaseId, QWidget* parentWidget);

    QWidget* parent;
    ColorHandler colorHandler;
    int currentColumn;
    std::vector<Node*> nodes; // Use std::vector
    int databaseId = -1;
};

#endif // NODEMANAGER_H
