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
    //void mousePressEvent(QMouseEvent *event);

public slots:
    void nodeAdded(Node* newNode, int parentDatabaseId);
private slots:
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

    void setupAutocomplete();
    void createTextInputIfNeeded();
    void setupConnections();
    void createExportButtons(); // New method to programmatically create export buttons
};

#endif
