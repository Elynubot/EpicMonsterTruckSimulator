#include "StdAfx.h"
#include "resource.h"
#include "DispositifD3D11.h"
#include "Util.h"
#include "InfoDispositif.h"

using namespace UtilitairesDX;

namespace PM3D
{
	D3D11_FILL_MODE CDispositifD3D11::fillMode = D3D11_FILL_SOLID;

	CDispositifD3D11::CDispositifD3D11(void)
	{
	}

	CDispositifD3D11::~CDispositifD3D11(void)
	{
		pSwapChain->SetFullscreenState(FALSE, NULL); // passer en mode fen�tr�

		DXRelacher(mSolidCullBackRS);
		DXRelacher(pDepthTexture);
		DXRelacher(pDepthStencilView);

		if (pImmediateContext) pImmediateContext->ClearState();
		DXRelacher(pRenderTargetView);
		DXRelacher(pImmediateContext);
		DXRelacher(pSwapChain);
		DXRelacher(pD3DDevice);
	}

	void CDispositifD3D11::SetRenderTargetView(ID3D11RenderTargetView* pRenderTargetView_in, ID3D11DepthStencilView* pDepthStencilView_in)
	{
		pRenderTargetView = pRenderTargetView_in; 
		pDepthStencilView = pDepthStencilView_in; 
		ID3D11RenderTargetView* tabRTV[1]; 
		tabRTV[0] = pRenderTargetView; 
		pImmediateContext->OMSetRenderTargets(1, tabRTV, pDepthStencilView);
	}

	//  FONCTION : CDispositifD3D11, constructeur param�tr� 
	//  BUT :	Constructeur de notre classe sp�cifique de dispositif 
	//  PARAM�TRES:		
	//		cdsMode:	CDS_FENETRE application fen�tr�e
	//					CDS_PLEIN_ECRAN application plein �cran
	//
	//		hWnd:	Handle sur la fen�tre Windows de l'application,
	//    	n�cessaire pour la fonction de cr�ation du 
	//				dispositif
	CDispositifD3D11::CDispositifD3D11(const CDS_MODE cdsMode,
		const HWND hWnd)
	{

		UINT largeur;
		UINT hauteur;
		UINT createDeviceFlags = 0;

#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		pImmediateContext = NULL;
		pSwapChain = NULL;
		pRenderTargetView = NULL;

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));

		// Obtenir les informations de l'adaptateur de d�faut
		CInfoDispositif Dispo0(ADAPTATEUR_COURANT);

		largeur = 1024;
		hauteur = 768;

		switch (cdsMode)
		{
		case CDS_FENETRE:
			sd.Windowed = TRUE;

			break;

		case CDS_PLEIN_ECRAN:
			sd.Windowed = FALSE;
			break;

		}

		DXGI_MODE_DESC  desc;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Height = hauteur;
		desc.Width = largeur;
		desc.RefreshRate.Numerator = 60;
		desc.RefreshRate.Denominator = 1;
		desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		CInfoDispositif DispoVoulu(desc);
		DispoVoulu.GetDesc(desc);

		largeur = desc.Width;
		hauteur = desc.Height;
		largeurEcran = float(largeur);
		hauteurEcran = float(hauteur);

		sd.BufferCount = 1;
		sd.BufferDesc.Width = desc.Width;
		sd.BufferDesc.Height = desc.Height;
		sd.BufferDesc.Format = desc.Format;
		sd.BufferDesc.RefreshRate.Numerator = desc.RefreshRate.Numerator;
		sd.BufferDesc.RefreshRate.Denominator = desc.RefreshRate.Denominator;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Permettre l'�change plein �cran

		// r�gler le probl�me no 1 du passage en mode fen�tr�
		RECT rcClient, rcWindow;
		POINT ptDiff;
		GetClientRect(hWnd, &rcClient);
		GetWindowRect(hWnd, &rcWindow);
		ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
		ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
		MoveWindow(hWnd, rcWindow.left, rcWindow.top, largeur + ptDiff.x, hauteur + ptDiff.y, TRUE);

		DXEssayer(D3D11CreateDeviceAndSwapChain(0,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			createDeviceFlags,
			featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION,
			&sd,
			&pSwapChain,
			&pD3DDevice,
			NULL,
			&pImmediateContext),
			DXE_ERREURCREATIONDEVICE);

		// Cr�ation d'un �render target view�
		ID3D11Texture2D *pBackBuffer;
		DXEssayer(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer), DXE_ERREUROBTENTIONBUFFER);

		DXEssayer(pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView), DXE_ERREURCREATIONRENDERTARGET);
		pBackBuffer->Release();

		InitDepthBuffer();

		// Initialiser les �tats de m�lange (blending states)
		InitBlendStates();

		pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)largeur;
		vp.Height = (FLOAT)hauteur;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		pImmediateContext->RSSetViewports(1, &vp);

		// Cr�ation et initialisation des �tats
		// 1. Culling
		D3D11_RASTERIZER_DESC rsDesc;
		ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_BACK;
		rsDesc.FrontCounterClockwise = false;
		pD3DDevice->CreateRasterizerState(&rsDesc, &mSolidCullBackRS);

		ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.FrontCounterClockwise = false;
		pD3DDevice->CreateRasterizerState(&rsDesc, &mSolidCullNoneRS);
		pImmediateContext->RSSetState(mSolidCullBackRS);

		// 2. Z-Buffer
		pImmediateContext->OMGetDepthStencilState(&mZBufferActif, 0);
		D3D11_DEPTH_STENCIL_DESC dsd;
		if (mZBufferActif)
			mZBufferActif->GetDesc(&dsd);
		else
			ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dsd.DepthEnable = false;
		dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsd.DepthFunc = D3D11_COMPARISON_LESS;
		pD3DDevice->CreateDepthStencilState(&dsd, &mZBufferInactif);

	}

	void CDispositifD3D11::PresentSpecific()
	{
		pSwapChain->Present(0, 0);
	}

	void CDispositifD3D11::InitDepthBuffer()
	{
		D3D11_TEXTURE2D_DESC depthTextureDesc;
		ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
		depthTextureDesc.Width = largeurEcran;
		depthTextureDesc.Height = hauteurEcran;
		depthTextureDesc.MipLevels = 1;
		depthTextureDesc.ArraySize = 1;
		depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTextureDesc.SampleDesc.Count = 1;
		depthTextureDesc.SampleDesc.Quality = 0;
		depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTextureDesc.CPUAccessFlags = 0;
		depthTextureDesc.MiscFlags = 0;

		DXEssayer(pD3DDevice->CreateTexture2D(&depthTextureDesc, NULL, &pDepthTexture), DXE_ERREURCREATIONTEXTURE);

		// Cr�ation de la vue du tampon de profondeur (ou de stencil)
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
		ZeroMemory(&descDSView, sizeof(descDSView));
		descDSView.Format = depthTextureDesc.Format;
		descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSView.Texture2D.MipSlice = 0;
		DXEssayer(pD3DDevice->CreateDepthStencilView(pDepthTexture, &descDSView, &pDepthStencilView), DXE_ERREURCREATIONDEPTHSTENCILTARGET);

	}

	void CDispositifD3D11::SetViewPortDimension(float largeur_in, float
		hauteur_in)
	{
		D3D11_VIEWPORT vp;
		vp.Width = largeur_in;
		vp.Height = hauteur_in;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		pImmediateContext->RSSetViewports(1, &vp);
	}
	void CDispositifD3D11::ResetViewPortDimension()
	{
		D3D11_VIEWPORT vp;
		vp.Width = largeurEcran;
		vp.Height = hauteurEcran;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		pImmediateContext->RSSetViewports(1, &vp);
	}
	void CDispositifD3D11::SetNormalRSState()
	{
		pImmediateContext->RSSetState(mSolidCullBackRS);
	}

	void CDispositifD3D11::InitBlendStates()
	{
		D3D11_BLEND_DESC blendDesc;

		// Effacer la description 
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

		// On initialise la description pour un m�lange alpha classique
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask =
			D3D11_COLOR_WRITE_ENABLE_ALL;

		// On cr�� l'�tat alphaBlendEnable
		DXEssayer(pD3DDevice->CreateBlendState(&blendDesc, &alphaBlendEnable),
			DXE_ERREURCREATION_BLENDSTATE);

		// Seul le booleen BlendEnable n�cessite d'�tre modifi�
		blendDesc.RenderTarget[0].BlendEnable = FALSE;

		// On cr�� l'�tat alphaBlendDisable 
		DXEssayer(pD3DDevice->CreateBlendState(&blendDesc, &alphaBlendDisable),
			DXE_ERREURCREATION_BLENDSTATE);
	}

	void CDispositifD3D11::ActiverMelangeAlpha() {
		float facteur[4] = { 0.0f,0.0f,0.0f,0.0f }; // Activer le m�lange - alpha blending. 
		pImmediateContext->OMSetBlendState(alphaBlendEnable, facteur, 0xffffffff);
	}

	void CDispositifD3D11::DesactiverMelangeAlpha() {
		float facteur[4] = { 0.0f,0.0f,0.0f,0.0f };
		// D�sactiver le m�lange - alpha blending. 
		pImmediateContext->OMSetBlendState(alphaBlendDisable, facteur, 0xffffffff);
	}

	void CDispositifD3D11::ActiverZBuffer()
	{
		pImmediateContext->OMSetDepthStencilState(mZBufferActif, 0);
	}
	void CDispositifD3D11::DesactiverZBuffer()
	{
		pImmediateContext->OMSetDepthStencilState(mZBufferInactif, 0);
	}
	void CDispositifD3D11::ActiverCulling()
	{
		pImmediateContext->RSSetState(mSolidCullBackRS);
	}
	void CDispositifD3D11::DesactiverCulling()
	{
		pImmediateContext->RSSetState(mSolidCullNoneRS);
	}
}

