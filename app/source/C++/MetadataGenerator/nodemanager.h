#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include "node.h"
#include "colorhandler.h"
//#include "mainwindow.h"

using namespace std;

class NodeManager
{
public:
    NodeManager(QWidget* parent, ColorHandler colorHandler);
    void processJson(QVariantMap jsonMap, int level, Node* nodeParent = nullptr);
    Node createNode(int level,  Node nodeParent = NULL);
    void addNode(Node* node);
    QWidget* parent;
    ColorHandler colorHandler;

private:
    std::vector<Node*> nodes;
};

#endif // NODEMANAGER_H
