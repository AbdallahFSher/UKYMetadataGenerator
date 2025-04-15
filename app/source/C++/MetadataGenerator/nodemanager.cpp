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

    for (const QString& key : keys) {
        if (jsonMap[key].canConvert<QVariantMap>()) {
            Node* newNode = new Node(parent, level, nodeParent);
            newNode->row = level;
            newNode->setName(nodes.size());
            newNode->header->setText(key);
            if (nodeParent != nullptr)
                newNode->bottomBar->setText(key);

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
        if(nodeParent)
            nodeParent->children.push_back(newNode);
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
