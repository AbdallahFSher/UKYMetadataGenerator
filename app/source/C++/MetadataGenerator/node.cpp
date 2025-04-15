#include "Node.h"
#include <iostream>

using namespace std;

// TODO: Give information to schema which delineates nodeVariant (color)
Node::Node(QWidget *parent, const int nodeVariant, Node* nodeParent)
    : QFrame(parent)
{
    this->collapsed = false;
    this->nodeParent = nodeParent;
    this->nodeVariant = nodeVariant;
    this->content = map<QString, QString>(); // dictionary
    this->children = std::vector<Node*>();
    this->spacer = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->header = new QLineEdit(this);
    this->header->setVisible(true);
    this->header->setAlignment(Qt::AlignCenter);
    this->header->setStyleSheet("font-weight: bold");

    this->bottomBar = new QLineEdit(this);
    this->bottomBar->setVisible(true);

    this->setLayout(new QVBoxLayout());
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->header->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->bottomBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(header, SIGNAL(textChanged(QString)), this, SLOT(resize_to_text()));
    connect(bottomBar, SIGNAL(textChanged(QString)), this, SLOT(resize_to_text()));

    this->updateGeometry();
    this->header->updateGeometry();
    this->bottomBar->updateGeometry();

    this->layout()->addWidget(header);
    this->layout()->addWidget(bottomBar);
    this->setMinimumSize(100, 80);

    this->setVisible(true);
}

Node::Node() {
    this->collapsed = false;
    this->nodeParent = nullptr;
    this->nodeVariant = 0;
    this->content = map<QString, QString>(); // dictionary
    this->children = std::vector<Node*>();
    this->spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

    this->header = new QLineEdit(this);
    this->header->setVisible(true);
    this->header->setText("head");
    this->header->setAlignment(Qt::AlignCenter);
    this->header->setStyleSheet("font-weight: bold");

    this->bottomBar = new QLineEdit(this);
    this->bottomBar->setVisible(true);

    this->setLayout(new QVBoxLayout());
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    this->header->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    this->bottomBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    connect(header, SIGNAL(textChanged(QString)), this, SLOT(resize_to_text()));
    connect(bottomBar, SIGNAL(textChanged(QString)), this, SLOT(resize_to_text()));

    this->updateGeometry();
    this->header->updateGeometry();
    this->bottomBar->updateGeometry();

    this->layout()->addWidget(header);
    this->layout()->addWidget(bottomBar);
    this->setMinimumSize(100, 80);
}

Node* Node::getNodeParent() {
    return nodeParent;
}

void Node::setNodeParent(Node *newNodeParent) {
    nodeParent = newNodeParent;
}

int Node::getName() {
    return name;
}

void Node::setName(int newName) {
    name = newName;
}

QString Node::getKey() {
    return key;
}

void Node::setKey(QString newKey) {
    key = newKey;
    this->header->setText(key);
}

QString Node::getValue() {
    return value;
}

void Node::setValue(QString newValue) {
    value = newValue;
    this->bottomBar->setText(value);
}

void Node::setValue(int value) {
    this->value = (QChar)value;
}

void Node::addChild(Node* newChild) {
    this->children.push_back(newChild);
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

void Node::mouseDoubleClickEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        hideNodes(this);
        collapsed = !collapsed;
    }
}

void Node::hideNodes(Node* root) {
    if(root->children.size() != 0) {
        for(int i = 0; i < root->children.size(); i++) {
            root->children[i]->setVisible(collapsed);
            hideNodes(root->children[i]);
            emit hidden(root->children[i]);
        }
    }
}

void Node::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && dragging) {
        //this->move(this->pos() + event->position().toPoint());
        //emit moved(this);
    }
}

void Node::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && dragging) {
        dragging = false;
        this->setCursor(Qt::ArrowCursor);
    }
}

void Node::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("application/nodeData")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void Node::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasFormat("application/nodeData")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void Node::mousePressEvent(QMouseEvent *event)
{
    //cout << "MOUSE CLICKED" << toPlainText().toStdString() << endl;
    if (event->button() == Qt::LeftButton) {
        emit this->beParent(this);

        this->setCursor(Qt::BlankCursor);
        dragging = true;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QByteArray nodeData;
        QDataStream dataStream(&nodeData, QIODevice::WriteOnly);
        dataStream << this->key << this->value << QPoint(event->position().toPoint() - this->pos());

        mimeData->setData("application/nodeData", nodeData);
        drag->setMimeData(mimeData);
        drag->setPixmap(this->grab());
        drag->setHotSpot(event->pos());

        if (drag->exec(Qt::MoveAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction) {

        } else {
            std::cout << "WE MOVIN" << std::endl;
            this->move(drag->hotSpot());
            emit moved(this);
        }
        /*
        QPoint startPos = mapFromGlobal(QCursor::pos());


        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setImageData(this->grab());
        drag->setMimeData(mimeData);
        drag->setPixmap(this->grab());

        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        QPoint endPos = mapFromGlobal(QCursor::pos());
        QPoint diff = endPos - startPos;

        this->move(this->pos() + diff);
        emit moved(this);
        */
    }
}


void Node::resize_to_text() {
    /*
    QString headerText = header->text();
    QString bottomBarText = bottomBar->text();
    QFont font("", 0);
    QFontMetrics fm(font);

    QRect headerRect = fm.boundingRect(headerText);
    QRect bottomRect = fm.boundingRect(bottomBarText);
    QRect newRect = this->frameRect();

    if (headerRect.width() > bottomRect.width()) {
        newRect.setWidth(headerRect.width());
    } else {
        newRect.setWidth(bottomRect.width());
    }

    this->setFrameRect(newRect);
    */
}
