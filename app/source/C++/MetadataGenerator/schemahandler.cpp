#include <QFile>
#include <stack>
#include <fstream>
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

Schema* SchemaHandler::addSchema(std::string inFile) {
    Schema* newSchema = new Schema();
    auto root = extractFieldNames(inFile, *newSchema);
    newSchema->setRoot(root);
    newSchema->printTree(root, 0);
    schemaList.push_back(newSchema);
    currentSchema = newSchema;
    return newSchema;
}


Schema* SchemaHandler::getCurrSchema() {
    return this->currentSchema;
}

Schema* SchemaHandler::setCurrSchema(Schema* schema) {
    this->currentSchema = schema;
    return currentSchema;
}

std::string SchemaHandler::trimString(std::string in) {
    int i = 0;
    while(illegalChars.find(in[i]) != illegalChars.end()) {
        i++;
    }
    int j = in.length() - 1;
    while(illegalChars.find(in[j]) != illegalChars.end()) {
        j--;
    }
    return in.substr(i, j - i + 1);
}

// Function to extract field names from the schema file
std::shared_ptr<Field> SchemaHandler::extractFieldNames(const std::string& filePath, Schema& newSchema) {
    auto root = std::make_shared<Field>("Root"); // Root of the tree
    std::ifstream inputFile(filePath); // Open the file

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return root;
    }

    std::string line;
    std::stack<std::string> contextStack; // Tracks nested contexts

    //TODO: Fix non alphanumeric characters appearing in Field Names
    while (std::getline(inputFile, line)) {
        // Remove leading and trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip empty lines or lines with only braces
        if (line.empty() || line == "{" || line == "}" || line == "[" || line == "]") {
            continue;
        }

        // Check if the line starts a new context (array or object)
        if (line.back() == '{' || line.back() == '[') {
            // Extract the context name (e.g., "Metadata", "Data Citation", etc.)
            std::string contextName = line.substr(0, line.find_last_of(" \t{["));
            contextName = trimString(contextName);
            contextStack.push(contextName); // Push the context onto the stack
        }
        // Check if the line ends a context
        else if (line.front() == '}' || line.front() == ']') {
            if (!contextStack.empty()) {
                contextStack.pop(); // Pop the context from the stack
            }
        }
        // Otherwise, it's a field name
        else {
            // Extract the field name (e.g., "Title", "doi", "Name", etc.)
            std::string fieldName = line.substr(0, line.find_last_of("\""));
            if (!fieldName.empty()) {
                // Construct the full field path (e.g., ["Metadata", "Data Citation", "Title"])
                fieldName = trimString(fieldName);
                std::vector<std::string> fieldPath;
                // Copy the stack contents to a vector for iteration
                std::vector<std::string> contextVector;
                while (!contextStack.empty()) {
                    contextVector.push_back(contextStack.top());
                    contextStack.pop();
                }
                // Restore the stack (since we popped all elements)
                for (auto it = contextVector.rbegin(); it != contextVector.rend(); ++it) {
                    contextStack.push(*it);
                }
                // Build the field path
                for (auto it = contextVector.rbegin(); it != contextVector.rend(); ++it) {
                    fieldPath.push_back(*it);
                }
                fieldPath.push_back(fieldName);
                // Add the field path to the tree
                newSchema.addFieldToTree(root, fieldPath);
            }
        }
    }

    inputFile.close(); // Close the file
    return root;
}

Schema* SchemaHandler::fromVariantMap(QVariantMap map) {
    Schema* newSchema = new Schema();
    this->setCurrSchema(newSchema);
    std::shared_ptr<Field> root = std::make_shared<Field>("root");
    std::vector<std::string> emptyVec;

    newSchema->setRoot(fieldsFromQVMap(map, emptyVec, root)->children[0]);
    newSchema->printTree(newSchema->getRoot(), 0);
    this->schemaList.push_back(newSchema);

    // TODO: Set this up to add a new schema and set it to the current schema instead of just returning it
    return newSchema;
}

// Loop through each key
// Each key can be a dictionary, a list, or a leaf
// If the key is a dictionary, convert it to a QVMap and call the function again using the new map
// If the key is a leaf, use the parentFields and the root to add to the tree
std::shared_ptr<Field> SchemaHandler::fieldsFromQVMap(QVariantMap jsonMap, std::vector<std::string> parentFields, std::shared_ptr<Field> root) {

    for(QString key : jsonMap.keys()) {

        // Create buffer vector
        std::vector<std::string> buff;
        for (auto& item : parentFields) { buff.push_back(item); }

        if (jsonMap[key].canConvert<QVariantMap>()) {

            buff.push_back(key.toStdString());
            fieldsFromQVMap(jsonMap[key].toMap(), buff, root);

        } else if(jsonMap[key].toString() == jsonMap[key]) {

            buff.push_back(key.toStdString());
            this->currentSchema->addFieldToTree(root, buff);

        } else {

            for(QVariant child : jsonMap[key].toList()) {

                QVariantMap mapChild = child.toMap();
                buff.clear();

                for (auto& item : parentFields) { buff.push_back(item); }

                for (QString childKey:mapChild.keys()) {

                    if (mapChild[childKey] == mapChild[childKey].toString()) {

                        buff.push_back(key.toStdString());
                        break;

                    }
                }

                fieldsFromQVMap(mapChild, buff, root);
            }
        }
    }
    return root;
}

