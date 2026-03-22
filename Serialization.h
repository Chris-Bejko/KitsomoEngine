#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>
#include <map>

enum VariableType
{
    None = 0,
    int_Type,
    float_Type,
    char_Type,
    bool_Type,
    entityRef_Type,
    compRef_Type,
};

struct SerializableVariable
{
    const char* name = 0;
    void* data = 0;
    int type = 0;
    std::string componentTypeHint = ""; // kept for inspector display only

    void assign(double number);
    double read();
};

struct ReadableSerializableVariableMap
{
    std::map<std::string, int>         intFields;
    std::map<std::string, float>       floatFields;
    std::map<std::string, std::string> stringFields; 
    std::map<std::string, bool>        boolFields;
};

struct SerializableComponent
{
    std::string componentName;
    std::string guiD;
    ReadableSerializableVariableMap fields;
    std::vector<SerializableVariable> variables;
};

struct SerializableEntity
{
    std::string guiD;
    std::string entityName;
    std::string parentGUID;
    std::vector<SerializableComponent> components;
};