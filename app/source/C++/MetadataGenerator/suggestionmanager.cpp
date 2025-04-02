#include "suggestionmanager.h"
#include <QSqlQuery>
#include <QDebug>

SuggestionManager::SuggestionManager(QObject* parent)
    : QObject(parent),
    m_dbManager(DatabaseManager::instance())
{
    m_requestTimer.setSingleShot(true);
    m_requestTimer.setInterval(300); // 300ms debounce delay
    connect(&m_requestTimer, &QTimer::timeout,
            this, &SuggestionManager::processSuggestionRequest);
}

SuggestionManager::~SuggestionManager()
{
    cancelPendingRequests();
}

void SuggestionManager::initialize()
{
    loadAllEntitiesFromDatabase();
}

void SuggestionManager::refreshDatabase()
{
    cancelPendingRequests();
    m_entitySet.clear();
    loadAllEntitiesFromDatabase();
    qDebug() << "Suggestion database refreshed";
}

void SuggestionManager::loadAllEntitiesFromDatabase()
{
    QSqlQuery query(m_dbManager.database());
    if (query.exec("SELECT DISTINCT name FROM schema_fields")) {
        while (query.next()) {
            m_entitySet.insert(query.value(0).toString());
        }
    }
    qDebug() << "Loaded" << m_entitySet.size() << "suggestions";
}

void SuggestionManager::requestSuggestions(const QString& partialInput)
{
    m_currentInput = partialInput.trimmed();
    m_requestTimer.start();
}

void SuggestionManager::cancelPendingRequests()
{
    if (m_requestTimer.isActive()) {
        m_requestTimer.stop();
    }
}

void SuggestionManager::processSuggestionRequest()
{
    QStringList matches;
    const QString lowerInput = m_currentInput.toLower();

    for (const QString& entity : m_entitySet) {
        if (entity.toLower().contains(lowerInput)) {
            matches.append(entity);

            if (matches.size() >= 10) {
                break;
            }
        }
    }

    matches.sort(Qt::CaseInsensitive);
    emit suggestionsReady(matches);
}
