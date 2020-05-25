#ifndef OBJECT_CLASS_H
#define OBJECT_CLASS_H

#include <string>

struct ObjectClass {
    int id;
    std::string name;

    ObjectClass(int id, std::string name)
    : id(id), name(name) {};
};

#endif