#pragma once
#include "Sphere.h"

namespace hvk
{
	namespace bounds
	{
		struct Bounds
		{
			float xMin;
			float xMax;
			float yMin;
			float yMax;
			float zMin;
			float zMax;
		};

		Bounds getBounds(const Sphere& sphere);

#if !defined(BOUNDS_IMPL)
#define BOUNDS_IMPL
		Bounds getBounds(const Sphere& sphere)
		{
			auto center = sphere.getCenter();
			auto radius = sphere.getRadius();

			Bounds b =
			{
				center.X() - radius,
				center.X() + radius,
				center.Y() - radius,
				center.Y() + radius,
				center.Z() - radius,
				center.Z() + radius
			};

			return b;
		}
#endif
	}
}
