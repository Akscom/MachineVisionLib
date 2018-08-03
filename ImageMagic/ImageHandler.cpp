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
void ImagePerspectiveTransformation::process(Mat& mSrcImg, Mat& mResultImg, Mat& mSrcShowImg, DOUBLE dblScaleFactor)
{
	CHAR *pszSrcImgWinName = "ԴͼƬ";
	CHAR *pszDestImgWinName = "Ŀ��ͼƬ";

	o_dblScaleFactor = dblScaleFactor;

	//* ������������
	namedWindow(pszDestImgWinName, WINDOW_AUTOSIZE);	
	namedWindow(pszSrcImgWinName, WINDOW_AUTOSIZE);	

	//* ����Ŀ�괰��
	cv2shell::ShowImageWindow(pszDestImgWinName, FALSE);

	//* ��ʾԭʼͼƬ
	imshow(pszSrcImgWinName, mSrcShowImg);	

	//* ��������¼��Ļص�����
	setMouseCallback(pszSrcImgWinName, EHImgPerspecTrans_OnMouse, this);

	BOOL blIsNotEndOpt = TRUE;
	BOOL blIsPut = FALSE;
	while (blIsNotEndOpt && cvGetWindowHandle(pszSrcImgWinName) && cvGetWindowHandle(pszDestImgWinName))
	{
		//* �ȴ�����
		CHAR bInputKey = waitKey(10);
		switch ((CHAR)toupper(bInputKey))
		{
		case 'R':
			o_vptROI.push_back(o_vptROI[0]);
			o_vptROI.erase(o_vptROI.begin());

			//* �ػ�
			o_blIsNeedPaint = TRUE;
			break;

		case 'I':
			swap(o_vptROI[0], o_vptROI[1]);
			swap(o_vptROI[2], o_vptROI[3]);

			//* �ػ�
			o_blIsNeedPaint = TRUE;
			break;
		
		case 'D':
		case 46:
			o_vptROI.clear();

			//* �ָ�ԭʼͼ��
			mSrcImg.copyTo(mResultImg);

			//* ����Ŀ�괰��
			cv2shell::ShowImageWindow(pszDestImgWinName, FALSE);

			//* �ػ�
			o_blIsNeedPaint = TRUE;
			break;
		
		case 'Q':
		case 27:	//* Esc��
			blIsNotEndOpt = FALSE;
			break;

		default:
			break;
		}

		//* �Ƿ���Ҫ���ƽǵ�����
		if (o_blIsNeedPaint)
		{
			o_blIsNeedPaint = FALSE;

			//* ���»�ȡԭʼ����
			Mat mShowImg = mSrcShowImg.clone();

			//* ���ƽǵ�
			for (INT i = 0; i < o_vptROI.size(); i++)
			{
				Point2f point = o_vptROI[i] * o_dblScaleFactor;	

				circle(mShowImg, point, 4, Scalar(0, 255, 0), 2);
				if (i)
				{
					line(mShowImg, o_vptROI[i - 1] * o_dblScaleFactor, point, Scalar(0, 0, 255), 2);
					circle(mShowImg, point, 5, Scalar(0, 255, 0), 3);
				}
			}

			//* 4����ѡ�������
			if (o_vptROI.size() == 4)
			{
				//* ��ͷβ�����ǵ�������
				Point2f point = o_vptROI[0] * o_dblScaleFactor;
				line(mShowImg, point, o_vptROI[3] * o_dblScaleFactor, Scalar(0, 0, 255), 2);
				circle(mShowImg, point, 5, Scalar(0, 255, 0), 3);

				//* ����͸��ת��
				//* ===================================================================================================================
				vector<Point2f> vptDstCorners(4);

				//* Ŀ��ǵ����ϵ㣬����4�����λ��ȷ���ǰ���˳ʱ�뷽����ģ�����֮���û������Ȱ������ϡ����ϡ����¡����µ�˳��
				//* ѡ��任������ܱ任���û�ϣ����ͼƬ����
				vptDstCorners[0].x = 0;
				vptDstCorners[0].y = 0;

				//* norm()�����Ĺ�ʽΪ:sqrt(x^2 + y^2)������õ�������ά��֮���ŷ����¾���
				//* �Ƚ�0��1��2��3��֮���Ǹ�ŷʽ������󣬴���Ǹ�����Ŀ��ǵ��е����ϵ㣬ͨ
				//* ��ŷʽ������㣬����֪��Ŀ�����ϵ��������λ����
				vptDstCorners[1].x = (FLOAT)max(norm(o_vptROI[0] - o_vptROI[1]), norm(o_vptROI[2] - o_vptROI[3]));
				vptDstCorners[1].y = 0;

				//* Ŀ��ǵ�����µ�
				vptDstCorners[2].x = (float)max(norm(o_vptROI[0] - o_vptROI[1]), norm(o_vptROI[2] - o_vptROI[3]));
				vptDstCorners[2].y = (float)max(norm(o_vptROI[1] - o_vptROI[2]), norm(o_vptROI[3] - o_vptROI[0]));

				//* Ŀ��ǵ�����µ�
				vptDstCorners[3].x = 0;
				vptDstCorners[3].y = (float)max(norm(o_vptROI[1] - o_vptROI[2]), norm(o_vptROI[3] - o_vptROI[0]));

				//* ����ת�����󣬲�ת��
				Mat H = findHomography(o_vptROI, vptDstCorners);
				warpPerspective(mSrcImg, mResultImg, H, Size(cvRound(vptDstCorners[2].x), cvRound(vptDstCorners[2].y)));

				//* ��ʾת�����
				cv2shell::ShowImageWindow(pszDestImgWinName, TRUE);
				imshow(pszDestImgWinName, mResultImg);
				//* ===================================================================================================================
			}

			imshow(pszSrcImgWinName, mShowImg);
		}		
	}	

	//* ����������������
	destroyWindow(pszSrcImgWinName);
	destroyWindow(pszDestImgWinName);
}

//* ��괦���¼�
static void EHImgPerspecTrans_OnMouse(INT nEvent, INT x, INT y, INT nFlags, void *pvIPT)
{
	ImagePerspectiveTransformation *pobjIPT = (ImagePerspectiveTransformation *)pvIPT;
	vector<Point2f>& vptROI = pobjIPT->GetROI();

	DOUBLE dblScaleFactor = pobjIPT->GetScaleFactor();

	//* ת���ԭʼͼƬ����
	FLOAT flSrcImgX = ((FLOAT)x) / dblScaleFactor;
	FLOAT flSrcImgY = ((FLOAT)y) / dblScaleFactor;

	//* ����Ѱ���
	if (nEvent == EVENT_LBUTTONDOWN)
	{
		//* �����û�������ǲ����Ѿ����ڵĽǵ㣬����ǣ���֧���û���ק�Ե���͸�ӱ任���򼰽Ƕ�
		for (INT i = 0; i < vptROI.size(); i++)
		{
			if (abs(vptROI[i].x - flSrcImgX) < MOUSE_SHOT_OFFSET && abs(vptROI[i].y - flSrcImgY) < MOUSE_SHOT_OFFSET)
			{
				pobjIPT->SetDragingCornerPointIndex(i);
				break;
			}
		}
	}

	//* �����λ�ô�������ɿ��¼�������if�����˳���ܱ䣬�����ſɱ�����������Ӧ�û����ĳ���ǵ����ק�¼�
	if (nEvent == EVENT_LBUTTONUP)
	{ 
		//* ��ûѡ����4���㣬��������֮
		if (vptROI.size() < 4 && INVALID_INDEX == pobjIPT->GetDragingCornerPointIndex())
		{
			vptROI.push_back(Point2f(flSrcImgX, flSrcImgY));
			pobjIPT->NeedToDrawCornerPoint();
		}

		//* ֻҪ�ɿ���꣬����֧����ק
		pobjIPT->SetDragingCornerPointIndex(INVALID_INDEX);
	}

	//* ����ƶ��¼�
	if (nEvent == EVENT_MOUSEMOVE)
	{
		//* �����û��Ƿ���ĳ���ǵ��ϵ����
		INT nCornerPointIdx = pobjIPT->GetDragingCornerPointIndex();
		if (nCornerPointIdx != INVALID_INDEX)
		{
			vptROI[nCornerPointIdx].x = flSrcImgX;
			vptROI[nCornerPointIdx].y = flSrcImgY;
			pobjIPT->NeedToDrawCornerPoint();
		}		
	}
}