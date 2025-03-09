#include <QFile>
#include "schemahandler.h"

SchemaHandler::SchemaHandler()
{
    this->schemaList = std::vector<Schema*>();
    this->currentSchema = nullptr;
}

Schema* SchemaHandler::addSchema(Schema* schema) {
    this->schemaList.push_back(schema);
    this->currentSchema = schema;
    return this->currentSchema;
}

Schema* SchemaHandler::addSchema(const char* inFile) {
    Schema* newSchema = new Schema();

    // Parse out information




    return newSchema;
}

Schema* SchemaHandler::getCurrSchema() {
    return this->currentSchema;
}

