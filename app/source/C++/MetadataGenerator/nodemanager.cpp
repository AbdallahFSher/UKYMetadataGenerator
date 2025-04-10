#include "nodemanager.h"
#include "colorhandler.h"
#include "node.h"
#include <iostream>

using namespace std;

NodeManager::NodeManager(QWidget* parent, ColorHandler colorHandler) {
    this->parent = parent;
    this->colorHandler = colorHandler;
    std::vector<Node*> nodes = vector<Node*>(); //This vector should be managed by a Node Manager class
    this->currentColumn = 0;
}

int processCnt = 0;

void NodeManager::processJson(QVariantMap jsonMap, int level, Node* nodeParent) {
    cout << "PROCESS COUNT: " << processCnt++ << endl;
    cout << "NODELIST LENGTH: " << std::to_string(nodes.size()) << endl;
    QList<QString> keys = jsonMap.keys();

    bool firstKid = true;

    for (QString key:keys) {

        if (jsonMap[key].canConvert<QVariantMap>()) {

            Node *newNode = new Node(parent, level, nodeParent);
            newNode->row = level;
            /*
            newNode->row = level;
            if (!firstKid) {
                cout << "\nFIRSTKID ALTERT!!!!!" << endl;
                this->currentColumn++;
                cout << "NEWWW CURRENTCOLUYMN ::: " + std::to_string(currentColumn) << endl;
            } else {
                cout << "1:FIRSTKID ISSSS::: " + std::to_string(firstKid) << endl;
                firstKid = false;
                cout << "FIRSTKID ISSSS::: " + std::to_string(firstKid) << endl;
            }
            newNode->column = this->currentColumn;
*/

            newNode->setName(nodes.size());
            newNode->header->setText(key + " :: " + QString::fromStdString(std::to_string(newNode->getName())));
            if (nodeParent != NULL)
                newNode->bottomBar->setText(key + " :: " + QString::fromStdString(std::to_string(nodeParent->getName())));
            newNode->adjustSize();
            nodes.push_back(newNode);

            cout << "MAP##" << level << endl;
            processJson(jsonMap[key].toMap(), level + 1, newNode);
        } else if (jsonMap[key] == jsonMap[key].toString()) {
//            newNode->bottomBar->setText(jsonMap[key].toString());
//            newNode->bottomBar->adjustSize();
//            newNode->adjustSize();
//            newNode->bottomBar->updateGeometry();
//            newNode->updateGeometry();
        } else {
            cout << "THE KEY " << key.toStdString() << " CORRESPONDS TOA  LIST BTW" << endl;
            Node *listNode = new Node(parent, level, nodeParent);
            listNode->row = level;
            listNode->adjustSize();
            listNode->setName(nodes.size());
            QString name = QString::fromStdString(std::to_string(listNode->getName()));
            listNode->header->setText(key + " :: " + QString::fromStdString(std::to_string(listNode->getName())));
            listNode->bottomBar->setText(QString::fromStdString(std::to_string(nodeParent->getName())));
            nodes.push_back(listNode);

            QList<QVariant> jsonList = (jsonMap[key].toList());

            for (QVariant child:(jsonMap[key].toList())) {

                QVariantMap mapChild = child.toMap();
                for (QString childKey:mapChild.keys()) {
                    cout << "\nHunting for a string in a dict if you cared..." << endl;
                    cout << "ChildKey: " << childKey.toStdString() << endl;
                    cout << "ChildValue" << mapChild[childKey].toString().toStdString() << endl;
                    if (mapChild[childKey] == mapChild[childKey].toString()) {
                        cout << "GOT one... if you cared..." << endl;
                        Node *newNode = new Node(parent, level, listNode);
                        newNode->setName(nodes.size());
                        newNode->row = level + 1;
                        newNode->header->setText(childKey + " :: " + QString::fromStdString(std::to_string(newNode->getName())));
                        newNode->adjustSize();
                        nodes.push_back(newNode);

                        newNode->bottomBar->setText(mapChild[childKey].toString() + " :: " + name);
                        newNode->bottomBar->adjustSize();
                        newNode->adjustSize();
                        newNode->bottomBar->updateGeometry();
                        newNode->updateGeometry();
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
}

std::vector<Node*> NodeManager::getNodes() {
    return nodes;
}
