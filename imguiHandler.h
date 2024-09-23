#pragma once
#include "imgui.h"
#include "imgui-SFML.h"
#include "Engine.h"
#include <vector>

class ImguiHandler
{
public:
	ImguiHandler() = default;
	~ImguiHandler() = default;


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
	bool savePressed;
};