#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtWidgets>
#include <iostream>
#include "node.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Node *beans = new Node(this, "bomb");
    Node *beans2 = new Node(this, "bones");
    beans->widget->setText("AHAAHAH");
    beans2->widget->setText("AHAAHAH");
}

void MainWindow::loadJsonButtonClicked(){


    QString fileName = QFileDialog::getOpenFileName(this, tr("Open JSON File"), "", tr("JSON Files (*.json)"));
    if (fileName != "") {
        cout << "File Found" << endl;
    }

    QFile file(fileName);
    QString jsonString;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        jsonString = file.readAll();
        file.close();
    }
    else
        qDebug()<< "file not found";

    auto jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonValue val;
    QByteArray byteArray;
    byteArray = QJsonDocument(jsonDoc).toJson();

    file.close();
    ui->jsonLabel->setText(byteArray);
}

MainWindow::~MainWindow()
{
    delete ui;
}
