#pragma once

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl\client.h>
#include <exception>
#include <cstdint>
#include <cassert>
#include <vector>

using namespace Microsoft::WRL;

namespace hvk
{
	namespace boiler
	{
		constexpr uint32_t kDefaultInstanceMask = 1;

		template <typename T, typename U>
		T Align(T offset, U alignment)
		{
			return (offset + (alignment - 1)) & ~(alignment - 1);
		}

		void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);

		bool SupportsRaytracing(ComPtr<ID3D12Device> device);

		HRESULT CreateFactory(ComPtr<IDXGIFactory4>& factoryOut);

		//HRESULT CreateDevice(ComPtr<IDXGIFactory4> factory, ComPtr<IDXGIAdapter1> hardwareAdapter, ComPtr<ID3D12Device>& deviceOut);

		template <typename DeviceClass>
		HRESULT CreateDevice(ComPtr<IDXGIFactory4> factory, ComPtr<IDXGIAdapter1> hardwareAdapter, ComPtr<DeviceClass>& deviceOut);

		HRESULT CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& cqOut);

		HRESULT CreateSwapchain(
			ComPtr<ID3D12CommandQueue> commandQueue,
			ComPtr<IDXGIFactory4> factory,
			HWND window,
			uint8_t bufferCount,
			uint16_t width,
			uint16_t height,
			ComPtr<IDXGISwapChain3>& scOut);

		HRESULT CreateDescriptorHeaps(
			ComPtr<ID3D12Device> device,
			uint8_t framebufferCount,
			ComPtr<ID3D12DescriptorHeap>& rtvOut,
			ComPtr<ID3D12DescriptorHeap>& miscOut,
			ComPtr<ID3D12DescriptorHeap>& samplersOut);

		HRESULT CreateRenderTargetView(
			ComPtr<ID3D12Device> device,
			ComPtr<IDXGISwapChain1> swapchain,
			ComPtr<ID3D12DescriptorHeap> rtvHeap,
			size_t numRendertargets,
			ComPtr<ID3D12Resource>* rtvOut);

		HRESULT CreateCommandAllocator(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandAllocator>& caOut);

		HRESULT CreateRootSignature(
			ComPtr<ID3D12Device> device,
			const std::vector<D3D12_ROOT_PARAMETER>& rootParams,
			const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplers,
			ComPtr<ID3D12RootSignature>& rsOut);

		HRESULT CreateGraphicsPipelineState(
			ComPtr<ID3D12Device> device,
			D3D12_INPUT_LAYOUT_DESC& inputLayout,
			ComPtr<ID3D12RootSignature> rootSig,
			const uint8_t* vertexShader,
			size_t vertexShaderSize,
			const uint8_t* pixelShader,
			size_t pixelShaderSize,
			ComPtr<ID3D12PipelineState>& psOut);

		HRESULT CreateComputePipelineState(
			ComPtr<ID3D12Device> device,
			ComPtr<ID3D12RootSignature> rootSig,
			const uint8_t* computeShader,
			size_t computeShaderSize,
			ComPtr<ID3D12PipelineState>& psOut);

		template <typename DeviceClass, typename GraphicsCommandListClass>
		HRESULT CreateCommandList(
			ComPtr<DeviceClass> device,
			ComPtr<ID3D12CommandAllocator> allocator,
			ComPtr<ID3D12PipelineState> pipeline,
			ComPtr<GraphicsCommandListClass>& clOut);

		HRESULT CreateFence(ComPtr<ID3D12Device> device, ComPtr<ID3D12Fence>& fOut);

		D3D12_VERTEX_BUFFER_VIEW CreateVertexBuffer(ComPtr<ID3D12Device> device, const uint8_t* vbData, uint32_t vbSize, uint32_t stride, ComPtr<ID3D12Resource>& vbOut);

		HRESULT WaitForGraphics(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue);

		D3D12_HEAP_PROPERTIES HeapPropertiesDefault();

		HRESULT CreateResource(
			ComPtr<ID3D12Device> device,
			D3D12_RESOURCE_DIMENSION resourceDimension,
			DXGI_FORMAT format,
			uint64_t width,
			uint64_t height,
			uint16_t depthOrArraySize,
			uint16_t numMips,
			D3D12_RESOURCE_FLAGS flags,
			D3D12_TEXTURE_LAYOUT layout,
			D3D12_RESOURCE_STATES resourceStates,
			D3D12_HEAP_TYPE heaptype,
			ComPtr<ID3D12Resource>& outResource);

		HRESULT RGBAToTexture(
			ComPtr<ID3D12Device> device,
			ComPtr<ID3D12GraphicsCommandList> commandList,
			ComPtr<ID3D12CommandQueue> commandQueue,
			std::vector<uint8_t> rgbaData,
			uint32_t width,
			uint32_t height,
			ComPtr<ID3D12Resource> outTexture);

		HRESULT CreateBuffer(
			ComPtr<ID3D12Device> device,
			const D3D12_HEAP_PROPERTIES& heapProps,
			uint64_t alignment,
			size_t width,
			D3D12_RESOURCE_FLAGS flags,
			ComPtr<ID3D12Resource>& outBuffer,
			D3D12_RESOURCE_STATES initialState=D3D12_RESOURCE_STATE_GENERIC_READ);

		D3D12_HEAP_PROPERTIES CreateHeapProperties(D3D12_HEAP_TYPE type, D3D12_CPU_PAGE_PROPERTY cpuPageProperty, D3D12_MEMORY_POOL memoryPoolPreferences);

		HRESULT CopyBufferGPUImmediate(
			ComPtr<ID3D12Device> device,
			ComPtr<ID3D12GraphicsCommandList> commandList,
			ComPtr<ID3D12CommandQueue> commandQueue,
			ComPtr<ID3D12Resource> sourceBuffer,
			ComPtr<ID3D12Resource> destBuffer,
			D3D12_RESOURCE_STATES destState);

		HRESULT CreateGeometryBLAS(
			ComPtr<ID3D12Device5> device,
			ComPtr<ID3D12GraphicsCommandList5> commandList,
			D3D12_GPU_VIRTUAL_ADDRESS aabbStart,
			const std::vector<std::vector<D3D12_RAYTRACING_AABB>>& aabbs,
			ComPtr<ID3D12Resource>& scratchOut,
			ComPtr<ID3D12Resource>& blasOut);

		void SetTLASInstanceValues(
			D3D12_RAYTRACING_INSTANCE_DESC* instanceAddress,
			uint32_t instanceId,
			uint32_t hitGroupIndex,
			const XMMATRIX& transform,
			D3D12_GPU_VIRTUAL_ADDRESS blasAddress,
			uint8_t instanceMask = kDefaultInstanceMask,
			uint8_t flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE);

#if !defined(D3D12_BOILER)
#define D3D12_BOILER

		const uint16_t kMiscDescriptors = 16;
		const uint16_t kSamplerDescriptors = 16;

		void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
		{
			ComPtr<IDXGIAdapter1> adapter;
			*ppAdapter = nullptr;

			for (uint8_t i = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(i, &adapter); ++i)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				// Skip software adapter (WARP)
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					continue;
				}

				// break on the first hardware adapter
				// TODO: Probably want to ensure it supports D3D12
				break;
			}

			*ppAdapter = adapter.Detach();
		}

		bool SupportsRaytracing(ComPtr<ID3D12Device> device)
		{
			D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
			device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
			return options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
		}

		HRESULT CreateFactory(ComPtr<IDXGIFactory4>& factoryOut)
		{
			auto hr = S_OK;

			hr = CreateDXGIFactory1(IID_PPV_ARGS(&factoryOut));

			return hr;
		}

		template <typename DeviceClass>
		HRESULT CreateDevice(ComPtr<IDXGIFactory4> factory, ComPtr<IDXGIAdapter1> hardwareAdapter, ComPtr<DeviceClass>& deviceOut)
		{
#if defined(_DEBUG)
			// Enable debug layers
			{
				ComPtr<ID3D12Debug1> debugController;
				assert(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))));
			}
#endif

			HRESULT hr = S_OK;

			hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&deviceOut));
			return hr;
		}

		HRESULT CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& cqOut)
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			auto hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cqOut));
			return hr;
		}

		HRESULT CreateSwapchain(
			ComPtr<ID3D12CommandQueue> commandQueue,
			ComPtr<IDXGIFactory4> factory,
			HWND window,
			uint8_t bufferCount,
			uint16_t width,
			uint16_t height,
			ComPtr<IDXGISwapChain3>& scOut)
		{
			DXGI_SWAP_CHAIN_DESC1 desc = {};
			desc.BufferCount = bufferCount;
			desc.Width = width;
			desc.Height = height;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.Scaling = DXGI_SCALING_STRETCH;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;

			// Using nullptr for the FULLSCREEN_DESC to ensure this is windowed
			ComPtr<IDXGISwapChain1> swap1;
			auto hr = factory->CreateSwapChainForHwnd(
				commandQueue.Get(),
				window,
				&desc,
				nullptr,
				nullptr,
				&swap1);
			assert(SUCCEEDED(hr));
			hr = swap1->QueryInterface<IDXGISwapChain3>(&scOut);
			return hr;
		}

		HRESULT CreateDescriptorHeaps(
			ComPtr<ID3D12Device> device,
			uint8_t framebufferCount,
			ComPtr<ID3D12DescriptorHeap>& rtvOut,
			ComPtr<ID3D12DescriptorHeap>& miscOut,
			ComPtr<ID3D12DescriptorHeap>& samplersOut)
		{
			auto hr = S_OK;

			D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
			rtvDesc.NumDescriptors = framebufferCount;
			rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			hr = device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvOut));
			if (hr != S_OK)
			{
				return hr;
			}

			D3D12_DESCRIPTOR_HEAP_DESC miscDesc = {};
			miscDesc.NumDescriptors = kMiscDescriptors;
			miscDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			miscDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			hr = device->CreateDescriptorHeap(&miscDesc, IID_PPV_ARGS(&miscOut));
			if (hr != S_OK)
			{
				return hr;
			}

			D3D12_DESCRIPTOR_HEAP_DESC samplersDesc = {};
			samplersDesc.NumDescriptors = kSamplerDescriptors;
			samplersDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			samplersDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			hr = device->CreateDescriptorHeap(&samplersDesc, IID_PPV_ARGS(&samplersOut));

			return hr;
		}

		HRESULT CreateRenderTargetView(
			ComPtr<ID3D12Device> device,
			ComPtr<IDXGISwapChain1> swapchain,
			ComPtr<ID3D12DescriptorHeap> rtvHeap,
			size_t numRendertargets,
			ComPtr<ID3D12Resource>* rtvOut)
		{
			auto hr = S_OK;
			auto rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

			auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			for (size_t i = 0; i < numRendertargets; ++i)
			{
				// TODO: provide D3D12_RENDER_TARGET_VIEW_DESC instead of nullptr
				hr = swapchain->GetBuffer(static_cast<uint32_t>(i), IID_PPV_ARGS(&rtvOut[i]));
				if (SUCCEEDED(hr))
				{
					device->CreateRenderTargetView(rtvOut[i].Get(), nullptr, rtvHandle);
					rtvHandle.ptr += descriptorSize;
				}
				else
				{
					break;
				}
			}
			return hr;
		}

		HRESULT CreateCommandAllocator(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandAllocator>& caOut)
		{
			auto hr = S_OK;

			hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&caOut));

			return hr;
		}

		HRESULT CreateRootSignature(
			ComPtr<ID3D12Device> device,
			const std::vector<D3D12_ROOT_PARAMETER>& rootParams,
			const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplers,
			ComPtr<ID3D12RootSignature>& rsOut)
		{
			auto hr = S_OK;

			D3D12_ROOT_SIGNATURE_DESC desc = {};
			desc.NumParameters = static_cast<uint8_t>(rootParams.size());
			desc.pParameters = rootParams.data();
			desc.NumStaticSamplers = static_cast<uint8_t>(samplers.size());
			desc.pStaticSamplers = samplers.data();
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
			hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rsOut));

			return hr;
		}

		HRESULT CreateGraphicsPipelineState(
			ComPtr<ID3D12Device> device,
			D3D12_INPUT_LAYOUT_DESC& inputLayout,
			ComPtr<ID3D12RootSignature> rootSig,
			const uint8_t* vertexShader,
			size_t vertexShaderSize,
			const uint8_t* pixelShader,
			size_t pixelShaderSize,
			ComPtr<ID3D12PipelineState>& psOut)
		{
			auto hr = S_OK;

			D3D12_RENDER_TARGET_BLEND_DESC rtBlend = {};
			rtBlend.BlendEnable = FALSE;
			rtBlend.LogicOpEnable = FALSE;
			rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			D3D12_BLEND_DESC blend = {};
			blend.AlphaToCoverageEnable = FALSE;
			blend.IndependentBlendEnable = FALSE;
			for (uint8_t i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			{
				blend.RenderTarget[i] = rtBlend;
			}

			D3D12_RASTERIZER_DESC raster = {};
			raster.FillMode = D3D12_FILL_MODE_SOLID;
			raster.CullMode = D3D12_CULL_MODE_NONE;
			raster.FrontCounterClockwise = FALSE;
			raster.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			raster.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			raster.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			raster.DepthClipEnable = TRUE;
			raster.MultisampleEnable = FALSE;
			raster.AntialiasedLineEnable = FALSE;
			raster.ForcedSampleCount = 0;
			raster.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			D3D12_DEPTH_STENCIL_DESC depthStencil = {};
			depthStencil.DepthEnable = FALSE;
			depthStencil.StencilEnable = FALSE;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
			desc.InputLayout = inputLayout;
			desc.pRootSignature = rootSig.Get();
			desc.VS = { vertexShader, vertexShaderSize };
			desc.PS = { pixelShader, pixelShaderSize };
			desc.RasterizerState = raster;
			desc.BlendState = blend;
			desc.SampleMask = UINT_MAX; // TODO: learn more about this
			desc.DepthStencilState = depthStencil;
			desc.NodeMask = 0;
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			desc.NumRenderTargets = 1;
			desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;

			hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&psOut));

			return hr;
		}

		HRESULT CreateComputePipelineState(
			ComPtr<ID3D12Device> device,
			ComPtr<ID3D12RootSignature> rootSig,
			const uint8_t* computeShader,
			size_t computeShaderSize,
			ComPtr<ID3D12PipelineState>& psOut)
		{
			auto hr = S_OK;

			D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
			desc.CS = { computeShader, computeShaderSize };
			desc.NodeMask = 0;
			desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			desc.pRootSignature = rootSig.Get();

			hr = device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&psOut));

			return hr;
		}

		template <typename DeviceClass, typename GraphicsCommandListClass>
		HRESULT CreateCommandList(
			ComPtr<DeviceClass> device,
			ComPtr<ID3D12CommandAllocator> allocator,
			ComPtr<ID3D12PipelineState> pipeline,
			ComPtr<GraphicsCommandListClass>& clOut)
		{
			auto hr = S_OK;

			hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), pipeline.Get(), IID_PPV_ARGS(&clOut));

			return hr;
		}

		HRESULT CreateFence(ComPtr<ID3D12Device> device, ComPtr<ID3D12Fence>& fOut)
		{
			auto hr = S_OK;

			hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fOut));

			return hr;
		}

		D3D12_VERTEX_BUFFER_VIEW CreateVertexBuffer(ComPtr<ID3D12Device> device, const uint8_t* vbData, uint32_t vbSize, uint32_t stride, ComPtr<ID3D12Resource>& vbOut)
		{
			auto hr = S_OK;

			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC vbDesc = {};
			vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			vbDesc.Alignment = 0;
			vbDesc.Width = vbSize;
			vbDesc.Height = 1;
			vbDesc.DepthOrArraySize = 1;
			vbDesc.MipLevels = 1;
			vbDesc.Format = DXGI_FORMAT_UNKNOWN;
			vbDesc.SampleDesc.Count = 1;
			vbDesc.SampleDesc.Quality = 0;
			vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			vbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			hr = device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&vbDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&vbOut));
			assert(SUCCEEDED(hr));

			D3D12_RANGE readRange = {};
			readRange.Begin = 0;
			readRange.End = 0;

			uint8_t* vbBegin;
			vbOut->Map(0, &readRange, reinterpret_cast<void**>(&vbBegin));
			memcpy(vbBegin, vbData, vbSize);
			vbOut->Unmap(0, nullptr);

			D3D12_VERTEX_BUFFER_VIEW vbView;
			vbView.BufferLocation = vbOut->GetGPUVirtualAddress();
			vbView.StrideInBytes = stride;
			vbView.SizeInBytes = vbSize;
			return vbView;
		}

		D3D12_HEAP_PROPERTIES HeapPropertiesDefault()
		{
			D3D12_HEAP_PROPERTIES props = {};
			props.Type = D3D12_HEAP_TYPE_DEFAULT;
			props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			props.CreationNodeMask = 1;
			props.VisibleNodeMask = 1;
			return props;
		}

		HRESULT CreateResource(
			ComPtr<ID3D12Device> device,
			D3D12_RESOURCE_DIMENSION resourceDimension,
			DXGI_FORMAT format,
			uint64_t width,
			uint64_t height,
			uint16_t depthOrArraySize,
			uint16_t numMips,
			D3D12_RESOURCE_FLAGS flags,
			D3D12_TEXTURE_LAYOUT layout,
			D3D12_RESOURCE_STATES resourceStates,
			D3D12_HEAP_TYPE heapType,
			ComPtr<ID3D12Resource>& outResource)
		{
			HRESULT hr = S_OK;

			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = resourceDimension;
			desc.Format = format;
			desc.Alignment = 0;
			desc.Width = width;
			desc.Height = height;
			desc.DepthOrArraySize = depthOrArraySize;
			desc.MipLevels = numMips;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Flags = flags;
			desc.Layout = layout;

			D3D12_HEAP_PROPERTIES heapProps = HeapPropertiesDefault();
			heapProps.Type = heapType;

			hr = device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				resourceStates,
				nullptr,
				IID_PPV_ARGS(&outResource));

			return hr;
		}

		HRESULT WaitForGraphics(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue)
		{
			HRESULT hr = S_OK;
			ComPtr<ID3D12Fence> fence;
			hr = CreateFence(device, fence);
			uint64_t fenceValue = 1;
			hr = commandQueue->Signal(fence.Get(), fenceValue);
			assert(SUCCEEDED(hr));

			if (SUCCEEDED(hr))
			{
				auto fenceEvent = CreateEvent(nullptr, false, false, nullptr);
				auto completedVal = fence->GetCompletedValue();
				if (completedVal != fenceValue)
				{
					hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
					assert(SUCCEEDED(hr));
					WaitForSingleObject(fenceEvent, INFINITE);
				}
			}

			return hr;
		}

		HRESULT RGBAToTexture(
			ComPtr<ID3D12Device> device,
			ComPtr<ID3D12GraphicsCommandList> commandList,
			ComPtr<ID3D12CommandQueue> commandQueue,
			std::vector<uint8_t> rgbaData,
			uint32_t width,
			uint32_t height,
			ComPtr<ID3D12Resource> outTexture)
		{
			HRESULT hr = S_OK;

			ComPtr<ID3D12Resource> tempBuffer;

			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			const auto paddedWidth = Align(width * 4, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * height - (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - (width * 4));
			D3D12_RESOURCE_DESC bufferDesc = {};
			bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			bufferDesc.Width = paddedWidth;
			bufferDesc.Height = 1;
			bufferDesc.DepthOrArraySize = 1;
			bufferDesc.MipLevels = 1;
			bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			bufferDesc.SampleDesc.Count = 1;
			bufferDesc.SampleDesc.Quality = 0;
			bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			hr = device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&tempBuffer));
			assert(SUCCEEDED(hr));

			uint8_t* bufferData;
			tempBuffer->Map(0, nullptr, reinterpret_cast<void**>(&bufferData));
			uint8_t* writeAt = bufferData;
			const size_t writeSize = width * 4;
			for (size_t i = 0; i < height; ++i)
			{
				size_t sourceOffset = (i * width * 4);
				memcpy(writeAt, rgbaData.data() + sourceOffset, writeSize);
				writeAt = reinterpret_cast<uint8_t*>(Align(reinterpret_cast<size_t>(writeAt) + writeSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
			}
			tempBuffer->Unmap(0, nullptr);

			D3D12_TEXTURE_COPY_LOCATION copySource = {};
			copySource.pResource = tempBuffer.Get();
			copySource.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

			size_t requiredSize = 0;
			device->GetCopyableFootprints(&outTexture->GetDesc(), 0, 1, 0, &copySource.PlacedFootprint, nullptr, nullptr, &requiredSize);

			D3D12_RESOURCE_BARRIER copyBarrier = {};
			copyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			copyBarrier.Transition.pResource = outTexture.Get();
			copyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			copyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
			commandList->ResourceBarrier(1, &copyBarrier);

			D3D12_TEXTURE_COPY_LOCATION copyDest = {};
			copyDest.pResource = outTexture.Get();
			copyDest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			copyDest.SubresourceIndex = 0;

			commandList->CopyTextureRegion(&copyDest, 0, 0, 0, &copySource, nullptr);

			D3D12_RESOURCE_BARRIER srvBarrier = {};
			srvBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			srvBarrier.Transition.pResource = outTexture.Get();
			srvBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			srvBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			commandList->ResourceBarrier(1, &srvBarrier);

			commandList->Close();
			ID3D12CommandList* commandLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
			hr = WaitForGraphics(device, commandQueue);

			return hr;
		}

		HRESULT CopyBufferGPUImmediate(
			ComPtr<ID3D12Device> device,
			ComPtr<ID3D12GraphicsCommandList> commandList,
			ComPtr<ID3D12CommandQueue> commandQueue,
			ComPtr<ID3D12Resource> sourceBuffer,
			ComPtr<ID3D12Resource> destBuffer,
			D3D12_RESOURCE_STATES destState)
		{
			auto hr = S_OK;

			//D3D12_RESOURCE_BARRIER sourceCopyBarrier = {};
			//sourceCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//sourceCopyBarrier.Transition.pResource = sourceBuffer.Get();
			//sourceCopyBarrier.Transition.StateBefore = sourceState;
			//sourceCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

			D3D12_RESOURCE_BARRIER destCopyBarrier = {};
			destCopyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			destCopyBarrier.Transition.pResource = destBuffer.Get();
			destCopyBarrier.Transition.StateBefore = destState;
			destCopyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

			const D3D12_RESOURCE_BARRIER preBarriers[] = { destCopyBarrier };
			commandList->ResourceBarrier(_countof(preBarriers), preBarriers);

			commandList->CopyResource(destBuffer.Get(), sourceBuffer.Get());

			//sourceCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			//sourceCopyBarrier.Transition.StateAfter = sourceState;
			destCopyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			destCopyBarrier.Transition.StateAfter = destState;

			const D3D12_RESOURCE_BARRIER postBarriers[] = { destCopyBarrier };
			commandList->ResourceBarrier(_countof(postBarriers), postBarriers);

			commandList->Close();
			ID3D12CommandList* commandLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
			hr = WaitForGraphics(device, commandQueue);

			return hr;
		}

		HRESULT CreateBuffer(
			ComPtr<ID3D12Device> device, 
			const D3D12_HEAP_PROPERTIES& heapProps, 
			uint64_t alignment, 
			size_t width, 
			D3D12_RESOURCE_FLAGS flags, 
			ComPtr<ID3D12Resource>& outBuffer,
			D3D12_RESOURCE_STATES initialState)
		{
			auto hr = S_OK;

			D3D12_RESOURCE_DESC bufferDesc = {};
			bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufferDesc.Alignment = alignment;
			bufferDesc.Width = width;
			bufferDesc.Height = 1;
			bufferDesc.DepthOrArraySize = 1;
			bufferDesc.MipLevels = 1;
			bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			bufferDesc.SampleDesc.Count = 1;
			bufferDesc.SampleDesc.Quality = 0;
			bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			bufferDesc.Flags = flags;

			hr = device->CreateCommittedResource(
				&heapProps, 
				D3D12_HEAP_FLAG_NONE, 
				&bufferDesc, 
				initialState, 
				nullptr, 
				IID_PPV_ARGS(&outBuffer));

			return hr;
		}

		D3D12_HEAP_PROPERTIES CreateHeapProperties(D3D12_HEAP_TYPE type, D3D12_CPU_PAGE_PROPERTY cpuPageProperty, D3D12_MEMORY_POOL memoryPoolPreference)
		{
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = type;
			heapProps.CPUPageProperty = cpuPageProperty;
			heapProps.MemoryPoolPreference = memoryPoolPreference;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			return heapProps;
		}

		HRESULT CreateGeometryBLAS(
			ComPtr<ID3D12Device5> device, 
			ComPtr<ID3D12GraphicsCommandList5> commandList,
			D3D12_GPU_VIRTUAL_ADDRESS aabbStart, 
			const std::vector<std::vector<D3D12_RAYTRACING_AABB>>& aabbs,
			ComPtr<ID3D12Resource>& scratchOut,
			ComPtr<ID3D12Resource>& blasOut)
		{
			HRESULT hr = S_OK;
			
			// Create a geometry description for each group that will correspond to a hit group
			std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
			geometryDescs.reserve(aabbs.size());
			for (const auto& geomGroup : aabbs)
			{
				D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE addressAndStride = {};
				addressAndStride.StartAddress = aabbStart;
				addressAndStride.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);

				D3D12_RAYTRACING_GEOMETRY_AABBS_DESC aabbsDesc = {};
				aabbsDesc.AABBCount = geomGroup.size();
				aabbsDesc.AABBs = addressAndStride;

				geometryDescs.push_back({ 
					.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS, 
					.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE, 
					.AABBs = aabbsDesc });
			}

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			inputs.NumDescs = geometryDescs.size();
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.pGeometryDescs = geometryDescs.data();
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
			device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);
			assert(prebuild.ResultDataMaxSizeInBytes > 0);

			hr = CreateBuffer(
				device, 
				CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN), 
				D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 
				prebuild.ScratchDataSizeInBytes, 
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				scratchOut,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			assert(SUCCEEDED(hr));

			hr = CreateBuffer(
				device, 
				CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN), 
				D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 
				prebuild.ResultDataMaxSizeInBytes, 
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				blasOut,
				D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
			assert(SUCCEEDED(hr));

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
			buildDesc.ScratchAccelerationStructureData = scratchOut->GetGPUVirtualAddress();
			buildDesc.DestAccelerationStructureData = blasOut->GetGPUVirtualAddress();
			buildDesc.Inputs = inputs;
			commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

			return hr;
		}

		void SetTLASInstanceValues(
			D3D12_RAYTRACING_INSTANCE_DESC* instanceAddress,
			uint32_t instanceId,
			uint32_t hitGroupIndex,
			const XMMATRIX& transform,
			D3D12_GPU_VIRTUAL_ADDRESS blasAddress,
			uint8_t instanceMask,
			uint8_t flags)
		{
			instanceAddress = reinterpret_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(Align(reinterpret_cast<size_t>(instanceAddress), D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT));
			instanceAddress->InstanceID = instanceId;
			instanceAddress->InstanceContributionToHitGroupIndex = hitGroupIndex;
			instanceAddress->InstanceMask = instanceMask;
			instanceAddress->Flags = flags;
			instanceAddress->AccelerationStructure = blasAddress;
			memcpy(instanceAddress->Transform, &transform, sizeof(XMMATRIX));
		}

		//HRESULT CreateTLASInputs(uint32_t numInputs, const std::vector<ComPtr<ID3D12Resource>>& topInstances)
		//{
		//	assert(topInstances.size() > 0);
		//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		//	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		//	inputs.NumDescs = topInstances.size();
		//	inputs.InstanceDescs = topInstances[0]->GetGPUVirtualAddress();
		//}
#endif // D3D12_BOILER
	}
}
