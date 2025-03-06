#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtWidgets>
#include <iostream>
#include "node.h"
#include "colorhandler.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QLayout *layout = this->ui->nodeHolderr;

    Node *beans = new Node(this->ui->nodeHolder, "bomb");
    Node *beans2 = new Node(this->ui->nodeHolder, "bones");
    beans->setText("One");
    beans->adjustSize();
    beans2->setText("Two");
    beans->move(200, 200);

    ColorHandler *colorHandler = new ColorHandler();

    this->setPalette(colorHandler->getPalette());

    //layout->addWidget(beans);
    //layout->addWidget(beans2);
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
    QByteArray byteArray;
    byteArray = QJsonDocument(jsonDoc).toJson();

    file.close();
    ui->jsonLabel->setText(byteArray);
}

MainWindow::~MainWindow()
{
    delete ui;
}
