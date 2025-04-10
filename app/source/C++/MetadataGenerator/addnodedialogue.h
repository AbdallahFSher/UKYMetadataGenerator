#ifndef ADDNODEDIALOGUE_H
#define ADDNODEDIALOGUE_H

#include <QDialog>
#include "node.h"
#include "selectparentdialogue.h"

namespace Ui {
class AddNodeDialogue;
}

class AddNodeDialogue : public QDialog
{
    Q_OBJECT

public:
    explicit AddNodeDialogue(QWidget *parent = nullptr);
    ~AddNodeDialogue();

public slots:
    // Slot which connects to clicked() signal from Nodes in MainWindow
    void setParent(Node* parent);

signals:
    // Signal sent to MainWindow to add node to Node Manager
    void createNode(Node*);


private slots:
    void on_buttonBox_accepted();

private:
    Ui::AddNodeDialogue *ui;
    Node* newNode;
    bool ignoreSignals;
    SelectParentDialogue* spd;

};

#endif // ADDNODEDIALOGUE_H
