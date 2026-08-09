#include "Serialization.h"



double SerializableVariable::read()
{
	switch (type)
	{
	case int_Type:
		return *((int*)data);
		break;
	case float_Type:
		return *((float*)data);
		break;
	case char_Type:
		return *((char*)data);
		break;
	case bool_Type:
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
	case int_Type:
		*((int*)data) = number;
		break;
	case float_Type:
		*((float*)data) = number;
		break;
	case char_Type:
		*((char*)data) = number;
		break;
	case bool_Type:
		*((bool*)data) = number;
		break;
	default:
		assert(0);
	}
}
