#include "addnodedialogue.h"
#include "ui_addnodedialogue.h"

AddNodeDialogue::AddNodeDialogue(QWidget *parent, SuggestionManager* suggestionManager)
    : QDialog(parent)
    , ui(new Ui::AddNodeDialogue)
{
    ui->setupUi(this);
    this->suggestionManager = suggestionManager;
    this->ignoreSignals = true;
    this->spd = new SelectParentDialogue();
}

AddNodeDialogue::~AddNodeDialogue()
{
    delete ui;
}

void AddNodeDialogue::setParent(Node* parent) {
    if (!parent || parent->getDatabaseId() < 0) {
        qWarning() << "Invalid parent node or missing database ID";
        return;
    }

    if (!this->ignoreSignals) {
        this->newNode = new Node(parent->parentWidget(), parent->row + 1, parent);
        this->newNode->setKey(ui->keyEdit->text());
        this->newNode->setValue(ui->valueEdit->text());
        this->newNode->row = parent->row + 1;
        this->newNode->column = parent->column;

        int parentDbId = parent->getDatabaseId();
        // Handle root node case if necessary, e.g., parentDbId = 0 might map to NULL in the database
        qDebug() << "Using parent database ID:" << parentDbId;

        spd->hide();
        ignoreSignals = true;
        emit createNode(newNode, parentDbId);
    }
}

void AddNodeDialogue::on_buttonBox_accepted()
{
    this->ignoreSignals = false;
    this->spd->show();
    this->hide();
}

void AddNodeDialogue::setupAutocomplete(QCompleter* completer) {
    this->ui->keyEdit->setCompleter(completer);
    this->ui->valueEdit->setCompleter(completer);
    connect(this->ui->keyEdit, &QLineEdit::textEdited,
            this, &AddNodeDialogue::handleTextInputChanged);
    connect(this->ui->valueEdit, &QLineEdit::textEdited,
            this, &AddNodeDialogue::handleTextInputChanged);
}

void AddNodeDialogue::handleTextInputChanged(const QString& text)
{
    suggestionManager->requestSuggestions(text);
}
