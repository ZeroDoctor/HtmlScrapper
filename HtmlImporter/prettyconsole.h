#pragma once
#include <iostream>
#include <windows.h>

namespace prettycon
{

	enum class Color : unsigned int
	{
		black = 0,
		blue,
		green,
		aqua,
		red,
		purple,
		yellow,
		white,
		gray,
		lblue,
		lgreen,
		laqua,
		lred,
		lpurple,
		lyellow,
		lwhite
	};

	enum class Attribute : unsigned int
	{
		mydefault = 0,
		fgbold = FOREGROUND_INTENSITY,
		bgbold = BACKGROUND_INTENSITY,
		underlined = COMMON_LVB_UNDERSCORE
	};

	class ColorConsole
	{
	private:
		int currentColor;
		int currentAt;

		HANDLE hConsole;

		template<typename E>
		constexpr auto toType(E e) -> typename std::underlying_type<E>::type
		{
			return static_cast<typename std::underlying_type<E>::type>(e);
		}

	public:

		void setColor(Color bg, Color fg, Attribute at) {
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			currentColor = (toType(bg) * 16) + toType(fg);
			currentAt = (int) at;

			SetConsoleTextAttribute(hConsole, currentColor | (int) at);
			//std::cout << "color is: " << currentColor << std::endl;
		}

		void restore() {
			currentColor = ((int) Color::black * 16) + (int) Color::white;
			currentAt = (int) Attribute::mydefault;

			SetConsoleTextAttribute(hConsole, currentColor);
			std::cout << std::endl;
		}
	};

	ColorConsole ccout;
}