#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtWidgets>
#include <iostream>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::loadJsonButtonClicked(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open JSON File"), "", tr("JSON Files (*.json)"));
    if (fileName != "") {
        cout << "File Found" << endl;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);

    QStringList fields;
    while(!in.atEnd()) {
        QString line = in.readLine();
        fields.append(line);
    }
    file.close();
    ui->jsonLabel->setText(fields.join("\n"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
