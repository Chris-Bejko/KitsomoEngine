#pragma once

#include "SFML/Graphics.hpp"

enum ColorEnum
{
	Black,
	White,
	Red,
	Green,
	Blue,
	Yellow,
	Magenta,
	Cyan,
	Transparent
};

struct Color
{
public:
	sf::Color GetColor()
	{
		return color;
	}

	Color() = default;
	~Color() = default;

	void SetColor(sf::Color color)
	{
		this->color = color;
	}
	void SetColor(std::string Color)
	{
		if (Color == "Black")
		{
			this->color = sf::Color(sf::Color::Black);
			return;
		}
		if (Color == "Red")
		{
			this->color = sf::Color(sf::Color::Red);
			return;
		}
		if (Color == "Green")
		{
			this->color = sf::Color(sf::Color::Green);
			return;
		}
		if (Color == "Blue")
		{
			this->color = sf::Color(sf::Color::Blue);
			return;

		}
		if (Color == "Yellow")
		{
			this->color = sf::Color(sf::Color::Yellow);
			return;

		}
		if (Color == "Magenta")
		{
			this->color = sf::Color(sf::Color::Magenta);
			return;

		}
		if (Color == "Cyan")
		{
			this->color = sf::Color(sf::Color::Cyan);
			return;

		}
		if (Color == "Transparent")
		{
			this->color = sf::Color(sf::Color::Transparent);
			return;
		}
		this->color = sf::Color(sf::Color::White);

	}

	std::string SerializeColor()
	{
		
		if(color == sf::Color::Black)
		{
			return "Black";
		}

		if (color == sf::Color::Red)
		{
			return "Black";
		}

		if (color == sf::Color::Green)
		{
			return "Black";
		}

		if (color == sf::Color::Blue)
		{
			return "Black";
		}

		if (color == sf::Color::Yellow)
		{
			return "Black";
		}

		if (color == sf::Color::Magenta)
		{
			return "Black";
		}

		if (color == sf::Color::Cyan)
		{
			return "Black";
		}

		if (color == sf::Color::Transparent)
		{
			return "Black";
		}
		return "White";
	}

	Color(ColorEnum colour)
	{
		switch (colour)
		{
		case Black:
			this->color = sf::Color(sf::Color::Black);
			break;
		case White:
			this->color = sf::Color(sf::Color::White);
			break;
		case Red:
			this->color = sf::Color(sf::Color::Red);
			break;
		case Green:
			this->color = sf::Color(sf::Color::Green);
			break;
		case Blue:
			this->color = sf::Color(sf::Color::Blue);
			break;
		case Yellow:
			this->color = sf::Color(sf::Color::Yellow);
			break;
		case Magenta:
			this->color = sf::Color(sf::Color::Magenta);
			break;
		case Cyan:
			this->color = sf::Color(sf::Color::Cyan);
			break;
		case Transparent:
			this->color = sf::Color(sf::Color::Transparent);
			break;
		default:
			this->color = sf::Color(sf::Color::White);
			break;
		}
	}

	inline bool operator==(const Color v2)const
	{
		return this->color == v2.color;
	}
private:
	sf::Color color;
};


