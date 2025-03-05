#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QJsonDocument>;

class fileParser
{
public:
    fileParser();
    QJsonDocument importJson(QString filePath);
    void exportJson();
};

#endif // FILEPARSER_H
