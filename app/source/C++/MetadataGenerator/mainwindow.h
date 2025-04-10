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

private slots:
    void handleTextInputChanged(const QString& text);
    void updateSuggestions(const QStringList& suggestions);
    void loadJsonButtonClicked();
    void on_actionLoad_Schema_triggered();
    void on_actionExport_as_triggered();
    void on_actionExport_JSON_triggered();
    void on_actionExport_XML_triggered();
    void on_actionExport_GAML_triggered();

private:
    Ui::MainWindow *ui;
    SuggestionManager* m_suggestionManager;
    QCompleter* m_completer;
    QLineEdit* m_textInput;
    FileParser* fileParser;
    SchemaHandler* schemaHandler;
    NodeManager* nodeManager;

    void setupAutocomplete();
    void createTextInputIfNeeded();
    void setupConnections();
    void createExportButtons(); // New method to programmatically create export buttons
};

#endif
