#ifndef SUGGESTIONMANAGER_H
#define SUGGESTIONMANAGER_H

#include <QObject>
#include <QStringList>
#include <QSet>
#include <QTimer>
#include "DatabaseManager.h"

class SuggestionManager : public QObject
{
    Q_OBJECT

public:
    explicit SuggestionManager(QObject* parent = nullptr);
    ~SuggestionManager();

    void initialize();
    void requestSuggestions(const QString& partialInput);
    void cancelPendingRequests();
    void refreshDatabase();  // Added refresh method

signals:
    void suggestionsReady(const QStringList& suggestions);

private slots:
    void processSuggestionRequest();

private:
    void loadAllEntitiesFromDatabase();

    QSet<QString> m_entitySet;
    QTimer m_requestTimer;
    QString m_currentInput;
    DatabaseManager& m_dbManager;
};

#endif // SUGGESTIONMANAGER_H
