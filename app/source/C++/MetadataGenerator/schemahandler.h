#include <vector>
#include <QJsonDocument>
#include <QJsonObject>
#include "schema.h"
#ifndef SCHEMAHANDLER_H
#define SCHEMAHANDLER_H




class SchemaHandler
{
public:
    SchemaHandler();
    Schema* getSchema(int index);
    Schema* setCurrSchema(Schema* schema);
    Schema* getCurrSchema();
    Schema* addSchema(Schema* schema); // TODO: Adds schema sent from schema editor
    Schema* addSchema(std::string inFile); // TODO: Adds schema from .sma file
    Schema* fromVariantMap(QVariantMap map);

private:
    std::vector<Schema*> schemaList;
    Schema* currentSchema;

    bool readJsonFile(std::string file_path, QVariantMap& result);
    std::shared_ptr<Field> extractFieldNames(const std::string& filePath, Schema& newSchema);
    std::shared_ptr<Field> fieldsFromQVMap(QVariantMap map);

};

#endif // SCHEMAHANDLER_H
