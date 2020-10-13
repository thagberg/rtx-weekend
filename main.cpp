#include <iostream>
#include <vector>
#include <optional>

#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <DirectXMath.h>

using namespace DirectX;
#endif

#include <entt/entt.hpp>
#include <d3d12boiler/d3d12boiler.h>

#include "Ray.h"
#include "Sphere.h"
#include "Material.h"
#include "hittest.h"
#include "math.h"
#include "HitRecord.h"
#include "Box.h"
#include "ThreadPool.h"
#include "Camera.h"
#include "aabb.h"

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 TexCoord;
};

using Color = hvk::Vector;

const hvk::Vector kSkyColor1 = hvk::Vector(1.f, 1.f, 1.f);
const hvk::Vector kSkyColor2 = hvk::Vector(0.5f, 0.7f, 1.f);

const uint16_t kNumSamples = 10;
const uint16_t kMaxRayDepth = 5;

const double kMinDepth = 0.01f;
const double kMaxDepth = 5.f;

const double kIORAir = 1.f;

const uint8_t kNumThreads = 16;
const uint8_t kFramebuffers = 2;

const float kClearColor[] = { 1.f, 0.f, 1.f, 1.f };

const size_t kNumVertices = 6;
const Vertex kVertices[kNumVertices] =
{
	{DirectX::XMFLOAT3(-1.f, -1.f, 0.5f), DirectX::XMFLOAT2(0.f, 1.f)},
	{DirectX::XMFLOAT3(-1.f, 1.f, 0.5f), DirectX::XMFLOAT2(0.f, 0.f)},
	{DirectX::XMFLOAT3(1.f, -1.f, 0.5f), DirectX::XMFLOAT2(1.f, 1.f)},
	{DirectX::XMFLOAT3(1.f, -1.f, 0.5f), DirectX::XMFLOAT2(1.f, 1.f)},
	{DirectX::XMFLOAT3(-1.f, 1.f, 0.5f), DirectX::XMFLOAT2(0.f, 0.f)},
	{DirectX::XMFLOAT3(1.f, 1.f, 0.5f), DirectX::XMFLOAT2(1.f, 0.f)},

};

struct RayTestResult
{
    Color image;
    Color reflect;
    Color normal;
    Color hit;
    double depth;
};


Color rayColor(const hvk::Ray& r, entt::registry& registry, int depth, std::optional<RayTestResult>& outResult)
{
    if (depth <=0)
    {
        return Color(0.f, 0.f, 0.f);
    }

    hvk::HitRecord earliestHitRecord = {};
    earliestHitRecord.t = std::numeric_limits<double>::max();
    hvk::Material earliestMaterial(hvk::MaterialType::Diffuse, hvk::Color(0.f, 0.f, 0.f), -1.f);

    auto sphereView = registry.view<hvk::Sphere, hvk::Material>();
    for (const auto entity : sphereView)
    {
        const auto& sphere = sphereView.get<hvk::Sphere>(entity);
        const auto& material = sphereView.get<hvk::Material>(entity);
        auto intersection = hvk::hit::SphereRayIntersect(sphere, r);
        if (intersection.has_value() && intersection.value() > 0.f && intersection.value() < earliestHitRecord.t)
        {
            earliestHitRecord.t = intersection.value();
            earliestHitRecord.point = r.PointAt(earliestHitRecord.t);
            earliestHitRecord.normal = (earliestHitRecord.point - sphere.getCenter()).Normalized();
//            if (hvk::Vector::Dot(earliestHitRecord.normal, r.getDirection().Normalized()) > 0.f)
//            {
//                int blah = 5 + 5;
//                std::cout << blah << std::endl;
//            }
            earliestMaterial = material;
        }
    }

    // test for plane intersections
    auto planeView = registry.view<hvk::Plane, hvk::Material>();
    for (const auto entity : planeView)
    {
        const auto& plane = planeView.get<hvk::Plane>(entity);
        const auto& material = planeView.get<hvk::Material>(entity);
        auto intersection = hvk::hit::PlaneRayIntersect(plane, r);
        if (intersection.has_value() && intersection.value() > 0.f && intersection.value() < earliestHitRecord.t)
        {
            earliestHitRecord.t = intersection.value();
            earliestHitRecord.point = r.PointAt(earliestHitRecord.t);
            earliestHitRecord.normal = plane.getDirection().Normalized();
            earliestMaterial = material;
        }
    }

    // test for box intersections
    // on a successful hit test we'll need:
    //  t
    //  the normal of the plane the intersection occurs at
     auto boxView = registry.view<hvk::Box, hvk::Material>();
     for (const auto entity : boxView)
     {
         const auto& box = boxView.get<hvk::Box>(entity);
         const auto& material = boxView.get<hvk::Material>(entity);
         auto intersection = hvk::hit::BoxRayIntersect(box, r);
         if (intersection.has_value() && intersection.value().second > 0.f && intersection.value().second < earliestHitRecord.t)
         {
             earliestHitRecord.t = intersection.value().second;
             earliestHitRecord.point = r.PointAt(earliestHitRecord.t);
             earliestHitRecord.normal = box.getSide(intersection.value().first).getDirection().Normalized();
             earliestMaterial = material;
         }
     }

    if (earliestHitRecord.t < std::numeric_limits<double>::max())
    {
        if (depth == kMaxRayDepth && outResult.has_value())
        {
            auto& result = outResult.value();
            result.hit += earliestHitRecord.point;
            result.depth += earliestHitRecord.t;
            result.normal += 0.5f * Color(
                    earliestHitRecord.normal.X() + 1,
                    earliestHitRecord.normal.Y() + 1,
                    earliestHitRecord.normal.Z() + 1);
            auto reflected = hvk::Vector::Reflect(r.getDirection(), earliestHitRecord.normal);
            result.reflect += 0.5f * Color(reflected.X() + 1, reflected.Y() + 1, reflected.Z() + 1);
            result.image += Color(0.f, 0.f, 0.f);
        }

        hvk::Ray scattered = hvk::Ray(hvk::Vector(), hvk::Vector());
        hvk::Color attenuation(0.f, 0.f, 0.f);
        bool shouldContinue = false;
        if (earliestMaterial.getType() == hvk::MaterialType::Diffuse)
        {
            shouldContinue = hvk::ScatterDiffuse(r, earliestMaterial, earliestHitRecord, attenuation, scattered);
        }
        else if (earliestMaterial.getType() == hvk::MaterialType::Metal)
        {
            shouldContinue = hvk::ScatterMetal(r, earliestMaterial, earliestHitRecord, attenuation, scattered);
            if (!shouldContinue)
            {
                return hvk::Color(0.f, 0.f, 1.f);
            }
        }
        else if (earliestMaterial.getType() == hvk::MaterialType::Dielectric)
        {
            shouldContinue = hvk::ScatterDielectric(r, earliestMaterial, kIORAir, earliestHitRecord, attenuation, scattered);
        }

        if (shouldContinue)
        {
//            // add biasing
//            scattered = hvk::Ray(scattered.getOrigin() + (0.01) * earliestHitRecord.normal.Normalized(), scattered.getDirection());
            return attenuation * rayColor(scattered, registry, depth-1, outResult);
        }

        return hvk::Color(0.f, 0.f, 0.f);
    }
    else
    {
        hvk::Vector unitDirection = r.getDirection();
        auto t = (unitDirection.Y() + 1.f) * 0.5f;
        return (kSkyColor1 * (1.0 - t)) + (kSkyColor2 * t);
    }
}

void writeColor(const Color& c)
{
    auto ir = static_cast<int>(255.999 * c.X());
    auto ig = static_cast<int>(255.999 * c.Y());
    auto ib = static_cast<int>(255.999 * c.Z());

    std::cout << ir << ' ' << ig << ' ' << ib << std::endl;
}

void writeBuffers(
        const std::vector<Color>& colors,
        uint16_t imageWidth,
        uint16_t imageHeight,
        const std::optional<std::vector<double>>& depth,
        const std::optional<std::vector<Color>>& normals,
        const std::optional<std::vector<Color>>& reflects,
        const std::optional<std::vector<Color>>& hits)
{
std::vector<std::vector<Color>> buffers;
    buffers.push_back(colors);

    // buffers.push_back(depth);
    // buffers.push_back(normals);
    // buffers.push_back(reflects);
    if (depth.has_value())
    {
        // prepare depth buffer as colors
        std::vector<Color> depthColors;
        depthColors.resize(depth->size());
        double minDepth = std::numeric_limits<double>().max();
        double maxDepth = std::numeric_limits<double>().min();
        for (auto d : depth.value())
        {
            if (d < minDepth)
            {
                minDepth = d;
            }
            if (d > maxDepth)
            {
                maxDepth = d;
            }
        }
        for (size_t i = 0; i < depthColors.size(); ++i)
        {
            const double c = (depth->at(i) - minDepth) / (maxDepth - minDepth);
            depthColors[i] = Color(c, c, c);
        }
        buffers.push_back(depthColors);
    }
    if (normals.has_value())
    {
        buffers.push_back(normals.value());
    }
    if (reflects.has_value())
    {
        buffers.push_back(reflects.value());
    }
    if (hits.has_value())
    {
        buffers.push_back(hits.value());
    }

    const auto numColumns = static_cast<uint16_t>(std::ceil(sqrt(buffers.size())));
    const auto numRows = numColumns;
    const uint16_t width = imageWidth * numColumns;
    const uint16_t height = imageHeight * numRows;

    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    std::vector<Color> finalBuffer;
    finalBuffer.resize(width * height);

    size_t row = 0;
    size_t column = 0;
    for (const auto& buffer : buffers)
    {
        const size_t viewportRowOffset = row * (width * imageHeight);
        const size_t viewportColumnOffset = column * imageWidth;

        for (size_t scanline = 0; scanline < buffer.size() / imageWidth; ++scanline)
        {
            const size_t srcOffset = scanline * imageWidth;
            const size_t copySize = imageWidth * sizeof(hvk::Color);
            const size_t destScanlineOffset = scanline * width;
            const size_t destOffset = viewportRowOffset + viewportColumnOffset + destScanlineOffset;
            memcpy(
                reinterpret_cast<hvk::Color*>(finalBuffer.data()) + destOffset,
                reinterpret_cast<const hvk::Color*>(buffer.data()) + srcOffset,
                copySize);
        }

        ++column;
        if (column >= numColumns)
        {
            column = 0;
            ++row;
        }
    }

    for (const auto& c : finalBuffer)
    {
        writeColor(c);
    }
}

bool LoadShaderByteCode(LPCWSTR filename, std::vector<uint8_t>& byteCodeOut)
{
	DWORD codeSize;
	DWORD bytesRead;

	HANDLE shaderHandle = CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
	codeSize = GetFileSize(shaderHandle, nullptr);
	byteCodeOut.resize(codeSize);
	bool readSuccess = ReadFile(shaderHandle, byteCodeOut.data(), codeSize, &bytesRead, nullptr);
	assert(readSuccess);
	assert(bytesRead == codeSize);
    CloseHandle(shaderHandle);

	return readSuccess && (bytesRead == codeSize);
}

// Main message handler for the sample.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            // Save the DXSample* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
            return 0;

        // case WM_KEYDOWN:
        //     if (pSample)
        //     {
        //         pSample->OnKeyDown(static_cast<UINT8>(wParam));
        //     }
        //     return 0;
        //
        // case WM_KEYUP:
        //     if (pSample)
        //     {
        //         pSample->OnKeyUp(static_cast<UINT8>(wParam));
        //     }
        //     return 0;
        //
        // case WM_PAINT:
        //     if (pSample)
        //     {
        //         pSample->OnUpdate();
        //         pSample->OnRender();
        //     }
        //     return 0;
        //
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const auto aspectRatio = 16.f / 9.f;
    const uint16_t imageWidth = 600;
    const uint16_t imageHeight = static_cast<uint16_t>(imageWidth / aspectRatio);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "DXRWeekendClass";
    RegisterClassEx(&windowClass);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(1));

    RECT windowRect = { 0, 0, static_cast<LONG>(imageWidth), static_cast<LONG>(imageHeight) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.

    uint32_t windowWidth = windowRect.right - windowRect.left;
    uint32_t windowHeight = windowRect.bottom - windowRect.top;

    auto hwnd = CreateWindow(
            windowClass.lpszClassName,
            "DXR Weekend",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowWidth,
            windowHeight,
            nullptr,        // We have no parent window.
            nullptr,        // We aren't using menus.
            hInstance,
            nullptr);       // App data, probably want to pass a pointer to something later

    ShowWindow(hwnd, nCmdShow);

    ComPtr<IDXGIFactory4> factory;
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    ComPtr<ID3D12Device5> device;
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain3> swapchain;
    ComPtr<ID3D12RootSignature> rootSig;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12GraphicsCommandList5> commandList;

	HRESULT hr = S_OK;
	hr = hvk::boiler::CreateFactory(factory);
	hvk::boiler::GetHardwareAdapter(factory.Get(), &hardwareAdapter);
	hr = hvk::boiler::CreateDevice(factory, hardwareAdapter, device);
    hr = hvk::boiler::CreateCommandAllocator(device, commandAllocator);
	hr = hvk::boiler::CreateCommandQueue(device, commandQueue);
	hr = hvk::boiler::CreateSwapchain(commandQueue, factory, hwnd, 2, windowWidth, windowHeight, swapchain);

    assert(hvk::boiler::SupportsRaytracing(device));

    entt::registry registry;

    // Image setup
    std::vector<Color> writeOutBuffer;
    writeOutBuffer.resize(imageHeight * imageWidth);
    std::vector<Color> normalBuffer;
    normalBuffer.resize(imageHeight * imageWidth);
    std::vector<Color> reflectBuffer;
    reflectBuffer.resize(imageHeight * imageWidth);
    std::vector<double> depthBuffer;
    depthBuffer.resize(imageHeight * imageWidth);
    std::vector<Color> hitBuffer;
    hitBuffer.resize(imageHeight * imageWidth);

    // create descriptor table
    D3D12_DESCRIPTOR_RANGE screenSrvRange = {};
    screenSrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    screenSrvRange.NumDescriptors = 1;
    screenSrvRange.BaseShaderRegister = 0;
    screenSrvRange.RegisterSpace = 0;
    screenSrvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER screenParam = {};
    screenParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    screenParam.DescriptorTable.NumDescriptorRanges = 1;
    screenParam.DescriptorTable.pDescriptorRanges = &screenSrvRange;
    screenParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC bilinearSampler = {};
	bilinearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	bilinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	bilinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	bilinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	bilinearSampler.MipLODBias = 0.f;
	bilinearSampler.MaxAnisotropy = 1;
	bilinearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	bilinearSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	bilinearSampler.MinLOD = 0;
	bilinearSampler.MaxLOD = 0;
	bilinearSampler.ShaderRegister = 0;
	bilinearSampler.RegisterSpace = 0;
	bilinearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // create root signature
    hr = hvk::boiler::CreateRootSignature(device, { screenParam }, { bilinearSampler }, rootSig);

    // create PSO
    D3D12_INPUT_ELEMENT_DESC vertexInputs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 3 * sizeof(float), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    D3D12_INPUT_LAYOUT_DESC vertexLayout = {};
    vertexLayout.NumElements = _countof(vertexInputs);
    vertexLayout.pInputElementDescs = vertexInputs;

    std::vector<uint8_t> displayVertexByteCode;
    std::vector<uint8_t> displayPixelByteCode;
    bool shaderLoadSuccess = LoadShaderByteCode(L"DisplayVertex.cso", displayVertexByteCode);
    assert(shaderLoadSuccess);
    shaderLoadSuccess = LoadShaderByteCode(L"DisplayPixel.cso", displayPixelByteCode);

    hr = hvk::boiler::CreateGraphicsPipelineState(
        device, 
        vertexLayout, 
        rootSig, 
        displayVertexByteCode.data(), 
        displayVertexByteCode.size(), 
        displayPixelByteCode.data(), 
        displayPixelByteCode.size(), 
        pipelineState);

    // create command list
    hr = hvk::boiler::CreateCommandList(device, commandAllocator, pipelineState, commandList);
    commandList->Close();

    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> uavHeap;
    ComPtr<ID3D12DescriptorHeap> samplerHeap;
    hr = hvk::boiler::CreateDescriptorHeaps(device, kFramebuffers, rtvHeap, uavHeap, samplerHeap);

    ComPtr<ID3D12Resource> renderTargets[kFramebuffers];
    hr = hvk::boiler::CreateRenderTargetView(device, swapchain, rtvHeap, kFramebuffers, renderTargets);

    // create display texture
    ComPtr<ID3D12Resource> displayTexture;
    D3D12_HEAP_PROPERTIES texHeap = {};
    texHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
    texHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    texHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    texHeap.CreationNodeMask = 1;
    texHeap.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Width = imageWidth;
    texDesc.Height = imageHeight;
    texDesc.MipLevels = 1;
    texDesc.DepthOrArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = device->CreateCommittedResource(
        &texHeap, 
        D3D12_HEAP_FLAG_NONE, 
        &texDesc, 
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
        nullptr, 
        IID_PPV_ARGS(&displayTexture));

    std::vector<uint8_t> colorBuffer;
    colorBuffer.resize(imageWidth* imageHeight * 4);

    // create vertex buffer
    ComPtr<ID3D12Resource> vertexBuffer;
    auto vertexBufferview = hvk::boiler::CreateVertexBuffer(
        device, 
        reinterpret_cast<const uint8_t*>(kVertices), 
        sizeof(kVertices), 
        sizeof(Vertex), vertexBuffer);

    // Camera setup
    const hvk::Camera camera(
            hvk::Vector(-1.5f, .6f, 0.8f),
            hvk::Vector(0.f, 0.f, -1.f),
            hvk::Vector(0.f, 1.f, 0.f),
            60.f,
            aspectRatio,
            0.001f,
            2.f);

    // World / Scene
    auto sphereEntity = registry.create();
    registry.emplace<hvk::Sphere>(sphereEntity, hvk::Vector(0.0f, 0.0f, -1.f), 0.5f);
    registry.emplace<hvk::Material>(sphereEntity, hvk::MaterialType::Diffuse, hvk::Color(1.f, 0.f, 0.f), -1.f);
    registry.assign<D3D12_RAYTRACING_AABB>(sphereEntity, hvk::aabb::getAABB(registry.get<hvk::Sphere>(sphereEntity)));

//    auto groundEntity = registry.create();
//    registry.emplace<hvk::Sphere>(groundEntity, hvk::Vector(0, -100.5f, -1.f), 100.f);
//    registry.emplace<hvk::Material>(groundEntity, hvk::MaterialType::Diffuse, hvk::Color(0.f, 1.f, 0.f), -1.f);

//    auto metalSphere1 = registry.create();
//    registry.emplace<hvk::Sphere>(metalSphere1, hvk::Vector(-1.0f, 0.f, -1.f), 0.5f);
//    registry.emplace<hvk::Material>(metalSphere1, hvk::MaterialType::Metal, hvk::Color(1.f, 1.f, 1.f));

    auto metalSphere2 = registry.create();
    registry.emplace<hvk::Sphere>(metalSphere2, hvk::Vector(1.0f, 0.f, -1.f), 0.5f);
    registry.emplace<hvk::Material>(metalSphere2, hvk::MaterialType::Metal, hvk::Color(1.f, 1.f, 1.f), -1.f);

    auto behindSphere = registry.create();
    registry.emplace<hvk::Sphere>(behindSphere, hvk::Vector(0.5f, 0.0f, 1.f), 0.5f);
    registry.emplace<hvk::Material>(behindSphere, hvk::MaterialType::Diffuse, hvk::Color(1.f, 0.f, 1.f), -1.f);

    auto glassSphere = registry.create();
    registry.emplace<hvk::Sphere>(glassSphere, hvk::Vector(2.f, 0.f, -1.f), 0.5f);
    registry.emplace<hvk::Material>(glassSphere, hvk::MaterialType::Dielectric, hvk::Color(0.8f, 0.8f, 0.8f), 1.5);

    auto smallGlass = registry.create();
    registry.emplace<hvk::Sphere>(smallGlass, hvk::Vector(0.3f, -.3f, -0.4f), 0.2f);
    registry.emplace<hvk::Material>(smallGlass, hvk::MaterialType::Dielectric, hvk::Color(1.f, 1.f, 1.f), 1.5);

    auto smallGlass2 = registry.create();
    registry.emplace<hvk::Sphere>(smallGlass2, hvk::Vector(-0.32f, -.3f, -0.42f), 0.2f);
    registry.emplace<hvk::Material>(smallGlass2, hvk::MaterialType::Dielectric, hvk::Color(1.f, 1.f, 1.f), 1.5);

    auto smallMetalSphere = registry.create();
    registry.emplace<hvk::Sphere>(smallMetalSphere, hvk::Vector(-0.68f, -.3f, -0.69f), 0.25f);
    registry.emplace<hvk::Material>(smallMetalSphere, hvk::MaterialType::Metal, hvk::Color(0.7f, 0.2f, 0.7f), -1.f);

    auto groundPlane = registry.create();
    registry.emplace<hvk::Plane>(groundPlane, hvk::Vector(0.f, -0.5f, 0.f), hvk::Vector(0.f, 1.f, 0.f));
    registry.emplace<hvk::Material>(groundPlane, hvk::MaterialType::Diffuse, hvk::Color(0.8f, 0.8f, 0.8f), -1.f);

    auto smallMetalBox = registry.create();
    registry.emplace<hvk::Box>(
            smallMetalBox,
            hvk::Plane(hvk::Vector(0.f, 0.5f, -4.f), hvk::Vector(0.f, 1.f, 0.f)), // top
            hvk::Plane(hvk::Vector(0.f, -0.5f, -4.f), hvk::Vector(0.f, -1.f, 0.f)), // bottom
            hvk::Plane(hvk::Vector(0.f, 0.f, -3.5f), hvk::Vector(0.f, 0.f, 1.f)), // front
            hvk::Plane(hvk::Vector(0.f, 0.f, -4.5f), hvk::Vector(0.f, 0.f, -1.f)), // back
            hvk::Plane(hvk::Vector(-0.5f, 0.f, -4.f), hvk::Vector(-1.f, 0.f, 0.f)), // left
            hvk::Plane(hvk::Vector(0.5f, 0.f, -4.f), hvk::Vector(1.f, 0.f, 0.f))); // right
    registry.emplace<hvk::Material>(smallMetalBox, hvk::MaterialType::Metal, hvk::Color(0.8f, 0.6f, 0.1f), -1.f);

    auto glassBox = registry.create();
    registry.emplace<hvk::Box>(
            glassBox,
            hvk::Plane(hvk::Vector(-0.5f, 0.0f, -2.5f), hvk::Vector(0.f, 1.f, 0.f)), // top
            hvk::Plane(hvk::Vector(-0.5f, -0.5f, -2.5f), hvk::Vector(0.f, -1.f, 0.f)), // bottom
            hvk::Plane(hvk::Vector(-0.5f, -0.25f, -2.f), hvk::Vector(0.f, 0.f, 1.f)), // front
            hvk::Plane(hvk::Vector(-0.5f, -0.25f, -2.5f), hvk::Vector(0.f, 0.f, -1.f)), // back
            hvk::Plane(hvk::Vector(-1.f, -0.25f, -2.5f), hvk::Vector(-1.f, 0.f, 0.f)), // left
            hvk::Plane(hvk::Vector(-0.5f, -0.25f, -2.5f), hvk::Vector(1.f, 0.f, 0.f))); // right
    registry.emplace<hvk::Material>(glassBox, hvk::MaterialType::Dielectric, hvk::Color(.9f, .9f, .9f), 1.5f);

//    auto diffuseBox = registry.create();
//    registry.emplace<hvk::Box>(
//            diffuseBox,
//            hvk::Plane(hvk::Vector(-1.5f, 1.f, -2.f), hvk::Vector(0.f, 1.f, 0.f)),
//            hvk::Plane(hvk::Vector(-1.5f, -0.5f, -2.f), hvk::Vector(0.f, -1.f, 0.f)),
//            hvk::Plane(hvk::Vector(-1.5f, 0.25f, -1.f), hvk::Vector(0.f, 0.f, 1.f)),
//            hvk::Plane(hvk::Vector(-1.5f, 0.25f, -3.f), hvk::Vector(0.f, 0.f, -1.f)),
//            hvk::Plane(hvk::Vector(-2.5f, 0.25f, -2.f), hvk::Vector(-1.f, 0.f, 0.f)),
//            hvk::Plane(hvk::Vector(-1.f, 0.25f, -2.f), hvk::Vector(1.f, 0.f, 0.f)));
//    registry.emplace<hvk::Material>(diffuseBox, hvk::MaterialType::Diffuse, hvk::Color(0.66, 0.2, 0.8));

    // auto metalBox = registry.create();
    // registry.emplace<hvk::Box>(
    //         metalBox,
    //         hvk::Plane(hvk::Vector(-1.5f, 1.f, -2.f), hvk::Vector(0.f, 1.f, 0.f)),
    //         hvk::Plane(hvk::Vector(-1.5f, -0.5f, -2.f), hvk::Vector(0.f, -1.f, 0.f)),
    //         hvk::Plane(hvk::Vector(-1.5f, 0.25f, 0.f), hvk::Vector(0.f, 0.f, 1.f)),
    //         hvk::Plane(hvk::Vector(-1.5f, 0.25f, -3.f), hvk::Vector(0.f, 0.f, -1.f)),
    //         hvk::Plane(hvk::Vector(-2.5f, 0.25f, -2.f), hvk::Vector(-1.f, 0.f, 0.f)),
    //         hvk::Plane(hvk::Vector(-1.f, 0.25f, -2.f), hvk::Vector(1.f, 0.f, 0.f)));
    // registry.emplace<hvk::Material>(metalBox, hvk::MaterialType::Metal, hvk::Color(.8f, .8f, .8f), -1.f);

    // Create DXR Structures
    ComPtr<ID3D12Resource> blas;
    ComPtr<ID3D12Resource> aabbBuffer;
    ComPtr<ID3D12Resource> aabbScratchBuffer;
    {
        ComPtr<ID3D12Resource> aabbCopyBuffer;
        const size_t unalignedSize = 20 * sizeof(D3D12_RAYTRACING_AABB);
        const size_t alignedSize = hvk::boiler::Align(unalignedSize, D3D12_RAYTRACING_AABB_BYTE_ALIGNMENT);
        hr = hvk::boiler::CreateBuffer(
            device,
            hvk::boiler::CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN),
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            alignedSize,
            D3D12_RESOURCE_FLAG_NONE,
            aabbCopyBuffer);
        assert(SUCCEEDED(hr));

        hr = hvk::boiler::CreateBuffer(
            device,
            hvk::boiler::CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN),
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            alignedSize,
            D3D12_RESOURCE_FLAG_NONE,
            aabbBuffer);

        uint8_t* bufferData;
        aabbCopyBuffer->Map(0, nullptr, reinterpret_cast<void**>(&bufferData));

        uint8_t* writeAt = bufferData;
        const auto aabbView = registry.view<hvk::Sphere, D3D12_RAYTRACING_AABB>();
        aabbView.each([writeAt](auto entity, const auto& sphere, const auto& aabb) mutable {
            memcpy(writeAt, &aabb, sizeof(D3D12_RAYTRACING_AABB));
            writeAt += hvk::boiler::Align(sizeof(D3D12_RAYTRACING_AABB), D3D12_RAYTRACING_AABB_BYTE_ALIGNMENT);
        });

        aabbCopyBuffer->Unmap(0, nullptr);

        commandList->Reset(commandAllocator.Get(), nullptr);
        hr = hvk::boiler::CopyBufferGPUImmediate(device, commandList, commandQueue, aabbCopyBuffer, aabbBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
        assert(SUCCEEDED(hr));
    }

    commandList->Reset(commandAllocator.Get(), nullptr);
    hr = hvk::boiler::CreateGeometryBLAS(device, commandList, aabbBuffer->GetGPUVirtualAddress(), { {{}} }, blas, aabbScratchBuffer);

    {
        commandList->Close();
        ID3D12CommandList* commandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
        hvk::boiler::WaitForGraphics(device, commandQueue);
    }

    ComPtr<ID3D12Resource> topScratch;
    ComPtr<ID3D12Resource> topAS;
    ComPtr<ID3D12Resource> topInstance;
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topInputs = topLevelBuildDesc.Inputs;
        topInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        topInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        topInputs.NumDescs = 1;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topPrebuild = {};
        device->GetRaytracingAccelerationStructurePrebuildInfo(&topInputs, &topPrebuild);
        assert(topPrebuild.ResultDataMaxSizeInBytes > 0);

        hvk::boiler::CreateBuffer(
            device, 
            hvk::boiler::CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN), 
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 
            topPrebuild.ScratchDataSizeInBytes, 
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            topScratch,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        assert(SUCCEEDED(hr));

        hvk::boiler::CreateBuffer(
            device, 
            hvk::boiler::CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN), 
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, 
            topPrebuild.ResultDataMaxSizeInBytes, 
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            topAS,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
        assert(SUCCEEDED(hr));

        hvk::boiler::CreateBuffer(
            device,
            hvk::boiler::CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN),
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
            D3D12_RESOURCE_FLAG_NONE,
            topInstance,
            D3D12_RESOURCE_STATE_GENERIC_READ);
        assert(SUCCEEDED(hr));

        // create TLAS instance
        D3D12_RAYTRACING_INSTANCE_DESC* instance;
        topInstance->Map(0, nullptr, reinterpret_cast<void**>(&instance));
        hvk::boiler::SetTLASInstanceValues(instance, 0, 0, XMMatrixIdentity()* XMMatrixTranslation(0.f, 0.f, -1.f), blas->GetGPUVirtualAddress());
        topInstance->Unmap(0, nullptr);

        topInputs.InstanceDescs = topInstance->GetGPUVirtualAddress();
        topLevelBuildDesc.ScratchAccelerationStructureData = topScratch->GetGPUVirtualAddress();
        topLevelBuildDesc.DestAccelerationStructureData = topAS->GetGPUVirtualAddress();

        // let's try figuring out what's required for this call from the top
        commandList->Reset(commandAllocator.Get(), nullptr);
        commandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

		{
			commandList->Close();
			ID3D12CommandList* commandLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
			hvk::boiler::WaitForGraphics(device, commandQueue);
		}
    }

	// Create thread pool
	hvk::ThreadPool pool(kNumThreads);

	// Render
	for (int i = imageHeight - 1; i >= 0; --i)
	{
		for (int j = 0; j < imageWidth; ++j)
		{
			pool.QueueWork([&, i, j]()
		   {
			   Color pixelColor(0.f, 0.f, 0.f);
			   auto result = std::make_optional(RayTestResult{});
			   for (size_t s = 0; s < kNumSamples; ++s)
			   {
				   auto u = static_cast<double>(j + hvk::math::getRandom<double, 0.0, 1.0>()) /
							(imageWidth - 1);
				   auto v = static_cast<double>(i + hvk::math::getRandom<double, 0.0, 1.0>()) /
							(imageHeight - 1);

				   hvk::Ray skyRay = camera.GetRay(u, v);
				   pixelColor += rayColor(skyRay, registry, kMaxRayDepth, result);
			   }
			   const size_t writeIndex = ((imageHeight - 1) - i) * imageWidth + j;
			   // const hvk::Color normalizedHit = 0.5f * hvk::Color(
			   //         result->hit.X() / viewportWidth + 1,
			   //         result->hit.Y() / viewportHeight + 1,
			   //         -result->hit.Z() / 1.5f);
			   writeOutBuffer[writeIndex] = (pixelColor / kNumSamples);
			   //depthBuffer[writeIndex] = (result->depth / kNumSamples);
			   //normalBuffer[writeIndex] = (result->normal / kNumSamples);
			   //reflectBuffer[writeIndex] = (result->reflect / kNumSamples);
			   // hitBuffer[writeIndex] = (normalizedHit / kNumSamples);
		   });
		}
	}

	const auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    uint8_t frameIndex = 0;
    bool running = true;
    MSG msg;
    while (running)
    {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT || msg.message == WM_CLOSE || msg.message == WM_DESTROY)
			{
				running = false;
			}
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

        auto framebuffer = renderTargets[frameIndex];

        hr = commandAllocator->Reset();
        hr = commandList->Reset(commandAllocator.Get(), nullptr);

        // prepare texture from current color contents
        for (size_t i = 0; i < imageWidth * imageHeight; ++i)
        {
            const auto& c = writeOutBuffer[i];

		    auto ir = static_cast<int>(255.999 * c.X());
		    auto ig = static_cast<int>(255.999 * c.Y());
		    auto ib = static_cast<int>(255.999 * c.Z());
            auto alpha = 255;

            const auto writeAt = i * 4;
            colorBuffer[writeAt] = ir;
            colorBuffer[writeAt + 1] = ig;
            colorBuffer[writeAt + 2] = ib;
            colorBuffer[writeAt + 3] = alpha;
        }
        hr = hvk::boiler::RGBAToTexture(device, commandList, commandQueue, colorBuffer, imageWidth, imageHeight, displayTexture);
        assert(SUCCEEDED(hr));

        hr = commandList->Reset(commandAllocator.Get(), pipelineState.Get());

        // set viewport / scissor
        D3D12_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(windowWidth);
        viewport.Height = static_cast<float>(windowHeight);
        viewport.TopLeftX = 0.f;
        viewport.TopLeftY = 0.f;
        viewport.MinDepth = 0.f;
        viewport.MaxDepth = 1.f;

        D3D12_RECT scissorRect = {};
        scissorRect.left = 0;
        scissorRect.right = windowWidth;
        scissorRect.top = 0;
        scissorRect.bottom = windowHeight;

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        // set root signature
        commandList->SetGraphicsRootSignature(rootSig.Get());

        // create resource views
        auto displayDesc = displayTexture->GetDesc();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = displayDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = displayDesc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        auto uavHandle = uavHeap->GetCPUDescriptorHandleForHeapStart();
        device->CreateShaderResourceView(displayTexture.Get(), &srvDesc, uavHandle);

        // set descriptor heaps
        ID3D12DescriptorHeap* heaps[] = { uavHeap.Get(), samplerHeap.Get() };
        commandList->SetDescriptorHeaps(_countof(heaps), heaps);

        // set root descriptor table
        commandList->SetGraphicsRootDescriptorTable(0, uavHeap->GetGPUDescriptorHandleForHeapStart());

        // transition present -> render target
        D3D12_RESOURCE_BARRIER backBuffer = {};
        backBuffer.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        //backBuffer.Transition.pResource = framebuffer.Get();
        backBuffer.Transition.pResource = renderTargets[frameIndex].Get();
        backBuffer.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        backBuffer.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        backBuffer.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        commandList->ResourceBarrier(1, &backBuffer);

        // set render target
        auto rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        D3D12_CPU_DESCRIPTOR_HANDLE rtvDesc = {};
        rtvDesc.ptr = rtvHandle.ptr + (descriptorSize * frameIndex);
        commandList->OMSetRenderTargets(1, &rtvDesc, false, nullptr);
        commandList->ClearRenderTargetView(rtvDesc, kClearColor, 0, nullptr);

        // set topology
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // set vertex buffer
        commandList->IASetVertexBuffers(0, 1, &vertexBufferview);

        // draw
        commandList->DrawInstanced(kNumVertices, 1, 0, 0);

        // transition render target -> present
        D3D12_RESOURCE_BARRIER present = {};
        present.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        //present.Transition.pResource = framebuffer.Get();
        present.Transition.pResource = renderTargets[frameIndex].Get();
        present.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        present.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        present.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &present);

        // execute command list
        hr = commandList->Close();
        ID3D12CommandList* commandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        // present
        hr = swapchain->Present(1, 0);

        hvk::boiler::WaitForGraphics(device, commandQueue);

        // get next swapchain index
        frameIndex = swapchain->GetCurrentBackBufferIndex();
    }

    //writeBuffers(
    //    writeOutBuffer,
    //    imageWidth,
    //    imageHeight,
    //    std::make_optional(depthBuffer),
    //    std::make_optional(normalBuffer),
    //    std::make_optional(reflectBuffer),
    //    std::nullopt);
    //    // std::make_optional(hitBuffer));

    return 0;
}
