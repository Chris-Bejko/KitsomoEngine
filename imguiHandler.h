#pragma once
#include "imgui.h"
#include "imgui-SFML.h"
#include "ECSEngine.h"
#include <vector>

class ImguiHandler
{
public:
	ImguiHandler() = default;
	~ImguiHandler() = default;


	void DisplayComponents(ComponentList list);

	void DisplayEntities(std::vector<std::unique_ptr<Entity>> entities);
	void Update(sf::Time rest);

	inline static ImguiHandler& get()
	{
		if (s_instance == nullptr)
		{
			s_instance = new ImguiHandler();
		}

		return *s_instance;
	}

private:
	static ImguiHandler * s_instance;
};