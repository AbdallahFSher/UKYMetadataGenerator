#include "nodemanager.h"
#include "colorhandler.h"
#include "node.h"
#include <iostream>

using namespace std;

NodeManager::NodeManager(QWidget* parent, ColorHandler colorHandler) {
    this->parent = parent;
    this->colorHandler = colorHandler;
    std::vector<Node*> nodes = vector<Node*>(); //This vector should be managed by a Node Manager class
}

void NodeManager::processJson(QVariantMap jsonMap, int level, Node* nodeParent) {
    QList<QString> keys = jsonMap.keys();

    for (QString key:keys) {
        Node *newNode = new Node(parent, level, nodeParent);
        newNode->header->setText(key);
        newNode->adjustSize();
        nodes.push_back(newNode);

        if (jsonMap[key].canConvert<QVariantMap>()) {
            cout << "MAP##" << level << endl;
            processJson(jsonMap[key].toMap(), level + 1, newNode);
        } else if (jsonMap[key] == jsonMap[key].toString()) {
            newNode->bottomBar->setText(jsonMap[key].toString());
            newNode->bottomBar->adjustSize();
            newNode->adjustSize();
            newNode->bottomBar->updateGeometry();
            newNode->updateGeometry();
        } else {
            cout << "THE KEY " << key.toStdString() << " CORRESPONDS TOA  LIST BTW" << endl;

            for (QVariant child:(jsonMap[key].toList())) {
                QVariantMap mapChild = child.toMap();
                processJson(mapChild, level + 1, newNode);
            }
        }
    }

    colorHandler.setColors(nodes);
}

void NodeManager::addNode(Node* node) {
    nodes.push_back(node);
}
