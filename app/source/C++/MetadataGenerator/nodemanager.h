#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <queue>
#include "node.h"
#include "colorhandler.h"
#include "schema.h"
//#include "mainwindow.h"

using namespace std;
#include <QWidget>
#include <vector>
#include <QVariantMap>

class NodeManager
{
public:
    // Constructor
    NodeManager(QWidget* parent, ColorHandler colorHandler);

    // Methods
    void processJson(QVariantMap jsonMap, int level, Node* nodeParent = nullptr);
    QString fillValue(QVariantMap jsonMap, std::queue<std::string> fieldName);
    Node createNode(int level,  Node nodeParent = NULL);
    Node* createNode(int level, Node* nodeParent = nullptr);  // Using pointer for consistency
    void addNode(Node* node);
    std::vector<Node*> getNodes() const;  // Marking as const since it's a getter

    // Member variables
    QWidget* parent;
    ColorHandler colorHandler;
    int currentColumn;
    std::vector<Node*> getNodes();
    QSize nodeSize;
    double fontSize;
    void alterNodeSize(double factor);
    void nodesFromSchema(Schema* schema, bool bottombars = true, QVariantMap* jsonMap = nullptr);
    void recCreateNodes(std::shared_ptr<Field> root, int level,
                        Node* nodeParent = nullptr, bool bottombars = true, std::queue<std::string> fullName = std::queue<std::string>());
    void emptyNodeList();

private:
    // Internal node container
    std::vector<Node*> nodes;
};

#endif // NODEMANAGER_H
