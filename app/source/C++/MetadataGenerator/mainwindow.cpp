#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtWidgets>
#include <QSqlQuery>
#include <QSqlError>
#include "node.h"
#include "colorhandler.h"
#include "suggestionmanager.h"
#include "fileparser.h"
#include "nodemanager.h"


using namespace std;

void insertFieldTree(const std::shared_ptr<Field>& node, int parentId, DatabaseManager& dbManager) {
    int currentId = dbManager.insertSchemaField(parentId, QString::fromStdString(node->name));
    if (currentId == -1) return;
    for (const auto& child : node->children) {
        insertFieldTree(child, currentId, dbManager);
    }
}

// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_suggestionManager(new SuggestionManager(this)),
    m_completer(new QCompleter(this)),
    m_textInput(nullptr),
    fileParser(new FileParser()),
    schemaHandler(new SchemaHandler())
{

    ui->setupUi(this);

    // Open the database FIRST
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.openDatabase("example.db")) {
        qDebug() << "Could not open database!";
    }

    // Create the table BEFORE inserting data
    QSqlQuery query(dbManager.database());
    if (!query.exec("CREATE TABLE IF NOT EXISTS schema_fields ("
                    "id INTEGER PRIMARY KEY, "
                    "parent_id INTEGER, "
                    "name TEXT, "
                    "FOREIGN KEY(parent_id) REFERENCES schema_fields(id))")) {
        qDebug() << "Error creating schema table:" << query.lastError().text();
    }

    // Then handle the schema
    Schema* currentSchema = schemaHandler->addSchema("..\\..\\..\\examples\\exampleSchema.sma");

    if (currentSchema) {
        auto rootField = currentSchema->getRoot();
        if (rootField) {
            insertFieldTree(rootField, 0, dbManager); // 0 indicates no parent
        }
    }

    // Print the table contents
    qDebug() << "\nDatabase Contents:";
    if (!query.exec("SELECT id, parent_id, name FROM schema_fields ORDER BY id")) {
        qDebug() << "Error reading schema table:" << query.lastError().text();
    } else {
        qDebug() << "ID\tParent\tName";
        qDebug() << "---------------------";
        while (query.next()) {
            qDebug() << query.value(0).toInt() << "\t"
                     << query.value(1).toInt() << "\t"
                     << query.value(2).toString();
        }
    }
/*
    // Calvin's Code
    Node *beans = new Node(this->ui->nodeHolder, 0);
    Node *beans2 = new Node(this->ui->nodeHolder, 1);
    nodes.push_back(beans);
    nodes.push_back(beans2);
    beans->setText("One");
    beans->adjustSize();
    beans2->setText("Two");
    beans->move(200, 200);
*/
    // Caleb's Code
    createTextInputIfNeeded();
    setupAutocomplete();
    setupConnections();
    ColorHandler *colorHandler = new ColorHandler();

    this->nodeManager = new NodeManager(this->ui->nodeHolder, *colorHandler);
    this->setPalette(colorHandler->getPalette());
}

// Suggestion Manager Functions
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

// General UI
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

    // Parse JSON data to QVariantMap
    QVariant jsonVariant = this->fileParser->importJson(fileName);
    QVariantMap jsonMap = jsonVariant.toMap();
    nodeManager->processJson(jsonMap, 0);
    this->ui->nodeHolder->update();
    //cout << "NODEHOLDER GOT THESE KIDS:: " << this->ui->nodeHolder->children.count() << endl;

    // Parse QVariantMap to construct schema
    //Schema* newSchema = this->schemaHandler->fromVariantMap(jsonMap);

    //if (newSchema) {
    //    newSchema->printTree(newSchema->getRoot(), 0);
    //}
    ui->jsonLabel->setText(this->fileParser->getCurrentJSON().toJson(QJsonDocument::Indented));
    m_suggestionManager->refreshDatabase();
}

Ui::MainWindow* MainWindow::getUi() {
    return this->ui;
}

MainWindow::~MainWindow()
{
    m_suggestionManager->cancelPendingRequests();
    delete m_suggestionManager;
    delete ui;
}
