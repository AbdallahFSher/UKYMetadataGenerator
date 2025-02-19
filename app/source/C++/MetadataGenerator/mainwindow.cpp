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
