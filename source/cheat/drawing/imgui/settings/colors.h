#pragma once
#include "../imgui.h"

class c_colors
{
public:
	ImColor accent{ 0.0348633f, 0.313692f, 0.865772f };

	struct
	{
		ImColor background_one{ 0.0563038f, 0.0563038f, 0.0671141f };
		ImColor background_two{ 0.0794559f, 0.0794559f, 0.0939597f };
		ImColor stroke{ 0.158101f, 0.158101f, 0.181208f };
		ImColor outline{ 0.141176f, 0.141176f, 0.188235f };
	} window;

	struct
	{
		ImColor stroke_two{ 0.141176f, 0.141176f, 0.188235f };
		ImColor text{ 0.784314f, 0.784314f, 0.784314f };
		ImColor text_inactive{ 0.533333f, 0.533333f, 0.533333f };
	} widgets;
};

inline c_colors* clr = new c_colors();
