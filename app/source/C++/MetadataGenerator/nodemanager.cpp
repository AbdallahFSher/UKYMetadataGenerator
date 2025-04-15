#include "nodemanager.h"
#include "colorhandler.h"
#include "node.h"
#include <iostream>

using namespace std;

NodeManager::NodeManager(QWidget* parent, ColorHandler colorHandler) {
    this->parent = parent;
    this->colorHandler = colorHandler;
    std::vector<Node*> nodes = vector<Node*>(); //This vector should be managed by a Node Manager class
    std::vector<std::vector<Node*>> nodeBuffer = vector<std::vector<Node*>>();
    this->currentColumn = 0;
    this->nodeSize = QSize(100, 80);
}

void NodeManager::emptyNodeList() {
    this->nodes = vector<Node*>();
}

int processCnt = 0;

QString NodeManager::fillValue(QVariantMap jsonMap, std::queue<std::string> fieldName) {
    QString key;
    QVariantMap newMap;
    QString out = "null";
    if(!fieldName.empty()) {
        key = QString::fromStdString(fieldName.front());
        fieldName.pop();
        out = jsonMap[key].toString();
        newMap = jsonMap[key].toMap();
        std::cout << out.toStdString() << std::endl;
        if (out == jsonMap[key])
            return out;
        else if (jsonMap[key].canConvert<QVariantMap>())
            return fillValue(newMap, fieldName);
        else
            for (QVariant child : jsonMap[key].toList()) {
                newMap = child.toMap();
                return fillValue(newMap, fieldName);
            }
    }
    return out;
}

void NodeManager::processJson(QVariantMap jsonMap, int level, Node* nodeParent) {
    //cout << "PROCESS COUNT: " << processCnt++ << endl;
    //cout << "NODELIST LENGTH: " << std::to_string(nodes.size()) << endl;
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
            //newNode->header->setText(key + " :: " + QString::fromStdString(std::to_string(newNode->getName())));
            newNode->header->setText(key);
            if (nodeParent != NULL)
                //newNode->bottomBar->setText(key + " :: " + QString::fromStdString(std::to_string(nodeParent->getName())));
                newNode->bottomBar->setText(key);

            //newNode->adjustSize();
            nodes.push_back(newNode);

            //cout << "MAP##" << level << endl;
            processJson(jsonMap[key].toMap(), level + 1, newNode);
        } else if (jsonMap[key] == jsonMap[key].toString()) {
            Node* newNode = new Node(parent, level, nodeParent);
            newNode->row = level;
            newNode->setName(nodes.size());
            newNode->header->setText(key);
            newNode->bottomBar->setText(jsonMap[key].toString());
            nodes.push_back(newNode);
        } else {
            //cout << "THE KEY " << key.toStdString() << " CORRESPONDS TOA  LIST BTW" << endl;
            Node *listNode = new Node(parent, level, nodeParent);
            listNode->row = level;
            //listNode->adjustSize();
            listNode->setName(nodes.size());
            QString name = QString::fromStdString(std::to_string(listNode->getName()));
            //listNode->header->setText(key + " :: " + QString::fromStdString(std::to_string(listNode->getName())));
            listNode->header->setText(key);
            //listNode->bottomBar->setText(QString::fromStdString(std::to_string(nodeParent->getName())));
            listNode->bottomBar->setText("This is a list node");
            nodes.push_back(listNode);

            QList<QVariant> jsonList = (jsonMap[key].toList());

            for (QVariant child:(jsonMap[key].toList())) {

                QVariantMap mapChild = child.toMap();
                for (QString childKey:mapChild.keys()) {
                    //cout << "\nHunting for a string in a dict if you cared..." << endl;
                    //cout << "ChildKey: " << childKey.toStdString() << endl;
                    //cout << "ChildValue" << mapChild[childKey].toString().toStdString() << endl;
                    if (mapChild[childKey] == mapChild[childKey].toString()) {
                        //cout << "GOT one... if you cared..." << endl;
                        Node *newNode = new Node(parent, level, listNode);
                        newNode->setName(nodes.size());
                        newNode->row = level + 1;
                        //newNode->header->setText(childKey + " :: " + QString::fromStdString(std::to_string(newNode->getName())));
                        newNode->header->setText(childKey);
                        //newNode->adjustSize();
                        nodes.push_back(newNode);

                        //newNode->bottomBar->setText(mapChild[childKey].toString() + " :: " + name);
                        newNode->bottomBar->setText(mapChild[childKey].toString());
                        //newNode->bottomBar->adjustSize();
                        //newNode->adjustSize();
                        //newNode->bottomBar->updateGeometry();
                        //newNode->updateGeometry();
                        break;
                    }
                }
                processJson(mapChild, level + 1, listNode);
            }
        }
    }

    colorHandler.setColors(nodes);
}

void NodeManager::nodesFromSchema(Schema* schema, bool bottombars, QVariantMap* jsonMap) {
    recCreateNodes(schema->getRoot(), 0, nullptr, bottombars);
    if (jsonMap != nullptr) {
        for (Node* node : nodes) {
            node->setValue(fillValue(*jsonMap, node->fullName));
        }
    }
}

void NodeManager::recCreateNodes(std::shared_ptr<Field> root, int level, Node* nodeParent, bool bottombars, std::queue<std::string> fullName) {
    // Schemas will know what type each node will be (list, map, etc.)
    Node* newNode;
    for(int i = 0; i < root->count; i++) {
        newNode = new Node(parent, level, nodeParent); // Node variant can reflect the type from schema
        newNode->setKey(QString::fromStdString(root->name));
        fullName.push(root->name);
        newNode->row = level;
        newNode->setName(nodes.size());
        newNode->fullName = fullName;
        if (!bottombars) {
            newNode->bottomBar->setVisible(false);
        }
        addNode(newNode);
    }
    if (!root->children.empty()) {
        for (std::shared_ptr<Field>& child : root->children) {
            recCreateNodes(child, level + 1, newNode, bottombars, fullName);
        }
    }
}

void NodeManager::alterNodeSize(double factor) {
    nodeSize *= factor;
}

void NodeManager::addNode(Node* node) {
    nodes.push_back(node);
    colorHandler.setColors(nodes);
}

std::vector<Node*> NodeManager::getNodes() {
    return nodes;
}
