#include "StdAfx.h"
#include "resource.h"

#include "MoteurWindows.h"
#include "InputManager.h"

namespace PM3D
{
HINSTANCE CMoteurWindows::hAppInstance;	// handle Windows de l'instance actuelle de l'application

CMoteurWindows::CMoteurWindows(void)
{
}

CMoteurWindows::~CMoteurWindows(void)
{
}

//   FONCTION�: SetWindowsAppInstance(HANDLE, int)
//
//   BUT�: Prend en note le handle de l'instance
//
void CMoteurWindows::SetWindowsAppInstance(HINSTANCE hInstance)
{
	hAppInstance = hInstance; // Stocke le handle d'instance de l'application, plusieurs fonctions sp�cifiques en auront besoin
}


//   FONCTION�: InitAppInstance()
//
//   BUT�:  Effectue des op�rations d'initialisation de l'application
//
//   COMMENTAIRES�:
//
//        Dans cette fonction, nous enregistrons le handle de l'instance dans une variable globale, puis
//        cr�ons et affichons la fen�tre principale du programme.
//
bool CMoteurWindows::InitAppInstance()
{
TCHAR szTitle[MAX_LOADSTRING];					// Le texte de la barre de titre
	
	// Initialise les cha�nes globales
	LoadString(hAppInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hAppInstance, IDC_PETITMOTEUR3D, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hAppInstance);

	hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hAppInstance, NULL);

	if (!hMainWnd)
	{
	  return false;
	}

	hAccelTable = LoadAccelerators(hAppInstance, MAKEINTRESOURCE(IDC_PETITMOTEUR3D));

	return true;
}

//
//  FONCTION�: MyRegisterClass()
//
//  BUT�: inscrit la classe de fen�tre.
//
//  COMMENTAIRES�:
//
//    Cette fonction et son utilisation sont n�cessaires uniquement si vous souhaitez que ce code
//    soit compatible avec les syst�mes Win32 avant la fonction 'RegisterClassEx'
//    qui a �t� ajout�e � Windows�95. Il est important d'appeler cette fonction
//    afin que l'application dispose des petites ic�nes correctes qui lui sont
//    associ�es.
//
//	  FJ:  Copie-coller de la fonction MyRegisterClass g�n�r�e automatiquement
//
ATOM CMoteurWindows::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= &WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PETITMOTEUR3D));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PETITMOTEUR3D);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


//  FONCTION�: WndProc(HWND, unsigned, WORD, LONG)
//
//  BUT�:  traite les messages pour la fen�tre principale.
//
//  WM_COMMAND	- traite le menu de l'application
//  WM_PAINT	- dessine la fen�tre principale
//  WM_DESTROY	- g�n�re un message d'arr�t et retourne
//
//
LRESULT CALLBACK CMoteurWindows::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Analyse les s�lections de menu�:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hAppInstance, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// �vitez d'ajouter du code ici...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int CMoteurWindows::Show()
{
	ShowWindow(hMainWnd, SW_SHOWNORMAL  );
	UpdateWindow(hMainWnd);

	return 0;
}

// Gestionnaire de messages pour la bo�te de dialogue � propos de.
INT_PTR CALLBACK CMoteurWindows::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

//
//   FONCTION : RunSpecific 
//
//   BUT :	�l�ments internes de la boucle de l'application (Boucle message)
//
//   NOTES:		
//		Il s'agit d'une version modifi�e sp�cifiquement pour nos besoins des 
//      �l�ments de la boucle message de Windows
//
bool CMoteurWindows::RunSpecific() 
{
MSG msg;
bool bBoucle=true;

	// Y-a-t'il un message Windows � traiter?
	if (::PeekMessage(&msg, NULL, 0,0, PM_REMOVE))
	{
		// Est-ce un message de fermeture ?
		if (msg.message==WM_QUIT) bBoucle=false;

		// distribuer le message
		if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return bBoucle;
}

int CMoteurWindows::InitialisationsSpecific()
{
	// Initialisations  de l'application;
	InitAppInstance();
	Show();

	// Initialisation de DirectInput
	InputManager::GetInstance().Init(hAppInstance, hMainWnd);

	return 0;
}

//
//  FONCTION : GetTimeSpecific 
//
//  BUT :	Fonction responsable d'obtenir l'heure syst�me en 
// 			milli�mes de secondes
//
//
__int64 CMoteurWindows::GetTimeSpecific()
{
	return ::timeGetTime(); 
}

//  FONCTION : CreationDispositifSpecific 
//
//  BUT :	Fonction responsable de cr�er le 
//			dispositif selon certains param�tres
//  NOTES:		
//		 CDS_MODE: 	CDS_FENETRE 		application fen�tr�e
//					CDS_PLEIN_ECRAN 	application plein �cran
//
CDispositifD3D11* CMoteurWindows::CreationDispositifSpecific(const CDS_MODE cdsMode)
{
CDispositifD3D11* pDispositif;

	pDispositif = new CDispositifD3D11(cdsMode, hMainWnd);
	
	return pDispositif;

}


void CMoteurWindows::BeginRenderSceneSpecific()
{
	ID3D11DeviceContext* pImmediateContext = pDispositif->GetImmediateContext();
	ID3D11RenderTargetView* pRenderTargetView = pDispositif->GetRenderTargetView();

	    // On efface la surface de rendu
    float Couleur[4] = { 0.0f, 1.0f, 0.0f, 1.0f };  //  RGBA - Noir pour le moment
    pImmediateContext->ClearRenderTargetView( pRenderTargetView, Couleur );	

	// On r�-initialise le tampon de profondeur
	ID3D11DepthStencilView* pDepthStencilView = pDispositif->GetDepthStencilView();	
	pImmediateContext->ClearDepthStencilView( pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}



void CMoteurWindows::EndRenderSceneSpecific()
{
}



} // namespace PM3D

