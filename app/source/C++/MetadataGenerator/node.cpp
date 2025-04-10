#include "Node.h"
#include <iostream>

using namespace std;

// TODO: Give information to schema which delineates nodeVariant (color)
Node::Node(QWidget *parent, const int nodeVariant, Node* nodeParent)
    : QFrame(parent)
{
    this->nodeParent = nodeParent;
    this->nodeVariant = nodeVariant;
    this->content = map<QString, QString>(); // dictionary
    this->children = std::vector<Node>();
    this->spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

    this->header = new QLineEdit(this);
    this->header->setVisible(true);
    this->header->setText("head");

    this->bottomBar = new QLineEdit(this);
    this->bottomBar->setText("tail");
    this->bottomBar->setVisible(true);

    this->setLayout(new QVBoxLayout());
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->header->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->bottomBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->updateGeometry();
    this->header->updateGeometry();
    this->bottomBar->updateGeometry();

    this->layout()->addWidget(header);
    this->layout()->addWidget(bottomBar);
    this->setMinimumSize(100, 80);

    this->setVisible(true);
}

//Node::Node() {
//    this->nodeParent;
//    this->nodeVariant = 0;
//    this->content = map<QString, QString>(); // dictionary
//    this->children = std::vector<Node>();
//    this->spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
//}

Node* Node::getNodeParent() {
    return nodeParent;
}

void Node::setNodeParent(Node *newNodeParent) {
    nodeParent = newNodeParent;
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
    if (nodeParent == Node2.nodeParent && name == Node2.name &&
        key == Node2.key && value == Node2.value) {
        return true;
    }
    return false;
}

void Node::mousePressEvent(QMouseEvent *event)
{
    //cout << "MOUSE CLICKED" << toPlainText().toStdString() << endl;
    if (event->button() == Qt::LeftButton) {

        QPoint startPos = mapFromGlobal(QCursor::pos());


        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        QImage image(QSize(400,300),QImage::Format_RGB32);
        QPainter painter(&image);
        painter.setBrush(QBrush(Qt::green));
        painter.fillRect(QRectF(0,0,400,300),Qt::green);
        painter.fillRect(QRectF(100,100,200,100),Qt::white);
        painter.setPen(QPen(Qt::black));
        painter.drawText(QRect(100,100,200,100),"Text you want to draw...");

        //QPixmap pixmap = QPixmap(this->size());
        //this->render(pixmap);
        //QRect rect = this->rect();
        //QVariant var = QVariant(rect);
        //mimeData->setImageData(image);

        mimeData->setText("");
        drag->setMimeData(mimeData);
        //QImage img = var.value<QImage>();
        //drag->setPixmap(pixmap);
        //drag->setPixmap(QPixmap::fromImage(image));

        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        QPoint endPos = mapFromGlobal(QCursor::pos());
        QPoint diff = endPos - startPos;
        //QPoint diff = endPos;

        //cout << "INITALPOS: " << (QString("%1x%2").arg(this->pos().x()).arg(this->pos().y())).toStdString() << endl;
        this->move(this->pos() + diff);
        //cout << "ENDPOS: " << (QString("%1x%2").arg(this->pos().x()).arg(this->pos().y())).toStdString() << endl;
        //cout << "START MOUSE POS: " << (QString("%1x%2").arg(startPos.x()).arg(startPos.y())).toStdString() << endl;
        //cout << "END MOUSE POS: " << (QString("%1x%2").arg(endPos.x()).arg(endPos.y())).toStdString() << endl;
        //cout << "DIFF: " << (QString("%1x%2").arg(diff.x()).arg(diff.y())).toStdString() << endl;

        //cout << drag->pixmap().height() << "x" << drag->pixmap().width() << endl;
    }
}
