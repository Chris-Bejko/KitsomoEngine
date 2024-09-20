#include "Serialization.h"



double SerializableVariable::read()
{
	switch (type)
	{
	case Int_Type:
		return *((int*)data);
		break;
	case Float_Type:
		return *((float*)data);
		break;
	case Char_Type:
		return *((char*)data);
		break;
	case Bool_Type:
		return *((bool*)data);
		break;
	default:
		assert(0);
	}
	return 0;

}

void SerializableVariable::assign(double number)
{
	switch (type)
	{
	case Int_Type:
		*((int*)data) = number;
		break;
	case Float_Type:
		*((float*)data) = number;
		break;
	case Char_Type:
		*((char*)data) = number;
		break;
	case Bool_Type:
		*((bool*)data) = number;
		break;
	default:
		assert(0);
	}
}
