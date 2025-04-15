#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <QJsonDocument>
#ifndef SCHEMA_H
#define SCHEMA_H

struct Field {
    std::shared_ptr<Field> parent;
    std::vector<std::shared_ptr<Field>> children;
    std::string name;
    int count = 1;

    Field(const std::string& name) : name(name) {}
};


class Schema
{
public:
    Schema();

    std::shared_ptr<Field> getRoot();
    void setRoot(std::shared_ptr<Field> root);
    std::shared_ptr<Field> getCurr();
    std::shared_ptr<Field> resetCurr();
    Field* createField(Field* field);
    Field* createField(std::shared_ptr<Field> parent, std::string name);
    void addFieldToTree(std::shared_ptr<Field> root, const std::vector<std::string>& fieldPath);

    // DEBUG
    void printTree(const std::shared_ptr<Field>& node, int depth);

private:
    std::shared_ptr<Field> root;
    std::shared_ptr<Field> curr;
    std::set<std::string> fieldsSet;
};

#endif // SCHEMA_H
