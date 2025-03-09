#include <vector>
#include <string>
#include <iostream>
#include <QJsonDocument>
#ifndef SCHEMA_H
#define SCHEMA_H

struct Field {
    Field* parent;
    std::vector<Field*> children;

    std::string name;
};


class Schema
{
public:
    Schema();

    Field* getRoot();
    Field* getCurr();
    Field* resetCurr();
    Field* createField(Field* field);
    Field* createField(Field* parent, std::string name);

    // DEBUG
    void printTree(Field* f);

private:
    Field* root;
    Field* curr;
};

#endif // SCHEMA_H
