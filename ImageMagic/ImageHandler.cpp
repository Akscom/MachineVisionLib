#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <vector>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MathAlgorithmLib.h"
#include "ImagePreprocess.h"
#include "ImageHandler.h"

#if NEED_GPU
#pragma comment(lib,"cublas.lib")
#pragma comment(lib,"cuda.lib")
#pragma comment(lib,"cudart.lib")
#pragma comment(lib,"curand.lib")
#pragma comment(lib,"cudnn.lib")
#endif

static void EHImgPerspecTrans_OnMouse(INT nEvent, INT x, INT y, INT nFlags, void *pvIPT);

//* ͼ��͸�ӱ任
void ImagePerspectiveTransformation::process(Mat& mSrcImg, Mat& mResultImg, Mat& mShowImg, DOUBLE dblScaleFactor)
{
	CHAR  *pszSrcImgWinName = "ԴͼƬ";
	CHAR  *pszDestImgWinName = "Ŀ��ͼƬ";	

	namedWindow(pszSrcImgWinName, WINDOW_AUTOSIZE);
	namedWindow(pszDestImgWinName, WINDOW_AUTOSIZE);

	//* ��������¼��Ļص�����
	setMouseCallback(pszSrcImgWinName, EHImgPerspecTrans_OnMouse, this);

	BOOL blIsNotEndOpt = TRUE;
	BOOL blIsPut = FALSE;
	while (blIsNotEndOpt && cvGetWindowHandle(pszSrcImgWinName) && cvGetWindowHandle(pszDestImgWinName))
	{
		//* �ȴ�����
		CHAR bInputKey = waitKey(10);
		if (bInputKey == 'q' || bInputKey == 'Q' || bInputKey == 27)
			blIsNotEndOpt = FALSE;	


	}	

	//* ����������������
	destroyWindow(pszSrcImgWinName);
	destroyWindow(pszDestImgWinName);
}

//* ��괦���¼�
static void EHImgPerspecTrans_OnMouse(INT nEvent, INT x, INT y, INT nFlags, void *pvIPT)
{
	vector<Point2f>& vptROI = ((ImagePerspectiveTransformation *)pvIPT)->GetROI();
	
	//* ����Ƿ�����
	if (nEvent == EVENT_LBUTTONDOWN)
	{ 
		//* 4�����Ѿ�ȷ����
		if (vptROI.size() == 4)
		{
			//* �����û�������ǲ����Ѿ����ڵĽǵ㣬����ǣ���֧���û���ק�Ե���͸�ӱ任���򼰽Ƕ�
			for (INT i = 0; i < 4; i++)
			{
				
			}
		}
	}
}