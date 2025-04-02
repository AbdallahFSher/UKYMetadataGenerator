#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QJsonDocument>
#include <QFile>

class FileParser
{
public:
    FileParser();
    QVariant importJson(QString filePath);
    void exportJson();

    QVariantMap getCurrentData();
    QJsonDocument getCurrentJSON();
    QVariant getCurrentVariant();

private:
    QVariantMap currentData;
    QJsonDocument currentJSON;
    QVariant currentVariant;
};

#endif // FILEPARSER_H
