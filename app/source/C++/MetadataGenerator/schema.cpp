
#include "schema.h"

Schema::Schema()
{
    this->root = nullptr;
    this->curr = root;
}

Field* Schema::getRoot() {
    return this->root;
}

Field* Schema::getCurr() {
    return this->curr;
}

Field* Schema::resetCurr() {
    curr = root;
    return curr;
}

// Creates field as child of curr
// Sets curr to new field
// returns curr
Field* Schema::createField(Field* field) {
    if(root == nullptr) {
        root = field;
        curr = root;
    } else {
        if(!field->parent)
            field->parent = curr;
        curr->children.push_back(field);
        curr = field;
    }

    return curr;
}

Field* Schema::createField(Field* parent, std::string name) {
    Field* out = new Field();
    out->parent = parent;
    out->name = name;

    return out;
}

void Schema::printTree(Field* f) {
    std::cout << f->name << std::endl;
    if(f->children.size() != 0) {
        for(int i = 0; i < f->children.size(); i++) {
            printTree(f->children[i]);
        }
    }
    return;
}

