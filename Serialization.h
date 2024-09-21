#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>


enum VariableType
{
	None = 0,
	int_Type,
	float_Type,
	char_Type,
	bool_Type,
};


struct SerializableVariable
{
	const char* name = 0;
	void* data = 0;
	int type = 0;

	void assign(double number);
	double read(); 
};

struct SerializableComponent
{
	std::string componentName;
	std::vector<SerializableVariable> fields;
};

struct SerializableEntity
{
	std::string entityName;
	std::vector<SerializableComponent> components;
};