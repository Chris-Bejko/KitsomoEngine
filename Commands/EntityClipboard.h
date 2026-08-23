#pragma once
#include <string>
#include <vector>

// Forward declaration
class SerializedComponent;

struct SerializedEntityClipboard
{
	std::string name;
	std::vector<SerializedComponent> components;
};

class EntityClipboard
{
public:
	static EntityClipboard& get()
	{
		static EntityClipboard instance;
		return instance;
	}

	void Copy(const SerializedEntityClipboard& entity)
	{
		clipboard = entity;
		hasContent = true;
	}

	SerializedEntityClipboard GetClipboard() const
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
		clipboard = SerializedEntityClipboard();
        clipboard.components.clear();
	}

private:
	EntityClipboard() = default;
	SerializedEntityClipboard clipboard;
	bool hasContent = false;
};
