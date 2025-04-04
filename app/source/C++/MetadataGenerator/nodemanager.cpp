#include "nodemanager.h"
#include "node.h"
#include <iostream>

using namespace std;

NodeManager::NodeManager(QWidget* parent) {
    this->parent = parent;
    std::vector<Node*> nodes = vector<Node*>(); //This vector should be managed by a Node Manager class
}

void NodeManager::processJson(QVariantMap jsonMap, int level, QWidget* rent) {
    cout << "I AM DOING STUDF TOO" << endl;
    Node *newNode = new Node(rent, 0);
    newNode->setText("I WAS BOURNE OF THE NODE MANAGER");

    QList<QString> keys = jsonMap.keys();

    for (QString key:keys) {
        Node *newNode = new Node(rent, 0);

        if(jsonMap[key].canConvert<QList<QVariantMap>>()) {
            for (QVariant child:(jsonMap[key].toList())) {
                QVariantMap mapChild = child.toMap();
                processJson(mapChild, level + 1, rent);
            }
        } else if (jsonMap[key].canConvert<QVariantMap>()) {
            processJson(jsonMap[key].toMap(), level + 1, rent);
        } else {
            jsonMap[key].toString();
        }

    }

}

void NodeManager::addNode(Node* node) {
    nodes.push_back(node);
}
