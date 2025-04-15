#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include "node.h"
#include "colorhandler.h"
#include <QWidget>
#include <vector>
#include <QVariantMap>

class NodeManager
{
public:
    // Constructor
    NodeManager(QWidget* parent, ColorHandler colorHandler);

    // Methods
    void processJson(QVariantMap jsonMap, int level, Node* nodeParent = nullptr);
    Node* createNode(int level, Node* nodeParent = nullptr);  // Using pointer for consistency
    void addNode(Node* node);
    std::vector<Node*> getNodes() const;  // Marking as const since it's a getter

    // Member variables
    QWidget* parent;
    ColorHandler colorHandler;
    int currentColumn;

private:
    // Internal node container
    std::vector<Node*> nodes;
};

#endif // NODEMANAGER_H
