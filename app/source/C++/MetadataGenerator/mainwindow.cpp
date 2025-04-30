#include "mainwindow.h"
#include "customscrollarea.h"
#include "GridLayoutUtil.h"
#include "./ui_mainwindow.h"
#include <QtWidgets>
#include <QSqlQuery>
#include <QSqlError>
#include <algorithm>
#include "DatabaseManager.h"
#include "node.h"
#include "colorhandler.h"
#include "suggestionmanager.h"
#include "fileparser.h"
#include "nodemanager.h"

using namespace std;

// ----------------------------------------------------------------------------
// static helper: walk the node's fullName queue in schema_fields to get its id
int MainWindow::lookupDbIdFor(Node* node)
{
    DatabaseManager& db = DatabaseManager::instance();
    QSqlQuery q(db.database());
    int parentId = 0;
    auto path = node->fullName;  // copy schema path

    while (!path.empty()) {
        QString key = QString::fromStdString(path.front());
        path.pop();

        // try exact field name and the array-suffix form
        QStringList candidates = { key, key + "[]" };
        bool found = false;

        for (const QString &name : candidates) {
            q.prepare("SELECT id FROM schema_fields "
                      "WHERE parent_id = ? AND name = ?");
            q.addBindValue(parentId);
            q.addBindValue(name);
            if (q.exec() && q.next()) {
                parentId = q.value(0).toInt();
                found = true;
                break;
            }
        }

        if (!found)
            return -1;
    }

    return parentId;
}


// ----------------------------------------------------------------------------
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

    // Open DB and enable foreign keys
    DatabaseManager& dbManager = DatabaseManager::instance();
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    QString fullPath = dbPath + "/metadata.db";

    if (!dbManager.openDatabase(fullPath)) {
        QMessageBox::critical(this, "Database Error", "Could not open or create the database.");
        return;
    }

    {
        QSqlQuery pragma(dbManager.database());
        pragma.exec("PRAGMA foreign_keys = ON");
    }

    // Create schema table
    QSqlQuery query(dbManager.database());
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS schema_fields ("
            "  id INTEGER PRIMARY KEY, "
            "  parent_id INTEGER, "
            "  name TEXT, "
            "  type TEXT, "
            "  FOREIGN KEY(parent_id) REFERENCES schema_fields(id) ON DELETE CASCADE)"
            )) {
        qDebug() << "Error creating schema table:" << query.lastError().text();
    }

    // Print existing contents
    qDebug() << "\nDatabase Contents:";
    if (query.exec("SELECT id, parent_id, name FROM schema_fields ORDER BY id")) {
        qDebug() << "ID\tParent\tName\n---------------------";
        while (query.next()) {
            qDebug() << query.value(0).toInt() << "\t"
                     << query.value(1).toInt() << "\t"
                     << query.value(2).toString();
        }
    }

    pw              = new PreferencesWindow(this);
    addNodeDialogue = new AddNodeDialogue(this, m_suggestionManager);
    setupConnections();

    colorHandler = new ColorHandler();
    nodeManager  = new NodeManager(ui->nodeHolder, *colorHandler);

    setPalette(colorHandler->getPalette());
    ui->menubar->setPalette(palette());
}

void MainWindow::handleTextInputChanged(const QString& text)
{
    m_suggestionManager->requestSuggestions(text);
}

void MainWindow::updateSuggestions(const QStringList& suggestions)
{
    auto model = qobject_cast<QStringListModel*>(m_completer->model());
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
        if (auto lay = ui->centralwidget->layout()) {
            lay->addWidget(m_textInput);
        } else {
            auto layout = new QVBoxLayout(ui->centralwidget);
            layout->addWidget(m_textInput);
            layout->addWidget(ui->nodeHolder);
        }
    }
}

void MainWindow::setupAutocomplete()
{
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);

    for (Node* n : nodeManager->getNodes()) {
        n->header->setCompleter(m_completer);
        connect(n->header, &QLineEdit::textEdited, this, &MainWindow::handleTextInputChanged);
        n->bottomBar->setCompleter(m_completer);
        connect(n->bottomBar, &QLineEdit::textEdited, this, &MainWindow::handleTextInputChanged);
        connect(n, &Node::requestDelete, this, &MainWindow::nodeDeleted);
    }

    addNodeDialogue->setupAutocomplete(m_completer);
    connect(m_suggestionManager, &SuggestionManager::suggestionsReady, this, &MainWindow::updateSuggestions);
    m_suggestionManager->initialize();
}

void MainWindow::setupConnections()
{
    if (auto btn = findChild<QPushButton*>("openButton")) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::loadJsonButtonClicked);
    } else if (auto act = findChild<QAction*>("actionOpen")) {
        connect(act, &QAction::triggered, this, &MainWindow::loadJsonButtonClicked);
    } else {
        qWarning() << "No JSON load trigger found";
    }

    connect(addNodeDialogue, SIGNAL(createNode(Node*)), this, SLOT(nodeAdded(Node*)));
}

void MainWindow::nodeAdded(Node* newNode)
{
    int parentDbId = 0;
    if (auto p = newNode->getNodeParent()) {
        parentDbId = (p->dbId > 0 ? p->dbId : lookupDbIdFor(p));
        p->dbId = parentDbId;
    }

    DatabaseManager& db = DatabaseManager::instance();
    int newDbId = db.insertSchemaField(parentDbId, newNode->getKey(), newNode->getValue());
    if (newDbId == -1) {
        QMessageBox::warning(this, tr("Add Node Failed"), tr("Could not add node to database."));
    }
    newNode->dbId = newDbId;

    newNode->setVisible(true);
    ui->gridLayout->addWidget(newNode, newNode->row, newNode->column);
    nodeManager->addNode(newNode);
    ui->nodeHolder->addWidgets(newNode->getNodeParent(), newNode);

    connect(newNode, SIGNAL(beParent(Node*)), addNodeDialogue, SLOT(setParent(Node*)));
    connect(newNode, SIGNAL(hidden(Node*)), ui->nodeHolder, SLOT(update()));
    connect(newNode, SIGNAL(moved(Node*)), ui->nodeHolder, SLOT(update()));
    connect(newNode, &Node::requestDelete, this, &MainWindow::nodeDeleted);
    connect(newNode->header, &QLineEdit::editingFinished, [this, newNode]() {
        updateNodeKey(newNode);
    });
    connect(newNode->bottomBar, &QLineEdit::editingFinished, [this, newNode]() {
        updateNodeValue(newNode);
    });
}

void MainWindow::nodeDeleted(Node* node)
{
    // 1) Gather every ID in the subtree
    DatabaseManager& mgr = DatabaseManager::instance();
    QSqlDatabase& conn = mgr.database();
    QSqlQuery q(conn);

    QVector<int> toDelete;
    std::function<void(int)> gather = [&](int pid) {
        QSqlQuery q2(conn);
        q2.prepare("SELECT id FROM schema_fields WHERE parent_id = ?");
        q2.addBindValue(pid);
        if (q2.exec()) {
            while (q2.next()) {
                int cid = q2.value(0).toInt();
                gather(cid);
                toDelete.append(cid);
            }
        }
    };

    gather(node->dbId);
    toDelete.append(node->dbId);

    // debug: print what we’re about to remove
    qDebug() << "Deleting IDs:" << toDelete;

    // 2) Delete each row individually
    for (int id : toDelete) {
        q.prepare("DELETE FROM schema_fields WHERE id = ?");
        q.addBindValue(id);
        if (!q.exec()) {
            qDebug() << "Failed to delete ID" << id << ":" << q.lastError().text();
        }
    }

    // debug: dump table after
    qDebug() << "---- DB after delete ----";
    if (q.exec("SELECT id, parent_id, name FROM schema_fields ORDER BY id")) {
        while (q.next()) {
            qDebug() << q.value(0).toInt()
            << q.value(1).toInt()
            << q.value(2).toString();
        }
    }

    // 3) Tidy up UI & in-memory
    if (auto p = node->getNodeParent()) {
        auto& sib = p->children;
        sib.erase(std::remove(sib.begin(), sib.end(), node), sib.end());
    }
    std::function<void(Node*)> reap = [&](Node* n) {
        for (auto* c : n->children) reap(c);
        ui->nodeHolder->removeWidgets(n->getNodeParent(), n);
        ui->gridLayout->removeWidget(n);
        nodeManager->removeNode(n);
        delete n;
    };
    reap(node);

    ui->nodeHolder->update();
}



void MainWindow::clearNodeUI()
{
    int rowCount = ui->gridLayout->rowCount();
    for (int row = 0; row < rowCount; ++row)
        GridLayoutUtil::removeRow(ui->gridLayout, row);

    if (!nodeManager->getNodes().empty())
        nodeManager->emptyNodeList();
}

void MainWindow::setupNodeUI()
{
    int currentColumn = 0;
    int maxParent = -1;

    for (auto* node : nodeManager->getNodes()) {
        if (node->getNodeParent()) {
            ui->nodeHolder->addWidgets(node->getNodeParent(), node);
            if (node->getNodeParent()->getName() > maxParent) {
                maxParent = node->getNodeParent()->getName();
            } else {
                currentColumn++;
            }
        }
        node->column = currentColumn;
        ui->gridLayout->addWidget(node, node->row, currentColumn);
        connect(node, SIGNAL(beParent(Node*)), addNodeDialogue, SLOT(setParent(Node*)));
        connect(node, SIGNAL(hidden(Node*)), ui->nodeHolder, SLOT(update()));
        connect(node, SIGNAL(moved(Node*)), ui->nodeHolder, SLOT(update()));
        connect(node, &Node::requestDelete, this, &MainWindow::nodeDeleted);
        connect(node->header,      &QLineEdit::editingFinished, [this,node]{ updateNodeKey(node); });
        connect(node->bottomBar,   &QLineEdit::editingFinished, [this,node]{ updateNodeValue(node); });
    }
}

void MainWindow::loadJsonButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open JSON File"),
        "",
        tr("JSON Files (*.json)")
        );
    if (fileName.isEmpty())
        return;

    QVariant jsonVariant = fileParser->importJson(fileName);
    if (!jsonVariant.isValid()) {
        qWarning() << "Failed to parse JSON file.";
        return;
    }
    QVariantMap jsonMap = jsonVariant.toMap();

    // clear UI and in‐memory nodes
    clearNodeUI();
    schemaHandler->fromVariantMap(jsonMap);
    nodeManager->nodesFromSchema(schemaHandler->getCurrSchema(), true, &jsonMap);
    ui->nodeHolder->update();

    // clear existing DB rows
    DatabaseManager& dbManager = DatabaseManager::instance();
    {
        QSqlQuery clearQ(dbManager.database());
        clearQ.exec("DELETE FROM schema_fields");
    }

    // recursively insert JSON into DB
    std::function<void(const QVariant&, const QString&, int)> insertJsonToDb;
    insertJsonToDb = [&](const QVariant& data, const QString& key, int parentId) {
        DatabaseManager& db = DatabaseManager::instance();
        if (data.type() == QVariant::Map) {
            QVariantMap map = data.toMap();
            int currentId = parentId;
            if (!key.isEmpty()) {
                currentId = db.insertSchemaField(parentId, key, "object");
                if (currentId == -1) return;
            }
            for (auto it = map.begin(); it != map.end(); ++it)
                insertJsonToDb(it.value(), it.key(), currentId);
        }
        else if (data.type() == QVariant::List) {
            QVariantList list = data.toList();
            if (list.isEmpty()) return;
            int arrayId = db.insertSchemaField(parentId, key + "[]", "array");
            if (arrayId == -1) return;
            for (int i = 0; i < list.size(); ++i) {
                int elemId = db.insertSchemaField(arrayId, QString::number(i), "array_element");
                if (elemId != -1)
                    insertJsonToDb(list[i], "", elemId);
            }
        }
        else {
            if (!key.isEmpty())
                db.insertSchemaField(parentId, key, data.toString());
        }
    };
    insertJsonToDb(jsonMap, "", 0);

    // debug dump
    dbManager.printSchemaTable();

    // ─── assign each Node its dbId by matching insertion order ────────────
    QSqlQuery mapQ(dbManager.database());
    if (!mapQ.exec("SELECT id FROM schema_fields ORDER BY id ASC")) {
        qWarning() << "Failed to fetch IDs for mapping:" << mapQ.lastError().text();
    } else {
        auto nodes = nodeManager->getNodes();
        for (Node* n : nodes) {
            if (!mapQ.next()) {
                qWarning() << "Ran out of DB rows before mapping all nodes.";
                break;
            }
            n->dbId = mapQ.value(0).toInt();
            qDebug() << "Mapped node key =" << n->getKey()
                     << " to dbId =" << n->dbId;
        }
    }

    // rebuild UI and autocomplete
    setupNodeUI();
    setupAutocomplete();
}



void MainWindow::on_actionLoad_Schema_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Schema File"), "", tr("Schema Files (*.sma)"));
    if (!fileName.isEmpty())
        schemaHandler->addSchema(fileName.toStdString());
}

void MainWindow::on_actionExport_as_triggered() {}

void MainWindow::on_actionJSON_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export as JSON"), "", tr("JSON Files (*.json)"));
    if (!fileName.isEmpty()) {
        DatabaseManager& db = DatabaseManager::instance();
        if (db.exportToJson(fileName))
            QMessageBox::information(this, "Export Successful", "Database exported to JSON successfully.");
        else
            QMessageBox::warning(this, "Export Failed", "Failed to export database to JSON.");
    }
}

void MainWindow::on_actionXML_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export as XML"), "", tr("XML Files (*.xml)"));
    if (!fileName.isEmpty()) {
        DatabaseManager& db = DatabaseManager::instance();
        if (db.exportToXml(fileName))
            QMessageBox::information(this, "Export Successful", "Database exported to XML successfully.");
        else
            QMessageBox::warning(this, "Export Failed", "Failed to export database to XML.");
    }
}

void MainWindow::on_actionGAML_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export as GAML"), "", tr("GAML Files (*.gaml)"));
    if (!fileName.isEmpty()) {
        DatabaseManager& db = DatabaseManager::instance();
        if (db.exportToGaml(fileName))
            QMessageBox::information(this, "Export Successful", "Database exported to GAML successfully.");
        else
            QMessageBox::warning(this, "Export Failed", "Failed to export database to GAML.");
    }
}

void MainWindow::on_actionYAML_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export as YAML"), "", tr("YAML Files (*.yaml *.yml)"));
    if (!fileName.isEmpty()) {
        DatabaseManager& db = DatabaseManager::instance();
        if (db.exportToYaml(fileName))
            QMessageBox::information(this, "Export Successful", "Database exported to YAML successfully.");
        else
            QMessageBox::warning(this, "Export Failed", "Failed to export database to YAML.");
    }
}

void MainWindow::on_actionPreferences_triggered() { pw->show(); }

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() != Qt::ControlModifier) return;
    event->setAccepted(false);
    QWidget *tracker = ui->scrollArea_2;
    int x=0,y=0;
    while (tracker != this) { x+=tracker->x(); y+=tracker->y(); tracker=tracker->parentWidget(); }
    double factor = event->angleDelta().y()<0?1/1.5:1.5;
    nodeManager->alterNodeSize(factor);
    nodeManager->fontSize *= factor;
    for (auto* n : nodeManager->getNodes()) {
        QFont f = n->header->font(); f.setPointSizeF(nodeManager->fontSize); n->header->setFont(f); n->bottomBar->setFont(f);
        n->setFixedSize(nodeManager->nodeSize);
        n->show();
    }
}

void MainWindow::updateNodeKey(Node* node)
{
    // grab the edited text from the QLineEdit
    QString newKey = node->header->text();

    // update the database row
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("UPDATE schema_fields SET name = ? WHERE id = ?");
    q.addBindValue(newKey);
    q.addBindValue(node->dbId);
    if (!q.exec()) {
        qDebug() << "Failed to update node key:" << q.lastError().text();
    }

    // keep the Node’s internal key in sync with the UI
    node->setKey(newKey);
}

void MainWindow::updateNodeValue(Node* node)
{
    // grab the edited text from the bottomBar
    QString newValue = node->bottomBar->text();

    // update the database row’s type/value column
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("UPDATE schema_fields SET type = ? WHERE id = ?");
    q.addBindValue(newValue);
    q.addBindValue(node->dbId);
    if (!q.exec()) {
        qDebug() << "Failed to update node value:" << q.lastError().text();
    }

    // keep the Node’s internal value in sync with the UI
    node->setValue(newValue);
}

void MainWindow::keyPressEvent(QKeyEvent *e) { if (e->key()==Qt::Key_Control) ui->scrollArea_2->setEnabled(false); }
void MainWindow::keyReleaseEvent(QKeyEvent *e){ if(e->key()==Qt::Key_Control) ui->scrollArea_2->setEnabled(true); }

void MainWindow::on_actionAddNode_triggered() { addNodeDialogue->show(); }

void MainWindow::on_actionEdit_Schema_triggered()
{
    clearNodeUI();
    nodeManager->nodesFromSchema(schemaHandler->getCurrSchema(), false);
    setupNodeUI();
}

MainWindow::~MainWindow()
{
    m_suggestionManager->cancelPendingRequests();
    auto& db = DatabaseManager::instance().database();
    QString path = db.databaseName();
    DatabaseManager::instance().closeDatabase();
    QFile f(path); if (f.exists()) f.remove();
    delete m_suggestionManager;
    delete ui;
}
