#ifndef NODE_H
#define NODE_H

#include <QtWidgets>

class node
{
public:
    node();
    node *parent;
    QString name;
    QString key;
    QString value;
    std::vector<node> children;

    node* getParent();
    void setParent(node *newParent);

    QString getName();
    void setName(QString newName);

    QString getKey();
    void setKey(QString newKey);

    QString getValue();
    void setValue(QString newValue);

    void addChild(node newChild);
    void removeChild(node oldChild);
    void removeChild(int index);

    bool equals(node node2);
};

#endif // NODE_H
