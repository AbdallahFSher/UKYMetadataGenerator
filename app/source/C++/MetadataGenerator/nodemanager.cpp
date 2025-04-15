#include "nodemanager.h"
#include "colorhandler.h"
#include "node.h"
#include <iostream>

using namespace std;

NodeManager::NodeManager(QWidget* parent, ColorHandler colorHandler)
    : parent(parent), colorHandler(colorHandler), currentColumn(0) {
    // nodes is automatically initialized as a member of the class, no need to do it here
}

void NodeManager::processJson(QVariantMap jsonMap, int level, Node* nodeParent) {
    static int processCnt = 0; // Make processCnt a static local variable to keep track of the count
    cout << "PROCESS COUNT: " << processCnt++ << endl;
    cout << "NODELIST LENGTH: " << nodes.size() << endl;
    QList<QString> keys = jsonMap.keys();

    bool firstKid = true;

    for (const QString& key : keys) {
        if (jsonMap[key].canConvert<QVariantMap>()) {
            Node* newNode = new Node(parent, level, nodeParent);
            newNode->row = level;
            newNode->setName(nodes.size());
            newNode->header->setText(key);
            if (nodeParent != nullptr)
                newNode->bottomBar->setText(key);

            nodes.push_back(newNode);

            cout << "MAP##" << level << endl;
            processJson(jsonMap[key].toMap(), level + 1, newNode);

        } else if (jsonMap[key] == jsonMap[key].toString()) {
            Node* newNode = new Node(parent, level, nodeParent);
            newNode->row = level;
            newNode->setName(nodes.size());
            newNode->header->setText(key);
            newNode->bottomBar->setText(jsonMap[key].toString());
            nodes.push_back(newNode);

        } else {
            cout << "THE KEY " << key.toStdString() << " CORRESPONDS TO A LIST BTW" << endl;
            Node* listNode = new Node(parent, level, nodeParent);
            listNode->row = level;
            listNode->setName(nodes.size());
            listNode->header->setText(key);
            listNode->bottomBar->setText("This is a list node");
            nodes.push_back(listNode);

            QList<QVariant> jsonList = jsonMap[key].toList();
            for (const QVariant& child : jsonList) {
                QVariantMap mapChild = child.toMap();
                for (const QString& childKey : mapChild.keys()) {
                    cout << "\nHunting for a string in a dict..." << endl;
                    cout << "ChildKey: " << childKey.toStdString() << endl;
                    cout << "ChildValue: " << mapChild[childKey].toString().toStdString() << endl;

                    if (mapChild[childKey] == mapChild[childKey].toString()) {
                        cout << "GOT one..." << endl;
                        Node* newNode = new Node(parent, level, listNode);
                        newNode->setName(nodes.size());
                        newNode->row = level + 1;
                        newNode->header->setText(childKey);
                        newNode->bottomBar->setText(mapChild[childKey].toString());
                        nodes.push_back(newNode);
                        break;
                    }
                }
                processJson(mapChild, level + 1, listNode);
            }
        }
    }

    colorHandler.setColors(nodes);
}

void NodeManager::addNode(Node* node) {
    nodes.push_back(node);
    colorHandler.setColors(nodes);
}

std::vector<Node*> NodeManager::getNodes() const {
    return nodes;
}
