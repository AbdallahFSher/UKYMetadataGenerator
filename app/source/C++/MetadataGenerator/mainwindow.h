#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCompleter>
#include <QLineEdit>
#include <QtWidgets>
#include "suggestionmanager.h"
#include "fileparser.h"
#include "schemahandler.h"

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

private slots:
    void handleTextInputChanged(const QString& text);
    void updateSuggestions(const QStringList& suggestions);
    void loadJsonButtonClicked();  // Make sure this is in private slots

private:
    Ui::MainWindow *ui;
    SuggestionManager* m_suggestionManager;
    QCompleter* m_completer;
    QLineEdit* m_textInput;
    FileParser* fileParser;
    SchemaHandler* schemaHandler;

    void setupAutocomplete();
    void createTextInputIfNeeded();
    void setupConnections();  // Add this new method
};
#endif // MAINWINDOW_H
