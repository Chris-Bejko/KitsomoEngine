#pragma once
#include <memory>

class Command
{
public:
	virtual ~Command() = default;
	
	virtual void Execute() = 0;
	virtual void Undo() = 0;
	virtual std::string GetDescription() const = 0;
};
