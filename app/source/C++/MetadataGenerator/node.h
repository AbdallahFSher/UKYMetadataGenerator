#ifndef NODE_H
#define NODE_H

#include <QtWidgets>
#include <qlineedit.h>
#include <queue>

using namespace std;


class Node : public QFrame
{
    Q_OBJECT

public:
    Node(QWidget* parent, const int nodeVariant = 0, Node* nodeParent = nullptr);
    Node();

    QLineEdit* header;
    QLineEdit* bottomBar;

    Node *nodeParent;
    int name;
    QString key;
    QString value;
    std::queue<std::string> fullName;
    int row;
    int column;
    vector<Node> children;
    QTextEdit *widget;
    QSpacerItem *spacer;
    int nodeVariant;
    map<QString, QString> content;

    Node* getNodeParent();
    void setNodeParent(Node *newParent);

    int getName();
    void setName(int newName);

    QString getKey();
    void setKey(QString newKey);

    QString getValue();
    void setValue(QString newValue);

    void addChild(Node newChild);
    void removeChild(Node oldChild);
    void removeChild(int index);

    bool equals(Node Node2);
    void mousePressEvent(QMouseEvent *event);

public slots:
    void setValue(int value);
    void resize_to_text();

signals:
    void valueChanged(int newValue);
    void beParent(Node* parent);

private:
    QLineEdit *lineEdit;
};

#endif // NODE_H
