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
    if (!dbManager.openDatabase("metadata.db")) {
        qDebug() << "Could not open database!";
    }

    // Create the table BEFORE inserting data
    QSqlQuery query(dbManager.database());
    if (!query.exec("CREATE TABLE IF NOT EXISTS schema_fields ("
                    "id INTEGER PRIMARY KEY, "
                    "parent_id INTEGER, "
                    "name TEXT, "
                    "FOREIGN KEY(parent_id) REFERENCES schema_fields(id) ON DELETE CASCADE)")) {
        qDebug() << "Error creating schema table:" << query.lastError().text();
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

    // Caleb's Code: Create text input, autocomplete and export buttons
    createTextInputIfNeeded();
    setupAutocomplete();
    createExportButtons();      // <-- New: programmatically create export buttons
    setupConnections();

    // Color and Node manager setup
    ColorHandler *colorHandler = new ColorHandler();
    this->nodeManager = new NodeManager(this->ui->nodeHolder, *colorHandler);
    this->setPalette(colorHandler->getPalette());
    this->ui->menubar->setPalette(this->palette());
}

// New method to create export buttons programmatically
void MainWindow::createExportButtons() {
    // Create export buttons and set object names for later connection
    QPushButton* exportJsonButton = new QPushButton("Export JSON", this);
    exportJsonButton->setObjectName("exportJsonButton");
    QPushButton* exportXmlButton = new QPushButton("Export XML", this);
    exportXmlButton->setObjectName("exportXmlButton");
    QPushButton* exportGamlButton = new QPushButton("Export GAML", this);
    exportGamlButton->setObjectName("exportGamlButton");

    // Add these buttons to the central widget layout.
    QVBoxLayout* layout = nullptr;
    if (ui->centralwidget->layout()) {
        layout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    } else {
        layout = new QVBoxLayout(ui->centralwidget);
        layout->addWidget(ui->nodeHolder); // Retain existing widget
    }
    layout->addWidget(exportJsonButton);
    layout->addWidget(exportXmlButton);
    layout->addWidget(exportGamlButton);
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
    m_textInput = findChild<QLineEdit*>("textInput");
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

// General UI connections
void MainWindow::setupConnections()
{
    // Connect JSON load buttons or actions
    if (auto button = findChild<QPushButton*>("openButton")) {
        connect(button, &QPushButton::clicked, this, &MainWindow::loadJsonButtonClicked);
    }
    else if (auto action = findChild<QAction*>("actionOpen")) {
        connect(action, &QAction::triggered, this, &MainWindow::loadJsonButtonClicked);
    }
    else {
        qWarning() << "No JSON load trigger found in UI";
    }

    // Connect our export buttons to their slots
    if (auto exportJsonButton = findChild<QPushButton*>("exportJsonButton")) {
        connect(exportJsonButton, &QPushButton::clicked,
                this, &MainWindow::on_actionExport_JSON_triggered);
    }
    if (auto exportXmlButton = findChild<QPushButton*>("exportXmlButton")) {
        connect(exportXmlButton, &QPushButton::clicked,
                this, &MainWindow::on_actionExport_XML_triggered);
    }
    if (auto exportGamlButton = findChild<QPushButton*>("exportGamlButton")) {
        connect(exportGamlButton, &QPushButton::clicked,
                this, &MainWindow::on_actionExport_GAML_triggered);
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

    // First process the JSON to create nodes
    nodeManager->processJson(jsonMap, 0);
    this->ui->nodeHolder->update();

    // Now insert the JSON structure into the database
    DatabaseManager& dbManager = DatabaseManager::instance();

    // Clear existing schema data
    QSqlQuery clearQuery(dbManager.database());
    if (!clearQuery.exec("DELETE FROM schema_fields")) {
        qDebug() << "Error clearing schema table:" << clearQuery.lastError().text();
    }

    // Recursive function to insert data into the database
    // Adjust this function in your recursive parser
    // Recursive function to insert data into the database
    std::function<void(const QVariant&, const QString&, int)> insertJsonToDb;
    insertJsonToDb = [&](const QVariant& data, const QString& key, int parentId) {
        if (data.type() == QVariant::Map) {
            // Convert to a QVariantMap
            QVariantMap map = data.toMap();
            int currentId = parentId;

            // If there is a key, insert it as a node regardless of its value
            if (!key.isEmpty()) {
                currentId = dbManager.insertSchemaField(parentId, key);
                if (currentId == -1) return;
            }

            // To preserve order as best as possible, iterate using iterator (not keys()).
            // Note: QJsonObject parsing may preserve the order as in the original JSON text.
            for (auto it = map.begin(); it != map.end(); ++it) {
                insertJsonToDb(it.value(), it.key(), currentId);
            }
        }
        else if (data.type() == QVariant::List) {
            QVariantList list = data.toList();

            int arrayParentId = parentId;
            if (!key.isEmpty()) {
                arrayParentId = dbManager.insertSchemaField(parentId, key + "[]");
                if (arrayParentId == -1) return;
            }

            for (const auto& item : list) {
                if (item.type() == QVariant::Map || item.type() == QVariant::List) {
                    int elementId = dbManager.insertSchemaField(arrayParentId, "element");
                    insertJsonToDb(item, "", elementId);
                } else {
                    // Ensure primitive is stored with quotes as string
                    QString value = item.toString();
                    QString quoted = "\"" + value + "\"";
                    QString name = "value: " + quoted;
                    dbManager.insertSchemaField(arrayParentId, name);
                }
            }
        }
        else {
            QString value = data.toString();
            QString quoted = "\"" + value + "\"";
            QString name = key.isEmpty() ? quoted : key + ": " + quoted;
            dbManager.insertSchemaField(parentId, name);
        }
    };



    // Initialize recursive insert for the root of the JSON structure
    insertJsonToDb(jsonMap, "", 0);


    // Print the database contents for verification
    dbManager.printSchemaTable();

    // Refresh suggestions with the new data
    m_suggestionManager->refreshDatabase();

    int currentColumn = 0;
    int maxParent = -1;
    qDebug() << "\n\n Node Time \n";
    qDebug() << "Here's our Stats, Boss:";
    qDebug() << "Columns:" << this->ui->gridLayout->columnCount();
    qDebug() << "Rows:" << this->ui->gridLayout->rowCount();
    qDebug() << "Size of this window:" << this->ui->scrollArea_2->geometry().width()
             << "x" << this->ui->nodeHolder->geometry().height();

    for (Node* node : nodeManager->getNodes()) {
        qDebug() << "currentColumn :" << currentColumn;
        qDebug() << "maxParent :" << maxParent;

        if (node->getNodeParent() == nullptr) {
            qDebug() << "\n" << node->header->text() << ": ROOT ::"
                     << node->row << ":" << currentColumn;
        } else {
            qDebug() << "\n" << node->header->text() << ":"
                     << QString::number(node->getNodeParent()->getName())
                     << " :: " << node->row << ":" << currentColumn;
        }

        if (node->getNodeParent() != nullptr) {
            if (node->getNodeParent()->getName() > maxParent) {
                maxParent = node->getNodeParent()->getName();
            } else if (node->getNodeParent()->getName() <= maxParent) {
                currentColumn++;
            }
        }

        // Adding widget to grid layout (adjust for your use case)
        if (currentColumn == -1)
            this->ui->gridLayout->addWidget(node, node->row, 0);
        else
            this->ui->gridLayout->addWidget(node, node->row, currentColumn);
    }
}

void MainWindow::on_actionExport_JSON_triggered() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export as JSON"), "",
                                                    tr("JSON Files (*.json)"));

    if (!fileName.isEmpty()) {
        DatabaseManager& dbManager = DatabaseManager::instance();
        if (dbManager.exportToJson(fileName)) {
            QMessageBox::information(this, "Export Successful",
                                     "Database exported to JSON successfully.");
        } else {
            QMessageBox::warning(this, "Export Failed",
                                 "Failed to export database to JSON.");
        }
    }
}

void MainWindow::on_actionExport_XML_triggered() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export as XML"), "",
                                                    tr("XML Files (*.xml)"));

    if (!fileName.isEmpty()) {
        DatabaseManager& dbManager = DatabaseManager::instance();
        if (dbManager.exportToXml(fileName)) {
            QMessageBox::information(this, "Export Successful",
                                     "Database exported to XML successfully.");
        } else {
            QMessageBox::warning(this, "Export Failed",
                                 "Failed to export database to XML.");
        }
    }
}

void MainWindow::on_actionExport_GAML_triggered() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export as GAML"), "",
                                                    tr("GAML Files (*.gaml)"));

    if (!fileName.isEmpty()) {
        DatabaseManager& dbManager = DatabaseManager::instance();
        if (dbManager.exportToGaml(fileName)) {
            QMessageBox::information(this, "Export Successful",
                                     "Database exported to GAML successfully.");
        } else {
            QMessageBox::warning(this, "Export Failed",
                                 "Failed to export database to GAML.");
        }
    }
}

// Optional method for handling the load schema action
void MainWindow::on_actionLoad_Schema_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Schema File"),
                                                    "",
                                                    tr("Schema Files (*.sma)"));
    if (!fileName.isEmpty())
        this->schemaHandler->addSchema(fileName.toStdString());
}

// Stub for additional export actions if needed
void MainWindow::on_actionExport_as_triggered() {
    // Additional export functionality can be implemented here
}

Ui::MainWindow* MainWindow::getUi() {
    return this->ui;
}

MainWindow::~MainWindow()
{
    m_suggestionManager->cancelPendingRequests();

    // Close and delete the database file
    DatabaseManager& dbManager = DatabaseManager::instance();
    QString dbFilePath = dbManager.database().databaseName();
    dbManager.closeDatabase(); // Make sure the DB is properly closed

    // Attempt to remove the database file
    QFile dbFile(dbFilePath);
    if (dbFile.exists()) {
        if (!dbFile.remove()) {
            qWarning() << "Failed to delete database file:" << dbFile.errorString();
        } else {
            qDebug() << "Database file deleted successfully.";
        }
    }

    delete m_suggestionManager;
    delete ui;
}
