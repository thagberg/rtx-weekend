#pragma once

#include "d3d12boiler/d3d12boiler.h"

namespace hvk
{
    namespace as
    {
        HRESULT CreateBottomLevelAS(
            ComPtr<ID3D12Device> device, 
            ComPtr<ID3D12GraphicsCommandList> commandList,
            ComPtr<ID3D12CommandQueue> commandQueue,
            ComPtr<ID3D12Resource>& outBlas)
        {

        }
    }
}
