#ifndef NODE_H
#define NODE_H

#include <QtWidgets>
#include <QLineEdit>
#include <vector>
#include <map>

class Node : public QTextEdit
{
    Q_OBJECT

public:
    Node(QWidget *parent = nullptr, const int nodeVariant = 0);
    Node();
    ~Node();

    // Toggle functionality
    void toggleMinimized();
    bool isMinimized() const { return m_minimized; }
    void updateChildrenVisibility();

    // Node properties
    Node *parent;
    QString name;
    QString key;
    QString value;
    std::vector<Node*> children;
    int nodeVariant;
    std::map<QString, QString> content;

    // Member functions
    Node* getParent();
    void setParent(Node *newParent);
    QString getName();
    void setName(QString newName);
    QString getKey();
    void setKey(QString newKey);
    QString getValue();
    void setValue(QString newValue);
    void addChild(Node* newChild);
    void removeChild(Node* oldChild);
    void removeChild(int index);
    bool equals(const Node* other) const;

    // Event overrides
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void toggled(Node* node);

private:
    QLineEdit *lineEdit;
    bool m_minimized = false;
    QPoint m_dragStartPos;
    QSpacerItem *spacer;

    void updateAppearance();
    QString formatContent() const;
};
#endif // NODE_H
