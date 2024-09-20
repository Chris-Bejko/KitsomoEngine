#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>


enum VariableType
{
	None = 0,
	Int_Type,
	Float_Type,
	Char_Type,
	Bool_Type,
};


struct SerializableVariable
{
	const char* name = 0;
	void* data = 0;
	int type = 0;

	void assign(double number);
	double read(); 
};
