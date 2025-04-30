#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCompleter>
#include <QLineEdit>
#include <QtWidgets>
#include "suggestionmanager.h"
#include "fileparser.h"
#include "schemahandler.h"
#include "nodemanager.h"
#include "preferenceswindow.h"
#include "addnodedialogue.h"
#include "drawconnection.h"

class SuggestionManager;
class Node;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow* getUi();
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public slots:
    void nodeAdded(Node* newNode);
    void nodeDeleted(Node* node);
    void updateNodeKey(Node* node);
    void updateNodeValue(Node* node);

private:
    Ui::MainWindow *ui;
    SuggestionManager* m_suggestionManager;
    QCompleter* m_completer;
    QLineEdit* m_textInput;
    FileParser* fileParser;
    SchemaHandler* schemaHandler;
    NodeManager* nodeManager;
    ColorHandler* colorHandler;
    PreferencesWindow* pw;
    AddNodeDialogue* addNodeDialogue;
    DrawConnection* drawConnection;

    // look up the DB id from a node's fullName path
    static int lookupDbIdFor(Node* node);

    void setupAutocomplete();
    void createTextInputIfNeeded();
    void setupConnections();
    void createExportButtons();
    void setupNodeUI();
    void clearNodeUI();

private slots:
    // handle autocomplete text changes
    void handleTextInputChanged(const QString& text);
    void updateSuggestions(const QStringList& suggestions);

    void loadJsonButtonClicked();
    void on_actionLoad_Schema_triggered();
    void on_actionExport_as_triggered();
    void on_actionJSON_triggered();
    void on_actionXML_triggered();
    void on_actionGAML_triggered();
    void on_actionYAML_triggered();
    void on_actionPreferences_triggered();
    void on_actionAddNode_triggered();
    void on_actionEdit_Schema_triggered();
};

#endif // MAINWINDOW_H
