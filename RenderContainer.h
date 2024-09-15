#pragma once

#include "SFML/Graphics.hpp"
class RenderContainer
{
public:
    RenderContainer() = default;
    ~RenderContainer() = default;

    inline static RenderContainer& get()
    {
        if (s_instance == nullptr)
        {
            s_instance = new RenderContainer();
        }

        return *s_instance;
    }

    inline sf::RenderWindow* GetRenderer()
    {
        return renderer;
    }

    inline void SetRenderer(sf::RenderWindow* renderer)
    {
        this->renderer = renderer;
    }

private:
    sf::RenderWindow* renderer;
    static RenderContainer* s_instance;
};