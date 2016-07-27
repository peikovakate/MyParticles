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


// You will use this array to track touch points
Pointer points[MAXPOINTS];


UINT cInputs;
PTOUCHINPUT pInputs;
POINT ptInput;
int idLookup[MAXPOINTS];
int touchCount = 0;
// For tracking dwId to points
int index;


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
XMMATRIX	g_world;

HRESULT hr;


//particleSolver
//------------------------------------
ID3DX11Effect *g_ParticleSolverEffect;
ID3DX11EffectTechnique *g_ParticleSolverTechnique;
ID3DX11EffectScalarVariable *g_groupDim;
ID3DX11EffectScalarVariable *g_sensivity;
ID3DX11EffectScalarVariable *g_maxParticles;
ID3DX11EffectScalarVariable *g_pointersCount;
ID3DX11EffectVectorVariable *g_attractor;
ID3DX11EffectConstantBuffer *g_pointers;

ID3D11Buffer *solverUAVParticles;
ID3D11UnorderedAccessView *g_uav;
ID3DX11EffectUnorderedAccessViewVariable *g_particlesUAV;
//------------------------------------

//particleRender 
//------------------------------------
ID3DX11Effect *g_ParticleRenderEffect;
ID3DX11EffectTechnique *g_ParticleRenderTechnique;
ID3D11Buffer *solverParticles;
ID3DX11EffectMatrixVariable *g_view;
ID3DX11EffectMatrixVariable *g_projection;

ID3DX11EffectSamplerVariable *g_particleSampler;
ID3D11Texture2D *g_texture;
ID3DX11EffectShaderResourceVariable *g_renderTexture;
ID3D11SamplerState *g_samplerState;
ID3D11InputLayout *g_vertexLayout;

ID3DX11EffectShaderResourceVariable *g_particlesStructuredBuffer;
ID3D11ShaderResourceView* g_particlesStructuredBufferView;
ID3D11Resource *g_textureResource;
ID3D11ShaderResourceView *g_textureView;

//------------------------------------

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void CleanupDevice();
void Render();
HRESULT InitDevice();


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYPARTICLES, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
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

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
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

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   //g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   0, 0, 3840, 2160, nullptr, nullptr, hInstance, nullptr);

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
	   idLookup[i] = -1;
   }


   ShowWindow(g_hWnd, nCmdShow);
  // UpdateWindow(g_hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
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
						OutputDebugString(std::to_wstring(ptInput.x).c_str());
						points[i].x = ((float)ptInput.x /(window.right - window.left)-0.5)*2;
						points[i].y = -((float)ptInput.y/(window.bottom-window.top)-0.5)*2;
						OutputDebugString(std::to_wstring(points[i].x).c_str());
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

// Message handler for about box.
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
}

const FLOAT clearColor[4] = { 0, 0, 0, 1 };

const int PARTICLES_COUNT = 600000;  

int _threadSize;
struct Particle {
	XMFLOAT3 Position;
	XMFLOAT3 Velocity;
	XMFLOAT3 StartPosition;

};
Particle initialParticles[PARTICLES_COUNT];
float Sensivity = 6;
float theta = 0;
float distance = 0;
XMFLOAT3 p;
void Update() {
	//
	//POINT p;
	//GetCursorPos(&p);
	//hr = g_groupDim->SetFloat(_threadSize);
	//hr = g_maxParticles->SetInt(PARTICLES_COUNT);
	//hr = g_sensivity->SetFloat(6);
	//hr = g_pointersCount->SetInt(touchCount);
	//
	//ID3DX11EffectPass* g_pass = g_ParticleSolverTechnique->GetPassByIndex(0);
	//hr = g_pass->Apply(NULL, g_pImmediateContext);
	//g_pImmediateContext->Dispatch(_threadSize, _threadSize, 1);
	theta = 0;
	distance = 0;
	
	for (int k = 0; k < PARTICLES_COUNT; k++) {
		p = initialParticles[k].Position;
		for (int j = 0; j <touchCount; j++) {
			theta = atan2(p.y - points[j].y, p.x - points[j].x);
			distance = Sensivity * 0.001 / sqrt((points[j].x - p.x)*(points[j].x - p.x)
				+ (points[j].y - p.y)*(points[j].y - p.y));
			p.x += (cos(theta)*distance + (initialParticles[k].StartPosition.x - p.x)*0.05);
			p.y += (sin(theta)*distance + (initialParticles[k].StartPosition.y - p.y)*0.05);
		}
		
		initialParticles[k].Position = p;
	}

	//creating buffer for initial particles
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(Particle)*PARTICLES_COUNT;
	cbDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	//cbDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS & D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	cbDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	cbDesc.StructureByteStride = sizeof(Particle);

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = &initialParticles;
	InitData.SysMemPitch = sizeof(Particle)*PARTICLES_COUNT;
	InitData.SysMemSlicePitch = 0;

	hr = g_pd3dDevice->CreateBuffer(&cbDesc, &InitData, &solverParticles);
	D3D11_SHADER_RESOURCE_VIEW_DESC svDesc = {};
	svDesc.Buffer.NumElements = PARTICLES_COUNT;
	svDesc.Buffer.FirstElement = 0;
	svDesc.Format = DXGI_FORMAT_UNKNOWN;
	svDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

	hr = g_pd3dDevice->CreateShaderResourceView(solverParticles, &svDesc, &g_particlesStructuredBufferView);

	//particle render
	hr = g_particleSampler->SetSampler(0, g_samplerState);
	hr = g_renderTexture->SetResource(g_textureView);
	hr = g_particlesStructuredBuffer->SetResource(g_particlesStructuredBufferView);
	
	//D3D11_INPUT_ELEMENT_DESC layout[] =
	//{
	//	{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};
	//UINT numElements = ARRAYSIZE(layout);

	//D3DX11_PASS_DESC PassDesc;
	//hr = g_ParticleRenderTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	//hr = g_pd3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
	//	PassDesc.IAInputSignatureSize, &g_vertexLayout);
	//
	//g_pImmediateContext->IASetInputLayout(g_vertexLayout);

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



//
//struct Particle {
//	XMFLOAT3 Position;
//	XMFLOAT3 Velocity;
//	XMFLOAT3 StartPosition;
//};




void createSetEffect() {

	srand(time(0));

	hr = D3DX11CompileEffectFromFile(L"ParticleSolver.fx", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		NULL, 0, g_pd3dDevice, &g_ParticleSolverEffect, nullptr);
	g_ParticleSolverTechnique = g_ParticleSolverEffect->GetTechniqueByName("ParticleSolver");
	D3DX11_EFFECT_DESC d;
	auto gr = g_ParticleSolverEffect->GetDesc(&d);
	//g_ParticleSolverTechnique = g_ParticleSolverEffect->GetTechniqueByIndex(0);
	//bool b = g_ParticleSolverTechnique->IsValid();

	hr = D3DX11CompileEffectFromFile(L"ParticleRender.fx", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
		D3DCOMPILE_DEBUG 
		, NULL, g_pd3dDevice, &g_ParticleRenderEffect, nullptr);
	g_ParticleRenderTechnique = g_ParticleRenderEffect->GetTechniqueByName("ParticleRender");

	//ID3DX11EffectTechnique

	int numGroups = 0;

	if (PARTICLES_COUNT % 768 != 0)
	{
		numGroups = (PARTICLES_COUNT / 768) + 1;
	}
	else
	{
		numGroups = PARTICLES_COUNT / 768;
	}

	double thirdRoot = pow((double)numGroups, (double)(1.0 / 2.0));
	thirdRoot = ceil(thirdRoot);
	_threadSize = _threadSize = _threadSize = (int)thirdRoot;


	

	int n = 34;
	int k = n / 2;

	for (int i = 0; i < PARTICLES_COUNT; i++) {
		//initialParticles[i].Position = XMFLOAT3((i % n - k)*100, (i / n - k)*100, 0);
		initialParticles[i].Position = XMFLOAT3(((float)rand()/(RAND_MAX)-0.5)*2, ((float)rand() / (RAND_MAX)-0.5) * 2, 0);
		initialParticles[i].StartPosition = initialParticles[i].Position;
		initialParticles[i].Velocity = XMFLOAT3(sinf(i / PARTICLES_COUNT), cosf(i / PARTICLES_COUNT), 0);

		//initialParticles[i].Position *= random.NextFloat(1f, 100f);
		
		float l = initialParticles[i].Position.x*initialParticles[i].Position.x+ initialParticles[i].Position.y*initialParticles[i].Position.y;
		float angle = -atan2f(initialParticles[i].Position.x, initialParticles[i].Position.z);

		initialParticles[i].Velocity = XMFLOAT3(cosf(angle), 0, sinf(angle)); //*5
	}


	////creating buffer for initial particles
	//D3D11_BUFFER_DESC cbDesc = {};
	//cbDesc.ByteWidth = sizeof(Particle)*PARTICLES_COUNT;
	//cbDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	////cbDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS & D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	//cbDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	//cbDesc.CPUAccessFlags = 0;
	//cbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//cbDesc.StructureByteStride = sizeof(Particle);

	//D3D11_BUFFER_DESC cbuavDesc = {};
	//cbuavDesc.ByteWidth = sizeof(Particle)*PARTICLES_COUNT;
	//cbuavDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	////cbuavDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS & D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	//cbuavDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
	//cbuavDesc.CPUAccessFlags = 0;
	//cbuavDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//cbuavDesc.StructureByteStride = sizeof(Particle);

	//// Fill in the subresource data.
	//D3D11_SUBRESOURCE_DATA InitData = {};
	//InitData.pSysMem = &initialParticles;
	//InitData.SysMemPitch = sizeof(Particle)*PARTICLES_COUNT;
	//InitData.SysMemSlicePitch = 0;

	//hr = g_pd3dDevice->CreateBuffer(&cbDesc, &InitData, &solverParticles);
	//hr = g_pd3dDevice->CreateBuffer(&cbuavDesc, &InitData, &solverUAVParticles);

	//D3D11_UNORDERED_ACCESS_VIEW_DESC uvDesc = {};
	//uvDesc.Buffer.FirstElement = 0;
	//uvDesc.Buffer.NumElements = PARTICLES_COUNT;
	//uvDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
	//uvDesc.Format = DXGI_FORMAT_UNKNOWN;
	//uvDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	//D3D11_SHADER_RESOURCE_VIEW_DESC svDesc = {};
	//svDesc.Buffer.NumElements = PARTICLES_COUNT;
	//svDesc.Buffer.FirstElement = 0;
	//svDesc.Format = DXGI_FORMAT_UNKNOWN;
	//svDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//

	//hr = g_pd3dDevice->CreateUnorderedAccessView(solverUAVParticles, &uvDesc, &g_uav);
	//hr = g_pd3dDevice->CreateShaderResourceView(solverParticles, &svDesc, &g_particlesStructuredBufferView);
	//solverParticles->Release();
	////setting solver shader
	//g_groupDim = (g_ParticleSolverEffect->GetVariableByName("GroupDim"))->AsScalar();
	//g_maxParticles = (g_ParticleSolverEffect->GetVariableByName("MaxParticles"))->AsScalar();
	//g_sensivity = (g_ParticleSolverEffect->GetVariableByName("Sensivity"))->AsScalar();
	////g_pointers = (g_ParticleSolverEffect->GetVariableByName("Pointers"))->AsConstantBuffer();
	//g_pointersCount = (g_ParticleSolverEffect->GetVariableByName("PointersCount"))->AsScalar();
	//g_particlesUAV = (g_ParticleSolverEffect->GetVariableByName("Particles"))->AsUnorderedAccessView();


	//setting render shader
	g_view = (g_ParticleRenderEffect->GetVariableByName("View"))->AsMatrix();
	g_projection = (g_ParticleRenderEffect->GetVariableByName("Projection"))->AsMatrix();
	g_particleSampler = (g_ParticleRenderEffect->GetVariableByName("ParticleSampler"))->AsSampler();
	g_particlesStructuredBuffer = (g_ParticleRenderEffect->GetVariableByName("Particles"))->AsShaderResource();
	g_renderTexture = (g_ParticleRenderEffect->GetVariableByName("ParticleTexture"))->AsShaderResource();

	//creating sampler state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_samplerState);

	//loading texture
	//hr = CreateWICTextureFromFile(g_pd3dDevice, g_immediateContext, L"Particle16x16.dds", &g_textureResource, &g_textureView);
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Particle16x16.dds", &g_textureResource,&g_textureView);
	//hr = g_particlesUAV->SetUnorderedAccessView(g_uav);

	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	//g_world = XMMatrixIdentity();
	//g_Camera.SetViewParams(s_Eye, s_At);	
}



//D3DX11_PASS_DESC PassDesc;
//V_RETURN(g_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc));
//V_RETURN(pd3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
//	PassDesc.IAInputSignatureSize, &g_pVertexLayout));
//
//// Set the input layout
//pd3dImmediateContext->IASetInputLayout(g_pVertexLayout);
//
//// Load the mesh
//V_RETURN(g_Mesh.Create(pd3dDevice, L"Tiny\\tiny.sdkmesh"));
//
//// Initialize the world matrices
//g_World = XMMatrixIdentity();
//
//// Setup the camera's view parameters
//static const XMVECTORF32 s_Eye = { 0.0f, 3.0f, -800.0f, 0.f };
//static const XMVECTORF32 s_At = { 0.0f, 1.0f, 0.0f, 0.f };
//g_Camera.SetViewParams(s_Eye, s_At);



HRESULT InitDevice()
	{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	//UINT width = rc.right - rc.left;
	//UINT height = rc.bottom - rc.top;
	UINT width = 3840;
	UINT height = 2160;

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
