#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtWidgets>
#include <QSqlQuery>
#include <QSqlError>
#include "node.h"
#include "colorhandler.h"
#include "suggestionmanager.h"
#include "fileparser.h"

using namespace std;

// 1. Added data table creation
void createDataTable(DatabaseManager& dbManager) {
    QSqlQuery query(dbManager.database());
    query.exec("CREATE TABLE IF NOT EXISTS data_fields ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "parent_id INTEGER DEFAULT 0, "
               "field_name TEXT, "
               "field_value TEXT, "
               "FOREIGN KEY(parent_id) REFERENCES data_fields(id))");
}

void insertFieldTree(const std::shared_ptr<Field>& node, int parentId, DatabaseManager& dbManager) {
    QSqlQuery checkQuery(dbManager.database());
    checkQuery.prepare("SELECT id FROM schema_fields WHERE parent_id = ? AND name = ?");
    checkQuery.addBindValue(parentId);
    checkQuery.addBindValue(QString::fromStdString(node->name));

    if (checkQuery.exec() && checkQuery.next()) {
        qDebug() << "Skipping duplicate schema field:" << node->name.c_str();
        return;
    }

    int currentId = dbManager.insertSchemaField(parentId, QString::fromStdString(node->name));
    if (currentId == -1) return;

    for (const auto& child : node->children) {
        insertFieldTree(child, currentId, dbManager);
    }
}

void MainWindow::setupConnections()
{
    // Connect the open button/action if they exist
    if (auto button = findChild<QPushButton*>("openButton")) {
        connect(button, &QPushButton::clicked,
                this, &MainWindow::loadJsonButtonClicked);
    }
    else if (auto action = findChild<QAction*>("actionOpen")) {
        connect(action, &QAction::triggered,
                this, &MainWindow::loadJsonButtonClicked);
    }
    else {
        qWarning() << "No JSON load trigger found in UI";
    }

    // Connect node signals if needed
    for (Node* node : m_nodes) {
        connect(node, &Node::toggled,
                this, &MainWindow::handleNodeToggle);
    }

    // Connect text input signals if they exist
    if (m_textInput) {
        connect(m_textInput, &QLineEdit::returnPressed,
                this, [](){ qDebug() << "Text input submitted"; });
    }
}

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

    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.openDatabase("example.db")) {
        qDebug() << "Could not open database!";
    }

    // Create both schema and data tables
    QSqlQuery schemaQuery(dbManager.database());
    schemaQuery.exec("CREATE TABLE IF NOT EXISTS schema_fields ("
                     "id INTEGER PRIMARY KEY, "
                     "parent_id INTEGER, "
                     "name TEXT, "
                     "FOREIGN KEY(parent_id) REFERENCES schema_fields(id))");

    createDataTable(dbManager); // Create data table

    Schema* currentSchema = schemaHandler->addSchema("C:\\Users\\abdal\\Documents\\UKYMetadataGenerator\\app\\examples\\exampleSchema.sma");
    if (currentSchema && currentSchema->getRoot()) {
        insertFieldTree(currentSchema->getRoot(), 0, dbManager);
    }

    Node *node1 = new Node(ui->nodeHolder, 0);
    Node *node2 = new Node(ui->nodeHolder, 1);
    node1->setText("One");
    node2->setText("Two");

    connect(node1, &Node::toggled, this, &MainWindow::handleNodeToggle);
    connect(node2, &Node::toggled, this, &MainWindow::handleNodeToggle);

    m_nodes.push_back(node1);
    m_nodes.push_back(node2);
    layoutNodes();

    createTextInputIfNeeded();
    setupAutocomplete();
    setupConnections();

    ColorHandler *colorHandler = new ColorHandler();
    this->setPalette(colorHandler->getPalette());
    colorHandler->setColors(m_nodes);
}

void MainWindow::handleNodeToggle(Node* node) {
    node->updateChildrenVisibility();
    layoutNodes();
}

void MainWindow::loadJsonButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open JSON File"),
                                                    "",
                                                    tr("JSON Files (*.json)"));
    if (fileName.isEmpty()) return;

    QVariantMap jsonMap = fileParser->importJson(fileName).toMap();
    DatabaseManager& dbManager = DatabaseManager::instance();
    QSqlDatabase db = dbManager.database();

    if (!db.transaction()) {
        qDebug() << "Transaction error:" << db.lastError().text();
        return;
    }

    try {
        std::function<void(const QVariantMap&, int)> insertMap = [&](const QVariantMap& map, int parentId) {
            for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                QSqlQuery checkQuery;
                checkQuery.prepare("SELECT COUNT(*) FROM data_fields WHERE parent_id = ? AND field_name = ?");
                checkQuery.addBindValue(parentId);
                checkQuery.addBindValue(it.key());

                if (!checkQuery.exec() || !checkQuery.next()) {
                    throw runtime_error("Duplicate check failed: " + checkQuery.lastError().text().toStdString());
                }

                if (checkQuery.value(0).toInt() > 0) {
                    qDebug() << "Skipping duplicate field:" << it.key();
                    continue;
                }

                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO data_fields (parent_id, field_name, field_value) VALUES (?, ?, ?)");
                insertQuery.addBindValue(parentId);
                insertQuery.addBindValue(it.key());
                insertQuery.addBindValue(it.value().toString());

                if (!insertQuery.exec()) {
                    throw runtime_error("Insert failed: " + insertQuery.lastError().text().toStdString());
                }

                if (it.value().type() == QVariant::Map) {
                    insertMap(it.value().toMap(), insertQuery.lastInsertId().toInt());
                }
            }
        };

        insertMap(jsonMap, 0);

        if (!db.commit()) {
            throw runtime_error("Commit failed: " + db.lastError().text().toStdString());
        }
    } catch (const exception& e) {
        db.rollback();
        qDebug() << "Transaction aborted:" << e.what();
    }

    ui->jsonLabel->setText(QJsonDocument::fromVariant(jsonMap).toJson(QJsonDocument::Indented));
    m_suggestionManager->refreshDatabase();
}

void MainWindow::layoutNodes()
{
    const int baseX = 200;
    const int baseY = 200;
    const int indent = 30;
    const int verticalSpacing = 20;

    std::function<void(Node*, int, int&)> layoutNode = [&](Node* node, int indentLevel, int& yPos) {
        if (!node->isVisible()) return;

        int x = baseX + (indentLevel * indent);
        node->move(x, yPos);
        yPos += node->height() + verticalSpacing;

        for (Node* child : node->children) {
            layoutNode(child, indentLevel + 1, yPos);
        }
    };

    int currentY = baseY;
    for (Node* node : m_nodes) {  // <-- Changed from foreach to range-based for
        if (!node->parent) { // Only layout root nodes
            layoutNode(node, 0, currentY);
        }
    }
}

void MainWindow::createTextInputIfNeeded()
{
    // Try to find existing text input
    m_textInput = findChild<QLineEdit*>("textInput");

    if (!m_textInput) {
        // Create new text input if it doesn't exist
        m_textInput = new QLineEdit(this);
        m_textInput->setObjectName("textInput");
        m_textInput->setPlaceholderText("Enter text here...");

        // Add to layout if one exists, otherwise create new layout
        if (ui->centralwidget->layout()) {
            // Insert at top of existing layout
            ui->centralwidget->layout()->addWidget(m_textInput);
        } else {
            // Create new vertical layout
            QVBoxLayout* layout = new QVBoxLayout(ui->centralwidget);
            layout->addWidget(m_textInput);
            layout->addWidget(ui->nodeHolder);
            layout->setContentsMargins(0, 0, 0, 0);
        }
    }
}

void MainWindow::setupAutocomplete()
{
    // Configure the completer
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setMaxVisibleItems(5);

    // Set the completer on the text input
    m_textInput->setCompleter(m_completer);

    // Connect signals and slots
    connect(m_textInput, &QLineEdit::textEdited,
            this, &MainWindow::handleTextInputChanged);
    connect(m_suggestionManager, &SuggestionManager::suggestionsReady,
            this, &MainWindow::updateSuggestions);

    // Initialize with empty suggestions
    QStringListModel* model = new QStringListModel(this);
    m_completer->setModel(model);

    // Initialize the suggestion manager
    m_suggestionManager->initialize();
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
    }
}



MainWindow::~MainWindow()
{
    // Cleanup allocated memory
    qDeleteAll(m_nodes);
    delete m_suggestionManager;
    delete fileParser;
    delete schemaHandler;
    delete ui;
}
