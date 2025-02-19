#ifndef NODE_H
#define NODE_H

#include <QtWidgets>
#include <qlineedit.h>



class Node : public QLineEdit
{
    Q_OBJECT

public:
    Node(QWidget *parent = nullptr, const char *name = nullptr);
    ~Node() {}

    Node *parent;
    QString name;
    QString key;
    QString value;
    std::vector<Node> children;
    QTextEdit *widget;

    Node* getParent();
    void setParent(Node *newParent);

    QString getName();
    void setName(QString newName);

    QString getKey();
    void setKey(QString newKey);

    QString getValue();
    void setValue(QString newValue);

    void addChild(Node newChild);
    void removeChild(Node oldChild);
    void removeChild(int index);

    bool equals(Node Node2);

private:
    QLineEdit *lineEdit;
};

#endif // NODE_H
