#include "Rect2D.h"

namespace Maple 
{
	Rect2D::Rect2D(int32_t left, int32_t top, int32_t right, int32_t bottom)
		:leftTop(left,top),rightBottom(right,bottom)
	{
	}

	Rect2D::Rect2D()
		: leftTop(), rightBottom()
	{

	}

};

