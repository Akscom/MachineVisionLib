// ImageMagic.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <wtypes.h>
#include <Commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <vector>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MathAlgorithmLib.h"
#include "ImagePreprocess.h"
#include "ImageHandler.h"
#include "ImageMagic.h"

#if NEED_GPU
#pragma comment(lib,"cublas.lib")
#pragma comment(lib,"cuda.lib")
#pragma comment(lib,"cudart.lib")
#pragma comment(lib,"curand.lib")
#pragma comment(lib,"cudnn.lib")
#endif

#define MAX_LOADSTRING 100

HINSTANCE hInst;					//* ��ǰʵ��
HWND hWndMain;						//* �����ھ��
HWND hWndStatic;					//* ��̬�ı�������OCV��ʾ���ڵĸ�����
CHAR szTitle[MAX_LOADSTRING];		//* �������ı�
CHAR szWindowClass[MAX_LOADSTRING];	//* ����������
CHAR szImgFileName[MAX_PATH + 1];	//* �򿪵�ͼ���ļ���
Mat mOpenedImg;						//* �����ԭʼͼƬ����
Mat mResultImg;						//* �����Ľ������

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void CreateTextStatic(void);

//* ���ҵ����
//* ----------------------------------------------------------------------
BOOL GetImageFile(CHAR *pszImgFileName, UINT unImgFileNameSize);	//* ѡ��һ��ͼ���ļ�
void OpenImgeFile(CHAR *pszImgFileName);							//* �򿪲��������ڻ��Ƹ�ͼƬ
void SetWindowSize(INT nWidth, INT nHeight);						//* �������ڴ�С
void DrawImage(void);												//* ����ͼƬ

//* ----------------------------------------------------------------------

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: �ڴ˷��ô��롣

    // ��ʼ��ȫ���ַ���
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_IMAGEMAGIC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ִ��Ӧ�ó����ʼ��: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	hWndStatic = NULL;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IMAGEMAGIC));

    MSG msg;

    // ����Ϣѭ��: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IMAGEMAGIC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_IMAGEMAGIC);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWndMain = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWndMain)
   {
      return FALSE;
   }

   //* ʹ�ܴ��ڽ����ļ��Ϸ�
   DragAcceptFiles(hWndMain, TRUE);

   ShowWindow(hWndMain, nCmdShow);
   UpdateWindow(hWndMain);

   return TRUE;
}

//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND  - ����Ӧ�ó���˵�
//  WM_PAINT    - ����������
//  WM_DESTROY  - �����˳���Ϣ������
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {	
	//* ��Ļ�ֱ��ʸ���֪ͨ
	case WM_DISPLAYCHANGE:
		DrawImage();
		break;

	//* �ļ��϶�֪ͨ
	case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, 0, szImgFileName, sizeof(szImgFileName));
		OpenImgeFile(szImgFileName);
		DragFinish((HDROP)wParam);
		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // �����˵�ѡ��: 
            switch (wmId)
            {
			case IDM_OPEN:
				if (GetImageFile(szImgFileName, sizeof(szImgFileName)))
				{
					//* �ȱ�����ʾͼƬ������ͼƬ��ȱ�����С��Сͼ��ά��1��1����
					OpenImgeFile(szImgFileName);
				}
				break;

			case IDM_CUT:
				IMGPROC_Cut();
				break;

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
			DrawImage();
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// �����ڡ������Ϣ�������
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

//* ����һ����̬�ı�����
void CreateTextStatic(void)
{
	if (hWndStatic != NULL)
	{
		//ShowWindow(hWndStatic, SW_HIDE);
		//DestroyWindow(hWndStatic);
		return;
	}

	RECT rect_client;

	GetClientRect(hWndMain, &rect_client);
	
	hWndStatic = CreateWindow(TEXT("STATIC"), TEXT(""),
								WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTERIMAGE | WS_EX_STATICEDGE,
								0, 0, rect_client.right, rect_client.bottom,
								hWndMain,
								(HMENU)NULL,
								hInst, NULL);	

	namedWindow(szTitle, WINDOW_NORMAL);

	HWND hWndOCVImgShow = (HWND)cvGetWindowHandle(szTitle);
	HWND hWndParentOCVImgShow = GetParent(hWndOCVImgShow);
	SetParent(hWndOCVImgShow, hWndStatic);		//* ���������л�ΪSTATIC�ؼ�
	ShowWindow(hWndParentOCVImgShow, SW_HIDE);	//* ����OpenCV�����д���
}

//* ѡ��һ��Ҫ�༭��ͼ���ļ�������pszImgFileName����ͼ���ļ���·�������ƣ�unImgFileNameSizeָ��pszImgFileName�������Ĵ�С����λ���ֽ�
BOOL GetImageFile(CHAR *pszImgFileName, UINT unImgFileNameSize)
{
	OPENFILENAME ofn;

	//* Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWndMain;
	ofn.lpstrFile = pszImgFileName;
	ofn.lpstrFile[0] = '\0';	//* ����Ϊ0����ʹ��pszImagFileName�е�������Ϊ��ʼ·��

	ofn.nMaxFile = unImgFileNameSize;
	ofn.lpstrFilter = _T("Jpeg Files (*.jpg)\0*.jpg\0PNG Files (*.png)\0*.png\0Bitmap Files (*.bmp)\0*.bmp\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = _T("��");

	return GetOpenFileName(&ofn);
}

//* �򿪲��������ڻ��Ƹ�ͼƬ
void OpenImgeFile(CHAR *pszImgFileName)
{
	INT nImgWidth, nImgHeight, nPCWidth, nPCHeight;

	CreateTextStatic();
	
	//* ����ͼ��
	Mat mSrcImg;
	mSrcImg = imread(pszImgFileName);
	if (mSrcImg.empty())
	{
		MessageBox(hWndMain, "�޷���ȡͼƬ�ļ�����ȷ��ͼƬ�ļ������Ҹ�ʽ��ȷ��", "����", MB_OK);
		return;
	}

	nImgWidth = mSrcImg.cols;
	nImgHeight = mSrcImg.rows;

	//* �����Ƿ񳬹���ǰ��Ļ��С
	nPCWidth = GetSystemMetrics(SM_CXSCREEN);
	nPCHeight = GetSystemMetrics(SM_CYSCREEN);
	if (nImgWidth > nPCWidth || nImgHeight > nPCHeight)
	{
		nImgWidth /= 2;
		nImgHeight /= 2;
	}
	SetWindowSize(nImgWidth, nImgHeight);

	mSrcImg.copyTo(mOpenedImg);
}

//* �����ڵ�������ͼƬһ���Ĵ�С
void SetWindowSize(INT nWidth, INT nHeight)
{
	RECT rect_window, rect_old_client;
	INT nBorderWidth, nBorderHeight, nWindowWidth, nWindowHeight;

	//* ��ȡ��Ļ��С
	INT nPCWidth = GetSystemMetrics(SM_CXSCREEN);
	INT nPCHeight = GetSystemMetrics(SM_CYSCREEN);

	//* ��ȡ���ں��û������RECT����
	GetWindowRect(hWndMain, &rect_window);
	GetClientRect(hWndMain, &rect_old_client);

	nBorderWidth = (rect_window.right - rect_window.left) - (rect_old_client.right - rect_old_client.left);
	nBorderHeight = (rect_window.bottom - rect_window.top) - (rect_old_client.bottom - rect_old_client.top);

	nWindowWidth = nBorderWidth + nWidth;
	nWindowHeight = nBorderHeight + nHeight;

	SetWindowPos(hWndMain, NULL, (nPCWidth - nWindowWidth) / 2, (nPCHeight - nWindowHeight) / 2, nWindowWidth, nWindowHeight, SWP_NOMOVE | SWP_NOZORDER);
	//MoveWindow(hWndMain, (nPCWidth - nWindowWidth) / 2, (nPCHeight - nWindowHeight) / 2, nWindowWidth, nWindowHeight, TRUE);
	SetWindowPos(hWndStatic, NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOZORDER);

	resizeWindow(szTitle, nWidth, nHeight);
}

//* ����ͼƬ
void DrawImage(void)
{
	if (mOpenedImg.empty())
		return;

	imshow(szTitle, mOpenedImg);
}

