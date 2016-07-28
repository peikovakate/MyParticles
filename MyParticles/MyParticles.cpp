// MyParticles.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MyParticles.h"
#include <windows.h>
#include <d3d11_1.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "D3Dcompiler.h"
#include <d3d11shader.h>
#include <d3d.h>
#include "D3Dcommon.h"
#include "inc\d3dx11effect.h"
#include <cmath>
#include <ctime>
#include <windows.h>    // included for Windows Touch
#include <windowsx.h>   // included for point conversion
#include <string>
#define MAXPOINTS 50
struct Pointer {
	float x;
	float y;
};
// to track touch points
Pointer points[MAXPOINTS];
UINT cInputs;
PTOUCHINPUT pInputs;
POINT ptInput;
int touchCount = 0;

const int SensivityMode1 = 1;
const int SensivityMode2 = -6;

using namespace DirectX;
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11DeviceContext*	g_immediateContext;
HRESULT hr;

//for particleSolver
//---------------------------------------------------
//out
ID3D11UnorderedAccessView *g_particlesUAV;
ID3DX11EffectUnorderedAccessViewVariable *g_particlesUAVVariavle;

ID3DX11Effect *g_ParticleSolverEffect;
ID3DX11EffectTechnique *g_ParticleSolverTechnique;

//in
ID3DX11EffectShaderResourceVariable *g_computedParticlesBufferResourse;
ID3D11ShaderResourceView* g_computedParticlesBufferView;

ID3DX11EffectScalarVariable *g_Sensivity;
ID3DX11EffectScalarVariable *g_maxParticles;
ID3DX11EffectScalarVariable *g_pointersCount;

ID3DX11EffectShaderResourceVariable *g_pointersBufferResourse;
ID3D11ShaderResourceView* g_pointersBufferView;

//buffers
ID3D11Buffer *g_particlesBufferIN;
ID3D11Buffer *g_particlesBufferOUT;
ID3D11Buffer *g_pointersBuffer;


//particleRender 
//------------------------------------
ID3DX11Effect *g_ParticleRenderEffect;
ID3DX11EffectTechnique *g_ParticleRenderTechnique;
ID3DX11EffectSamplerVariable *g_particleSampler;
ID3D11Texture2D *g_texture;
ID3DX11EffectShaderResourceVariable *g_renderTexture;
ID3D11SamplerState *g_samplerState;
//ID3D11InputLayout *g_vertexLayout; ????
ID3D11Resource *g_textureResource;
ID3D11ShaderResourceView *g_textureView;
//in
ID3DX11EffectShaderResourceVariable *g_ParticlesBufferResourse;
ID3D11ShaderResourceView* g_ParticlesBufferView;
//------------------------------------

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void CleanupDevice();
void Render();
void createSetEffect();
HRESULT InitDevice();

int counter = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYPARTICLES, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYPARTICLES));
    MSG msg;

  //  // Main message loop:
  //  while (GetMessage(&msg, nullptr, 0, 0))
  //  {
  //      if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
  //      {
  //          TranslateMessage(&msg);
  //          DispatchMessage(&msg);
  //      }
		//else
		//{
		//	Render();
		//}
  //  }

  //  return (int) msg.wParam;
	msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
	CleanupDevice();
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYPARTICLES));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MYPARTICLES);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  /* g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   0, 0, 3840, 2160, nullptr, nullptr, hInstance, nullptr);*/

   if (!g_hWnd)
   {
      return FALSE;
   }


   // register the window for touch instead of gestures
   RegisterTouchWindow(g_hWnd, 0);

   // the following code initializes the points
   for (int i = 0; i< MAXPOINTS; i++) {
	   points[i].x = -1;
	   points[i].y = -1;
   }


   ShowWindow(g_hWnd, nCmdShow);
  // UpdateWindow(g_hWnd);

   return TRUE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_TOUCH:
		cInputs = LOWORD(wParam);
		touchCount = cInputs;
		pInputs = new TOUCHINPUT[cInputs];
		if (pInputs) {
			if (GetTouchInputInfo((HTOUCHINPUT)lParam, cInputs, pInputs, sizeof(TOUCHINPUT))) {
				for (int i = 0; i < static_cast<INT>(cInputs); i++) {
					TOUCHINPUT ti = pInputs[i];
					if (ti.dwID != 0) {
						ptInput.x = TOUCH_COORD_TO_PIXEL(ti.x);
						ptInput.y = TOUCH_COORD_TO_PIXEL(ti.y);
						ScreenToClient(hWnd, &ptInput);
						RECT window;
						GetWindowRect(hWnd, &window);
						if (ti.dwFlags & TOUCHEVENTF_UP) {
							points[i].x = 10000;
							points[i].y = 10000;
						}
						else {
							points[i].x = ((float)ptInput.x / (window.right - window.left) - 0.5) * 2;
							points[i].y = -((float)ptInput.y / (window.bottom - window.top) - 0.5) * 2;
						}

						
					}
				}
			}
			CloseTouchInputHandle((HTOUCHINPUT)lParam);
			delete[] pInputs;
		}
		else {
			// Handle the error here 
		}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain1) g_pSwapChain1->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice1) g_pd3dDevice1->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
	if (g_computedParticlesBufferResourse)g_computedParticlesBufferResourse->Release();
	if (g_computedParticlesBufferView)g_computedParticlesBufferView->Release();
	if (g_particlesBufferIN)g_particlesBufferIN->Release();
	if (g_particlesBufferOUT)g_particlesBufferOUT->Release();
	if (g_particlesUAV)g_particlesUAV->Release();
	if (g_particlesUAVVariavle)g_particlesUAVVariavle->Release();
	if (g_pointersBuffer)g_pointersBuffer->Release();

}

const FLOAT clearColor[4] = { 0, 0, 0, 1 };
const int PARTICLES_COUNT = 200000;  

struct Particle {
	XMFLOAT3 Position;
	XMFLOAT3 Color;
	XMFLOAT3 StartPosition;
};
Particle particlesArray[PARTICLES_COUNT];
float Sensivity = SensivityMode1;

void loadSamplerAndTexture() {
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_samplerState);
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Particle16x16.dds", &g_textureResource, &g_textureView);
}
void createInBuffer() {
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = sizeof(Particle) * PARTICLES_COUNT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(Particle);
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = particlesArray;
	hr = g_pd3dDevice->CreateBuffer(&desc, &InitData, &g_particlesBufferIN);
}
void createOutBuffer() {
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = sizeof(Particle) * PARTICLES_COUNT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(Particle);
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = particlesArray; // maybe null ???
	hr = g_pd3dDevice->CreateBuffer(&desc, &InitData, &g_particlesBufferOUT);
}
void CreateComputeBufferSRV() {
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	g_particlesBufferIN->GetDesc(&descBuf);
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	hr = g_pd3dDevice->CreateShaderResourceView(g_particlesBufferIN, &desc, &g_computedParticlesBufferView);
}
void CreateRenderBufferSRV() {
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	g_particlesBufferIN->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	hr = g_pd3dDevice->CreateShaderResourceView(g_particlesBufferIN, &desc, &g_ParticlesBufferView);
}
void CreateBufferUAV() {
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	g_particlesBufferOUT->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	hr = g_pd3dDevice->CreateUnorderedAccessView(g_particlesBufferOUT, &desc, &g_particlesUAV);
}
void createPointerBuffer() {
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = sizeof(Pointer) * MAXPOINTS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(Pointer);
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = points;
	hr = g_pd3dDevice->CreateBuffer(&desc, &InitData, &g_pointersBuffer);
}
void createPointerBufferSRV() {
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	g_pointersBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	hr = g_pd3dDevice->CreateShaderResourceView(g_pointersBuffer, &desc, &g_pointersBufferView);
}

void createSetEffect() {
	hr = D3DX11CompileEffectFromFile(L"ParticleSolver.fx", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		NULL, 0, g_pd3dDevice, &g_ParticleSolverEffect, nullptr);
	g_ParticleSolverTechnique = g_ParticleSolverEffect->GetTechniqueByName("ParticleSolver");

	hr = D3DX11CompileEffectFromFile(L"ParticleRender.fx", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, NULL, g_pd3dDevice, &g_ParticleRenderEffect, nullptr);
	g_ParticleRenderTechnique = g_ParticleRenderEffect->GetTechniqueByName("ParticleRender");

	//initializing starting position for particles
	int Nx = sqrt(16 * PARTICLES_COUNT / 9);
	float xStep = 1.0 / Nx;
	for (int i = 0; i < PARTICLES_COUNT; i++) {
		particlesArray[i].Position = XMFLOAT3((i%Nx)*xStep * 2 - 1, (i / Nx)*xStep * 2 * 16 / 9 - 1, 0);
		particlesArray[i].StartPosition = particlesArray[i].Position;
		particlesArray[i].Color = XMFLOAT3(sinf((i%Nx)*xStep * 2 - 1), cosf((i%Nx)*xStep * 2 - 1), 1);
	}

	//setting solver effect
	g_computedParticlesBufferResourse = (g_ParticleSolverEffect->GetVariableByName("ParticlesIN"))->AsShaderResource();
	g_particlesUAVVariavle = (g_ParticleSolverEffect->GetVariableByName("ParticlesOUT"))->AsUnorderedAccessView();
	g_maxParticles = (g_ParticleSolverEffect->GetVariableByName("MaxParticles"))->AsScalar();
	g_pointersCount = (g_ParticleSolverEffect->GetVariableByName("PointersCount"))->AsScalar();
	g_Sensivity = (g_ParticleSolverEffect->GetVariableByName("Sensivity"))->AsScalar();
	g_pointersBufferResourse = (g_ParticleSolverEffect->GetVariableByName("Pointers"))->AsShaderResource();

	//setting render effect
	g_particleSampler = (g_ParticleRenderEffect->GetVariableByName("ParticleSampler"))->AsSampler();
	g_ParticlesBufferResourse = (g_ParticleRenderEffect->GetVariableByName("Particles"))->AsShaderResource();
	g_renderTexture = (g_ParticleRenderEffect->GetVariableByName("ParticleTexture"))->AsShaderResource();

	createInBuffer();
	createOutBuffer();
	createPointerBuffer();
	CreateComputeBufferSRV();
	CreateRenderBufferSRV();
	CreateBufferUAV();
	createPointerBuffer();
	createPointerBufferSRV();

	loadSamplerAndTexture();

	hr = g_particleSampler->SetSampler(0, g_samplerState);
	hr = g_renderTexture->SetResource(g_textureView);
	hr = g_Sensivity->SetFloat(Sensivity);
	hr = g_maxParticles->SetInt(PARTICLES_COUNT);
}

ID3D11Buffer* tempBuf;

void Update() {
	counter++;
	if (counter == 2000) {
		counter = 0;
		Sensivity = (Sensivity == SensivityMode1) ? SensivityMode2 : SensivityMode1;
		hr = g_Sensivity->SetFloat(Sensivity);
	}

	g_pImmediateContext->UpdateSubresource(g_pointersBuffer, 0, NULL, points, MAXPOINTS * sizeof(Pointer), 0);
	hr = g_pointersCount->SetInt(touchCount);
	hr = g_computedParticlesBufferResourse->SetResource(g_computedParticlesBufferView);
	hr = g_particlesUAVVariavle->SetUnorderedAccessView(g_particlesUAV);
	hr = g_pointersBufferResourse->SetResource(g_pointersBufferView);

	ID3DX11EffectPass* g_pass = g_ParticleSolverTechnique->GetPassByIndex(0);
	hr = g_pass->Apply(NULL, g_pImmediateContext);
	g_pImmediateContext->Dispatch(PARTICLES_COUNT, 1, 1);

	tempBuf = nullptr;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	g_particlesBufferOUT->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	g_pd3dDevice->CreateBuffer(&desc, nullptr, &tempBuf);
	g_pImmediateContext->CopyResource(tempBuf, g_particlesBufferOUT);

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	Particle *p;
	g_pImmediateContext->Map(tempBuf, 0, D3D11_MAP_READ, 0, &MappedResource);
	p = (Particle*)MappedResource.pData;

	g_pImmediateContext->UpdateSubresource(g_particlesBufferIN, 0, NULL, p, PARTICLES_COUNT * sizeof(Particle), 0);
	tempBuf->Release();

	//particle render
	hr = g_ParticlesBufferResourse->SetResource(g_ParticlesBufferView);
	ID3DX11EffectPass* g_passRender = g_ParticleRenderTechnique->GetPassByIndex(0);
	hr = g_passRender->Apply(NULL, g_pImmediateContext);
	g_pImmediateContext->Draw(PARTICLES_COUNT, 0);
	
}
void Render()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	Update();

	g_pSwapChain->Present(1, 0);
}


HRESULT InitDevice()
	{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;


	UINT createDeviceFlags = 0;
//#ifdef _DEBUG
//	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			 //DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	//ID3DBlob *effect; 
	//ID3D10Effect *eff;
	//auto h = D3DCompileFromFile(L"ParticleRender.fx", NULL, NULL, NULL, "fx_5_0", NULL, NULL, &effect, NULL);

	createSetEffect();

	return S_OK;
}
