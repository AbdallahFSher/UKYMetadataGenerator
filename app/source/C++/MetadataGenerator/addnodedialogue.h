#ifndef ADDNODEDIALOGUE_H
#define ADDNODEDIALOGUE_H

#include <QDialog>
#include "node.h"
#include "selectparentdialogue.h"
#include "suggestionmanager.h"

namespace Ui {
class AddNodeDialogue;
}

class AddNodeDialogue : public QDialog
{
    Q_OBJECT

public:
    explicit AddNodeDialogue(QWidget *parent = nullptr,
                             SuggestionManager* suggestionManager = nullptr);
    ~AddNodeDialogue();

    void setupAutocomplete(QCompleter* completer);

public slots:
    void setParent(Node* parent);

signals:
    // Updated signal with database parent ID
    void createNode(Node* node, int parentDatabaseId);

private slots:
    void on_buttonBox_accepted();
    void handleTextInputChanged(const QString& text);

private:
    Ui::AddNodeDialogue *ui;
    Node* newNode;
    bool ignoreSignals;
    SelectParentDialogue* spd;
    SuggestionManager* suggestionManager;

    // Parent tracking
    Node* currentParent = nullptr;
    int parentDatabaseId = 0;  // Default to root (0)
};

#endif // ADDNODEDIALOGUE_H
