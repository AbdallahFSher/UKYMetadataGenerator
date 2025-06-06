
#include "schema.h"

Schema::Schema()
{
    this->root = nullptr;
    this->curr = root;
}

std::shared_ptr<Field> Schema::getRoot() {
    return this->root;
}

void Schema::setRoot(std::shared_ptr<Field> root) {
    this->root = root;
}

std::shared_ptr<Field> Schema::getCurr() {
    return this->curr;
}

std::shared_ptr<Field> Schema::resetCurr() {
    curr = root;
    return curr;
}


Field* Schema::createField(std::shared_ptr<Field> parent, std::string name) {
    Field* out = new Field(nullptr);
    out->parent = parent;
    out->name = name;

    return out;
}

// Function to print the tree (for debugging)
void Schema::printTree(const std::shared_ptr<Field>& node, int depth = 0) {
    for (int i = 0; i < depth; ++i) {
        std::cout << "  "; // Indent based on depth
    }
    std::cout << node->name << std::endl;
    for (const auto& child : node->children) {
        printTree(child, depth + 1);
    }
}

// Function to add a field path to the tree
void Schema::addFieldToTree(std::shared_ptr<Field> root, const std::vector<std::string>& fieldPath) {
    auto currentNode = root;
    std::string pathString = "";
    for (const auto& part : fieldPath) {
        pathString += part;
        // Check if the current node already has a child with this name
        auto it = std::find_if(currentNode->children.begin(), currentNode->children.end(),
                               [&part](const std::shared_ptr<Field>& node) {
                                   return node->name == part;
                               });
        if (it == currentNode->children.end()) {
            // If not, add a new child node
            auto newNode = std::make_shared<Field>(part);
            newNode->parent = currentNode;
            currentNode->children.push_back(newNode);
            currentNode = newNode;
        } else {
            // If yes, move to the existing child node
            currentNode = *it;
        }
    }
    if(!fieldsSet.insert(pathString).second)
        currentNode->count++;
}

