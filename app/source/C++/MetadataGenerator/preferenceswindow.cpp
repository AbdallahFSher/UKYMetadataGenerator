#include "preferenceswindow.h"
#include "ui_preferenceswindow.h"

PreferencesWindow::PreferencesWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferencesWindow)
{
    ui->setupUi(this);
    this->colorScheme = 0;
}

PreferencesWindow::~PreferencesWindow()
{
    delete ui;
}
