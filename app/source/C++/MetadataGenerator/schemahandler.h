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
    Schema* setCurrSchema(int index);
    Schema* getCurrSchema();
    Schema* addSchema(Schema* schema); // TODO: Adds schema sent from schema editor
    Schema* addSchema(const char* inFile); // TODO: Adds schema from .sma file

private:
    std::vector<Schema*> schemaList;
    Schema* currentSchema;

    bool readJsonFile(std::string file_path, QVariantMap& result);

};

#endif // SCHEMAHANDLER_H
