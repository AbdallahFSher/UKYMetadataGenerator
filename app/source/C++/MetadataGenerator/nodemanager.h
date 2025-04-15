#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <queue>
#include "node.h"
#include "colorhandler.h"
#include "schema.h"
//#include "mainwindow.h"

using namespace std;

class NodeManager
{
public:
    NodeManager(QWidget* parent, ColorHandler colorHandler);
    void processJson(QVariantMap jsonMap, int level, Node* nodeParent = nullptr);
    QString fillValue(QVariantMap jsonMap, std::queue<std::string> fieldName);
    Node createNode(int level,  Node nodeParent = NULL);
    void addNode(Node* node);
    QWidget* parent;
    ColorHandler colorHandler;
    int currentColumn;
    std::vector<Node*> getNodes();
    QSize nodeSize;
    void alterNodeSize(double factor);
    void nodesFromSchema(Schema* schema, bool bottombars = true, QVariantMap* jsonMap = nullptr);
    void recCreateNodes(std::shared_ptr<Field> root, int level,
                        Node* nodeParent = nullptr, bool bottombars = true, std::queue<std::string> fullName = std::queue<std::string>());
    void emptyNodeList();

private:
    std::vector<Node*> nodes;
};

#endif // NODEMANAGER_H
