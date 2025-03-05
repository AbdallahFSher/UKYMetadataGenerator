#include "Node.h"
#include <iostream>

using namespace std;

Node::Node(QWidget *parent, const char *name)
    : QTextEdit(parent)
{
    this->parent;
    this->name = "";
    this->key = "";
    this->value = "";
    this->children = std::vector<Node>();
    this->spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
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

void Node::mousePressEvent(QMouseEvent *event)
{
    cout << "MOUSE CLICKED" << toPlainText().toStdString() << endl;
    if (event->button() == Qt::LeftButton) {

        QPoint startPos = mapFromGlobal(QCursor::pos());


        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText("");
        drag->setMimeData(mimeData);

        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        QPoint endPos = mapFromGlobal(QCursor::pos());
        QPoint diff = endPos - startPos;
        //QPoint diff = endPos;

        cout << "INITALPOS: " << (QString("%1x%2").arg(this->pos().x()).arg(this->pos().y())).toStdString() << endl;
        this->move(this->pos() + diff);
        cout << "ENDPOS: " << (QString("%1x%2").arg(this->pos().x()).arg(this->pos().y())).toStdString() << endl;
        cout << "START MOUSE POS: " << (QString("%1x%2").arg(startPos.x()).arg(startPos.y())).toStdString() << endl;
        cout << "END MOUSE POS: " << (QString("%1x%2").arg(endPos.x()).arg(endPos.y())).toStdString() << endl;
        cout << "DIFF: " << (QString("%1x%2").arg(diff.x()).arg(diff.y())).toStdString() << endl;

        cout << drag->pixmap().height() << "x" << drag->pixmap().width() << endl;
    }
}
