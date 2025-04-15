#include "mainwindow.h"
#include "customscrollarea.h"
#include "GridLayoutUtil.h"
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
    int currentId = dbManager.insertSchemaField(parentId, QString::fromStdString(node->name), "field");
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
    schemaHandler(new SchemaHandler()),
    drawConnection(new DrawConnection())
{
    ui->setupUi(this);

    // Open the database FIRST
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.openDatabase("metadata.db")) {
        qDebug() << "Could not open database!";
    }

    // Create the table with type and correct foreign key relationship
    QSqlQuery query(dbManager.database());
    if (!query.exec("CREATE TABLE IF NOT EXISTS schema_fields ("
                    "id INTEGER PRIMARY KEY, "
                    "parent_id INTEGER, "
                    "name TEXT, "
                    "type TEXT, "  // Ensure the 'type' column is defined
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

    this->pw = new PreferencesWindow(this);
    this->addNodeDialogue = new AddNodeDialogue(this, m_suggestionManager);

    // Caleb's Code: Create text input, autocomplete and export buttons
    //createTextInputIfNeeded();
    setupConnections();

    // Color and Node manager setup
    this->colorHandler = new ColorHandler();
    this->nodeManager = new NodeManager(this->ui->nodeHolder, *colorHandler);
    this->setPalette(colorHandler->getPalette());
    this->ui->menubar->setPalette(this->palette());


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

    for(Node* n:this->nodeManager->getNodes()){
        n->header->setCompleter(m_completer);
        connect(n->header, &QLineEdit::textEdited,
                this, &MainWindow::handleTextInputChanged);
        n->bottomBar->setCompleter(m_completer);
        connect(n->bottomBar, &QLineEdit::textEdited,
                this, &MainWindow::handleTextInputChanged);
    }
    addNodeDialogue->setupAutocomplete(m_completer);
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

    connect(this->addNodeDialogue, SIGNAL(createNode(Node*)), this, SLOT(nodeAdded(Node*)));
}

void MainWindow::nodeAdded(Node* newNode) {
    newNode->setVisible(true);
    this->ui->gridLayout->addWidget(newNode, newNode->column, newNode->row);
    this->nodeManager->addNode(newNode);
    this->ui->nodeHolder->addWidgets(newNode->getNodeParent(), newNode);
    connect(newNode, SIGNAL(beParent(Node*)), this->addNodeDialogue, SLOT(setParent(Node*)));
    connect(newNode, SIGNAL(hidden(Node*)), this->ui->nodeHolder, SLOT(update()));
    connect(newNode, SIGNAL(moved(Node*)), this->ui->nodeHolder, SLOT(update()));
}

void MainWindow::clearNodeUI() {
    int rowCount = this->ui->gridLayout->rowCount();
    for(int row = 0; row < rowCount; row++) {
        GridLayoutUtil::removeRow(this->ui->gridLayout, row);
    }

    if (!nodeManager->getNodes().empty()) {
        nodeManager->emptyNodeList();
    }
}

void MainWindow::setupNodeUI() {
    int currentColumn = 0;
    int maxParent = -1;

    for (Node* node : nodeManager->getNodes()) {

        if (node->getNodeParent() != nullptr) {
            this->ui->nodeHolder->addWidgets(node->getNodeParent(), node);
            if (node->getNodeParent()->getName() > maxParent) {
                maxParent = node->getNodeParent()->getName();
            } else if (node->getNodeParent()->getName() <= maxParent) {
                currentColumn++;
            }
        }

        node->column = currentColumn;

        // Adding widget to grid layout (adjust for your use case)
        if (currentColumn == -1)
            this->ui->gridLayout->addWidget(node, 0, node->row);
        else
            this->ui->gridLayout->addWidget(node, currentColumn, node->row);

        connect(node, SIGNAL(beParent(Node*)), this->addNodeDialogue, SLOT(setParent(Node*)));
        connect(node, SIGNAL(hidden(Node*)), this->ui->nodeHolder, SLOT(update()));
        connect(node, SIGNAL(moved(Node*)), this->ui->nodeHolder, SLOT(update()));
        for (Node* node2 : nodeManager->getNodes()) {
            //connect(node, SIGNAL(moved(Node*)), node2, SLOT(hoveredOver(Node*)));
        }
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
    if (!jsonVariant.isValid()) {
        qWarning() << "Failed to parse JSON file.";
        return;
    }

    QVariantMap jsonMap = jsonVariant.toMap();

    // Clear the UI
    clearNodeUI();

    // Process JSON to a schema
    schemaHandler->fromVariantMap(jsonMap);

    // Fill Node fields with JSON values
    //nodeManager->processJson(jsonMap, 0);
    nodeManager->nodesFromSchema(schemaHandler->getCurrSchema(), true, &jsonMap);
    this->ui->nodeHolder->update();

    // Now insert the JSON structure into the database
    DatabaseManager& dbManager = DatabaseManager::instance();

    // Clear existing schema data
    QSqlQuery clearQuery(dbManager.database());
    if (!clearQuery.exec("DELETE FROM schema_fields")) {
        qDebug() << "Error clearing schema table:" << clearQuery.lastError().text();
    }

    // Recursive function to insert data into the database
    // In MainWindow::loadJsonButtonClicked()
    std::function<void(const QVariant&, const QString&, int)> insertJsonToDb;
    insertJsonToDb = [&](const QVariant& data, const QString& key, int parentId) {
        DatabaseManager& dbManager = DatabaseManager::instance();

        if (data.type() == QVariant::Map) {
            QVariantMap map = data.toMap();
            int currentId = parentId;

            if (!key.isEmpty()) {
                currentId = dbManager.insertSchemaField(parentId, key, "object");
                if (currentId == -1) return;
            }

            for (auto it = map.begin(); it != map.end(); ++it) {
                insertJsonToDb(it.value(), it.key(), currentId);
            }
        }
        else if (data.type() == QVariant::List) {
            QVariantList list = data.toList();

            if (list.isEmpty()) return;

            // Insert array node with "[]" suffix
            int arrayId = dbManager.insertSchemaField(parentId, key + "[]", "array");
            if (arrayId == -1) return;

            // Insert each array element with index-based name (e.g., "0", "1")
            for (int i = 0; i < list.size(); ++i) {
                QString elementName = QString::number(i); // Unique name for each element
                int elementId = dbManager.insertSchemaField(arrayId, elementName, "array_element");
                if (elementId == -1) continue;
                insertJsonToDb(list[i], "", elementId); // Recursively insert element content
            }
        }
        else {
            // Leaf node: insert key-value pair directly
            if (!key.isEmpty()) {
                QString valueStr = data.toString();
                dbManager.insertSchemaField(parentId, key, valueStr);
            }
        }
    };

    // Initialize recursive insert for the root of the JSON structure
    insertJsonToDb(jsonMap, "", 0);

    // Print the database contents for verification
    dbManager.printSchemaTable();

    // Refresh suggestions with the new data
    m_suggestionManager->refreshDatabase();

    setupNodeUI();

    setupAutocomplete();
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

void MainWindow::on_actionJSON_triggered()
{
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


void MainWindow::on_actionXML_triggered()
{
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


void MainWindow::on_actionGAML_triggered()
{
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

void MainWindow::on_actionYAML_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export as YAML"), "",
                                                    tr("YAML Files (*.yaml *.yml)"));

    if (!fileName.isEmpty()) {
        DatabaseManager& dbManager = DatabaseManager::instance();
        if (dbManager.exportToYaml(fileName)) {
            QMessageBox::information(this, "Export Successful",
                                     "Database exported to YAML successfully.");
        } else {
            QMessageBox::warning(this, "Export Failed",
                                 "Failed to export database to YAML.");
        }
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    this->pw->show();
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() != Qt::ControlModifier)
        return;
    event->setAccepted(false);

    QWidget *tracker = this->ui->scrollArea_2;
    int xTracker = 0;
    int yTracker = 0;
    while (tracker != this) {
        xTracker += tracker->x();
        yTracker += tracker->y();
        tracker = tracker->parentWidget();
    }
    double sizeFactor = 1.5;
    if (event->angleDelta().y() < 0)  // scroll up
        sizeFactor = 1/sizeFactor;
    nodeManager->alterNodeSize(sizeFactor);
    cout << nodeManager->getNodes()[0]->header->font().pointSize() << " IS POINT SIZE" << endl;
    nodeManager->fontSize *= sizeFactor;

    for (Node* node:nodeManager->getNodes()) {
        QFont tempFont = node->header->font();
        if (nodeManager->fontSize < 1)
            tempFont.setPointSize(1);
        else
            tempFont.setPointSize(nodeManager->fontSize);
        node->header->setFont(tempFont);
        tempFont.setBold(false);
        node->bottomBar->setFont(tempFont);
        node->setFixedSize(nodeManager->nodeSize);
        node->show();
//        cout << "NODE size :: " << node->size().width() << endl;
//        cout << "NODE size :: " << node->size().height() << endl;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        this->ui->scrollArea_2->setEnabled(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        this->ui->scrollArea_2->setEnabled(true);
    }
}

void MainWindow::on_actionAddNode_triggered()
{
    this->addNodeDialogue->show();
}


void MainWindow::on_actionEdit_Schema_triggered()
{
    // Clear the UI
    clearNodeUI();
    nodeManager->nodesFromSchema(schemaHandler->getCurrSchema(), false);
    setupNodeUI();
}

