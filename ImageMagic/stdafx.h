// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
//* ���DialogBox()��������
//* error C2664: ��INT_PTR DialogBoxParamA(HINSTANCE,LPCSTR,HWND,DLGPROC,LPARAM)��: �޷������� 4 �ӡ�INT_PTR (__cdecl *)(HWND,UINT,WPARAM,LPARAM)��ת��Ϊ��DLGPROC��
//* ���NO_STRICT������Ԥ���������������ӣ���Ϊcaffe��Ҫ�õ�������������ʱcaffe�ᱨ����DLGPROC��ԭ������ȴ��STRICT�ģ���˵����������������������
#define STRICT
#include <windows.h>
#undef STRICT

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
