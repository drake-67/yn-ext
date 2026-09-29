#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <string>
#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <Windows.h>

#define canadian static

namespace logger {
	enum level {
		info,
		warn,
		error,
		debug
	};

	inline void setup() {
		CONSOLE_FONT_INFOEX font;
		ZeroMemory(&font, sizeof(font));

		font.cbSize = sizeof(font);
		font.nFont = 0;
		font.dwFontSize.X = 0;
		font.dwFontSize.Y = 17;
		font.FontFamily = FF_DONTCARE;
		font.FontWeight = FW_NORMAL;
		wcscpy_s(font.FaceName, L"Courier New");

		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &font);
	}

	template<level T>
	void print(const char* fmt, ...) {
		std::time_t now = std::time(nullptr);
		std::tm tm_buffer;
		localtime_s(&tm_buffer, &now);
		char time_str[100];
		std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buffer);

		switch (T) {
		case level::info:
			std::printf("\x1b[32m[INFO][%s]\x1b[0m ", time_str);
			break;
		case level::warn:
			std::printf("\x1b[33m[WARN][%s]\x1b[0m ", time_str);
			break;
		case level::error:
			std::printf("\x1b[31m[ERROR][%s]\x1b[0m ", time_str);
			break;
		case level::debug:
			std::printf("\x1b[34m[DEBUG][%s]\x1b[0m ", time_str);
			break;
		default:
			std::printf("\x1b[37m[LOG][%s]\x1b[0m ", time_str);
			break;
		}

		va_list args;
		va_start(args, fmt);
		std::vprintf(fmt, args); 
		va_end(args);

		std::printf("\n");  
	}
}

#endif // OUTPUT_HPP
