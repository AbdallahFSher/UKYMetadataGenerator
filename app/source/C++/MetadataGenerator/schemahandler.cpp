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

Schema SchemaHandler::addSchema(const char* inFile) {
    Schema newSchema = Schema();
    auto curr = newSchema.getRoot();
    Field* newField;

    // Parse out information
    std::shared_ptr<Field> root = extractFieldNames(inFile, newSchema);
    newSchema.setRoot(root);
    newSchema.printTree(root, 0);

    return newSchema;
}

Schema* SchemaHandler::getCurrSchema() {
    return this->currentSchema;
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
            std::string fieldName = line.substr(0, line.find_last_of(" \t"));
            if (!fieldName.empty()) {
                // Construct the full field path (e.g., ["Metadata", "Data Citation", "Title"])
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

