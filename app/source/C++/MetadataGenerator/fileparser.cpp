#include "fileparser.h"
#include "json.hpp"

FileParser::FileParser() {

}

QVariant FileParser::importJson(QString filePath) {
    // Open the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Could not open file: " + filePath.toStdString());
    }

    // Read the file content
    QByteArray jsonData = file.readAll();
    file.close();

    // Parse JSON document
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        throw std::runtime_error("JSON parse error: " + parseError.errorString().toStdString());
    }

    // Convert to QVariant
    QVariant jsonVariant = jsonDoc.toVariant();
    this->currentData = jsonVariant.toMap();
    this->currentJSON = jsonDoc;
    this->currentVariant = jsonVariant;

    return jsonVariant;
}

QVariantMap FileParser::getCurrentData() {
    return this->currentData;
}

QJsonDocument FileParser::getCurrentJSON() {
    return this->currentJSON;
}

QVariant FileParser::getCurrentVariant() {
    return this->currentVariant;
}
