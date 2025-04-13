#include "fileparser.h"
#include "json.hpp"

FileParser::FileParser() {

}

QVariant FileParser::importJson(QString filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Could not open file: " + filePath.toStdString());
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        throw std::runtime_error("JSON parse error: " + parseError.errorString().toStdString());
    }

    QVariant jsonVariant = jsonDoc.toVariant();
    this->currentJSON = jsonDoc;
    this->currentVariant = jsonVariant;

    // Handle either an object or array at the top level
    if (jsonVariant.type() == QVariant::Map) {
        this->currentData = jsonVariant.toMap(); // top-level object
    } else if (jsonVariant.type() == QVariant::List) {
        QVariantMap wrapper;
        wrapper["root"] = jsonVariant; // wrap list in map for compatibility
        this->currentData = wrapper;
    } else {
        throw std::runtime_error("Unsupported JSON top-level type");
    }

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
