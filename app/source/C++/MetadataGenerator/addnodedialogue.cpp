#include "addnodedialogue.h"
#include "ui_addnodedialogue.h"

AddNodeDialogue::AddNodeDialogue(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddNodeDialogue)
{
    ui->setupUi(this);
    this->ignoreSignals = true;
    this->spd = new SelectParentDialogue();
}

AddNodeDialogue::~AddNodeDialogue()
{
    delete ui;
}

void AddNodeDialogue::setParent(Node* parent) {
    if (!this->ignoreSignals) {
        this->newNode = new Node(parent->parentWidget(), parent->row + 1, parent);
        this->newNode->setKey(this->ui->keyEdit->text());
        this->newNode->setValue(this->ui->valueEdit->text());
        this->newNode->row = parent->row + 1;
        this->newNode->column = parent->column;
        this->spd->hide();
        this->ignoreSignals = true;
        emit this->createNode(newNode);
    }
}

void AddNodeDialogue::on_buttonBox_accepted()
{
    this->ignoreSignals = false;
    this->spd->show();
    this->hide();
}
