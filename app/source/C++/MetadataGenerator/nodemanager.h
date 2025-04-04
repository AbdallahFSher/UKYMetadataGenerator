#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include "node.h"
//#include "mainwindow.h"

using namespace std;

class NodeManager
{
public:
    NodeManager(QWidget* parent);
    void processJson(QVariantMap jsonMap, int level, QWidget* rent);
    Node createNode(int level,  Node nodeParent = NULL);
    void addNode(Node* node);
    QWidget* parent;

private:
    std::vector<Node*> nodes;
};

#endif // NODEMANAGER_H
