#include "selectparentdialogue.h"
#include "ui_selectparentdialogue.h"

SelectParentDialogue::SelectParentDialogue(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectParentDialogue)
{
    ui->setupUi(this);
}

SelectParentDialogue::~SelectParentDialogue()
{
    delete ui;
}
