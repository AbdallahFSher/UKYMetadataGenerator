#ifndef SELECTPARENTDIALOGUE_H
#define SELECTPARENTDIALOGUE_H

#include <QDialog>

namespace Ui {
class SelectParentDialogue;
}

class SelectParentDialogue : public QDialog
{
    Q_OBJECT

public:
    explicit SelectParentDialogue(QWidget *parent = nullptr);
    ~SelectParentDialogue();

private:
    Ui::SelectParentDialogue *ui;
};

#endif // SELECTPARENTDIALOGUE_H
