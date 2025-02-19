#include "Node.h"

Node::Node(QWidget *parent, const char *name)
    : QLineEdit(parent)
{
    this->parent;
    this->name = "";
    this->key = "";
    this->value = "";
    this->children = std::vector<Node>();
    this->widget = new QTextEdit();
    widget->setText(name);
}

Node* Node::getParent() {
    return parent;
}

void Node::setParent(Node *newParent) {
    parent = newParent;
}

QString Node::getName() {
    return name;
}

void Node::setName(QString newName) {
    name = newName;
}

QString Node::getKey() {
    return key;
}

void Node::setKey(QString newKey) {
    key = newKey;
}

QString Node::getValue() {
    return value;
}

void Node::setValue(QString newValue) {
    value = newValue;
}

void Node::addChild(Node newChild) {
//    children.push_back(newChild);
}

void Node::removeChild(Node oldChild) {
//    int index = 0;
//    while (!children[index].equals(oldChild)) {
//        index++;
//    }
//    removeChild(index);
}


void Node::removeChild(int index) {
//    children.erase(children.begin() + index);
}

bool Node::equals(Node Node2) {
    if (parent == Node2.parent && name == Node2.name &&
        key == Node2.key && value == Node2.value) {
        return true;
    }
    return false;
}
