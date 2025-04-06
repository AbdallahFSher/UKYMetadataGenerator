#include "Node.h"
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <iostream>

Node::Node(QWidget *parent, const int nodeVariant)
    : QTextEdit(parent), m_minimized(false), nodeVariant(nodeVariant), parent(nullptr),
    spacer(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding)),
    lineEdit(nullptr)
{
    setMouseTracking(true);
}

Node::Node() : Node(nullptr, 0) {} // Delegating constructor

void Node::addChild(Node* child) {
    children.push_back(child);
    child->setParent(this);
    child->setVisible(!m_minimized);
}

bool Node::equals(const Node* other) const {
    return (parent == other->parent &&
            name == other->name &&
            key == other->key &&
            value == other->value);
}

void Node::removeChild(Node* oldChild) {
    children.erase(std::remove(children.begin(), children.end(), oldChild), children.end());
}

void Node::removeChild(int index) {
    if(index >= 0 && index < static_cast<int>(children.size())) {
        children.erase(children.begin() + index);
    }
}

void Node::setParent(Node* newParent)
{
    // Remove from current parent's children list if needed
    if (this->parent && this->parent != newParent) {
        this->parent->removeChild(this);
    }

    // Set the new parent
    this->parent = newParent;

    // Add to new parent's children list if needed
    if (newParent && std::find(newParent->children.begin(),
                               newParent->children.end(),
                               this) == newParent->children.end()) {
        newParent->children.push_back(this);
    }

    // Update visibility based on parent's minimized state
    if (newParent) {
        this->setVisible(!newParent->isMinimized());
    }
}

// Fixed drag initialization
void Node::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
        event->accept();  // Ensure event is handled
        return;
    }
    QTextEdit::mousePressEvent(event);
}

// Fixed drag operation
void Node::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) return;

    if ((event->pos() - m_dragStartPos).manhattanLength()
        >= QApplication::startDragDistance()) {

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(this->toPlainText());

        // Create transparent drag image
        QPixmap pixmap(size());
        pixmap.fill(Qt::transparent);
        render(&pixmap);
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos() - rect().topLeft());

        drag->exec(Qt::MoveAction);
    }
}

// Fixed toggle handling
void Node::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if ((event->pos() - m_dragStartPos).manhattanLength()
            < QApplication::startDragDistance()) {
            toggleMinimized();
        }
    }
    QTextEdit::mouseReleaseEvent(event);
}

// Fixed content formatting
QString Node::formatContent() const {
    QStringList items;
    if (!name.isEmpty()) items << "Name: " + name;
    if (!key.isEmpty()) items << "Key: " + key;
    if (!value.isEmpty()) items << "Value: " + value;
    for (const auto& pair : content) {
        if (!pair.first.isEmpty() && !pair.second.isEmpty()) {
            items << pair.first + ": " + pair.second;
        }
    }
    return items.join("\n");
}

void Node::toggleMinimized()
{
    m_minimized = !m_minimized;
    updateAppearance();
    updateChildrenVisibility();
    emit toggled(this);
}

void Node::updateChildrenVisibility()
{
    for (Node* child : children) {
        child->setVisible(!m_minimized);
        child->updateChildrenVisibility();
    }
}

void Node::updateAppearance()
{
    if (m_minimized) {
        setMaximumHeight(30);
        setPlainText(name + " [minimized]");
    } else {
        setMaximumHeight(16777215); // Reset to default Qt maximum
        setPlainText(formatContent());
    }
}

Node::~Node()
{
    delete spacer;
    // Delete children if owned by this node
    for (auto child : children) {
        if (child->parent == this) {
            delete child;
        }
    }
}
