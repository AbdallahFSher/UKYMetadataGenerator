#include "suggestionmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

SuggestionManager::SuggestionManager(QObject* parent)
    : QObject(parent),
    m_dbManager(DatabaseManager::instance()) // Make sure DatabaseManager::instance() is returning a valid instance
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
    m_entitySet.clear();  // Clear current set of entities before refreshing
    loadAllEntitiesFromDatabase();
    qDebug() << "Suggestion database refreshed";
}

void SuggestionManager::loadAllEntitiesFromDatabase()
{
    QSqlQuery query(m_dbManager.database());
    if (query.exec("SELECT DISTINCT name FROM schema_fields")) {
        while (query.next()) {
            QString entity = query.value(0).toString();
            m_entitySet.insert(entity);
        }
        qDebug() << "Loaded" << m_entitySet.size() << "suggestions";
    } else {
        qWarning() << "Failed to load suggestions from database:" << query.lastError().text();
    }
}

void SuggestionManager::requestSuggestions(const QString& partialInput)
{
    m_currentInput = partialInput.trimmed();
    if (!m_currentInput.isEmpty()) {
        m_requestTimer.start();
    }
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
                break; // Limit the number of suggestions
            }
        }
    }

    matches.sort(Qt::CaseInsensitive);  // Sort suggestions alphabetically
    emit suggestionsReady(matches);
}
