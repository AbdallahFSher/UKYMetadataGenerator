#include "node.h"

node::node() {
    this->parent;
    this->name = "";
    this->key = "";
    this->value = "";
    this->children = std::vector<node>();
}

node* node::getParent() {
    return parent;
}

void node::setParent(node *newParent) {
    parent = newParent;
}

QString node::getName() {
    return name;
}

void node::setName(QString newName) {
    name = newName;
}

QString node::getKey() {
    return key;
}

void node::setKey(QString newKey) {
    key = newKey;
}

QString node::getValue() {
    return value;
}

void node::setValue(QString newValue) {
    value = newValue;
}

void node::addChild(node newChild) {
    children.push_back(newChild);
}

void node::removeChild(node oldChild) {
    int index = 0;
    while (!node(children[index]).equals(oldChild)) {
        index++;
    }
    removeChild(index);
}

void node::removeChild(int index) {
    children.erase(children.begin() + index);
}

bool node::equals(node node2) {
    if (parent == node2.parent && name == node2.name &&
        key == node2.key && value == node2.value) {
        return true;
    }
    return false;
}
