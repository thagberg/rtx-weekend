#pragma once

#include <d3d12.h>
#include "bounds.h"

namespace hvk
{
	namespace aabb
	{
		template <typename ImplicitShape>
		D3D12_RAYTRACING_AABB getAABB(const ImplicitShape& is);

#if !defined(AABB_IMPL)
#define AABB_IMPL
		template <typename ImplicitShape>
		D3D12_RAYTRACING_AABB getAABB(const ImplicitShape& is)
		{
			auto b = bounds::getBounds(is);
			return D3D12_RAYTRACING_AABB{
				b.xMin,
				b.yMin,
				b.zMin,
				b.xMax,
				b.yMax,
				b.zMax
			};
		}
#endif
	}
}
