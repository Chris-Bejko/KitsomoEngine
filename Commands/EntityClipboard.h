#pragma once
#include <string>
#include <vector>

// Forward declaration
struct SerializableComponent;

struct SerializedEntity
{
	std::string name;
	std::vector<SerializableComponent> components;
};

class EntityClipboard
{
public:
	static EntityClipboard& get()
	{
		static EntityClipboard instance;
		return instance;
	}

	void Copy(const SerializedEntity& entity)
	{
		clipboard = entity;
		hasContent = true;
	}

	SerializedEntity GetClipboard() const
	{
		return clipboard;
	}

	bool HasContent() const
	{
		return hasContent;
	}

	void Clear()
	{
		hasContent = false;
		clipboard = SerializedEntity();
	}

private:
	EntityClipboard() = default;
	SerializedEntity clipboard;
	bool hasContent = false;
};
