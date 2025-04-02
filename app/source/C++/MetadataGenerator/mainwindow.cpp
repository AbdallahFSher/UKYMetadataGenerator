#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtWidgets>
#include <iostream>
#include "node.h"
#include "colorhandler.h"
#include "suggestionmanager.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_suggestionManager(new SuggestionManager(this)),
    m_completer(new QCompleter(this)),
    m_textInput(nullptr)
{
    std::vector<Node*> nodes = vector<Node*>();
    ui->setupUi(this);

    Node *beans = new Node(this->ui->nodeHolder, 0);
    Node *beans2 = new Node(this->ui->nodeHolder, 1);
    nodes.push_back(beans);
    nodes.push_back(beans2);
    beans->setText("One");
    beans->adjustSize();
    beans2->setText("Two");
    beans->move(200, 200);
    createTextInputIfNeeded();
    setupAutocomplete();
    setupConnections();

    // Initialize nodes
    Node *node1 = new Node(ui->nodeHolder, "node1");
    Node *node2 = new Node(ui->nodeHolder, "node2");
    node1->setText("One");
    node1->adjustSize();
    node2->setText("Two");
    node1->move(200, 200);

    ColorHandler *colorHandler = new ColorHandler();

    this->setPalette(colorHandler->getPalette());
    colorHandler->setColors(nodes);

    //layout->addWidget(beans);
    //layout->addWidget(beans2);
}

void MainWindow::handleTextInputChanged(const QString& text)
{
    m_suggestionManager->requestSuggestions(text);
}

void MainWindow::updateSuggestions(const QStringList& suggestions)
{
    QStringListModel* model = qobject_cast<QStringListModel*>(m_completer->model());
    if (model) {
        model->setStringList(suggestions);
    } else {
        m_completer->setModel(new QStringListModel(suggestions, m_completer));
    }
}

void MainWindow::setupConnections()
{
    // Connect to a button if it exists
    if (auto button = findChild<QPushButton*>("openButton")) {
        connect(button, &QPushButton::clicked, this, &MainWindow::loadJsonButtonClicked);
    }
    // Or connect to a menu action if it exists
    else if (auto action = findChild<QAction*>("actionOpen")) {
        connect(action, &QAction::triggered, this, &MainWindow::loadJsonButtonClicked);
    }
    else {
        qWarning() << "No JSON load trigger found in UI";
    }
}

void MainWindow::loadJsonButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open JSON File"),
                                                    "",
                                                    tr("JSON Files (*.json)"));

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening file:" << file.errorString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        qDebug() << "Invalid JSON document";
        return;
    }

    ui->jsonLabel->setText(jsonDoc.toJson(QJsonDocument::Indented));
    m_suggestionManager->refreshDatabase();
}

void MainWindow::createTextInputIfNeeded()
{
    m_textInput = findChild<QLineEdit*>();
    if (!m_textInput) {
        m_textInput = new QLineEdit(this);
        m_textInput->setObjectName("textInput");
        if (ui->centralwidget->layout()) {
            ui->centralwidget->layout()->addWidget(m_textInput);
        } else {
            QVBoxLayout* layout = new QVBoxLayout(ui->centralwidget);
            layout->addWidget(m_textInput);
            layout->addWidget(ui->nodeHolder);
        }
    }
}

void MainWindow::setupAutocomplete()
{
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_textInput->setCompleter(m_completer);

    connect(m_textInput, &QLineEdit::textEdited,
            this, &MainWindow::handleTextInputChanged);
    connect(m_suggestionManager, &SuggestionManager::suggestionsReady,
            this, &MainWindow::updateSuggestions);

    m_suggestionManager->initialize();
}

MainWindow::~MainWindow()
{
    m_suggestionManager->cancelPendingRequests();
    delete m_suggestionManager;
    delete ui;
}
