#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <facedetect-dll.h>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MathAlgorithmLib.h"
#include "ImagePreprocess.h"

//* ����ֱ��ͼ�����㷨��ǿͼ��ԱȶȺ������ȣ�ע������ͼ��һ���ǻҶ�ͼ�����ڻҶȼ��Ͷ�̬��Χ�ĸ��
//* https://blog.csdn.net/hit2015spring/article/details/50448025
//* ֱ��ͼ���⻯��ǿ�����۵�ַ��ע�⣬�ĵ����L���ǻҶ�ֵ������Ҳ����256���Ҷ�ֵ��0-255����
//* https://www.cnblogs.com/newpanderking/articles/2950242.html
//* http://lib.csdn.net/article/aiframework/52656
IMGPREPROC void imgpreproc::HistogramEqualization(Mat& matInputGrayImg, Mat& matDestImg)
{
	matDestImg = Mat(matInputGrayImg.rows, matInputGrayImg.cols, CV_8UC1);

	//* ��������
	DOUBLE dblPixelNum = matInputGrayImg.rows * matInputGrayImg.cols;

	INT naGrayLevelStatistic[256] = { 0 };			//* �Ҷȼ�ͳ�����飬ͳ�Ƹ��Ҷ�ֵ��ͼ���е���������ʼ��Ϊ0
	for (INT i = 0; i < matInputGrayImg.rows; i++)	//* �Ҷ�ֵ�ķ�ΧΪ0-255��ͨ������ѭ�����ǰ�256���Ҷ�ֵ��ͼ���г��ֵĴ���ͳ�Ƴ����������浽�����У�Ҳ���ǻ�ȡ��ͼ��ĻҶȼ��ֲ�����
	{
		UCHAR *pubData = matInputGrayImg.ptr<UCHAR>(i);
		for (INT j = 0; j < matInputGrayImg.cols; j++)
		{
			//* �ۼƸûҶ�ֵ�������������Ҷ�ֵ��Χ0-255�����ᳬ�����鷶Χ��ͨ��ͳ�����Ǿ�֪���÷�ͼ��ĻҶȷֲ������
			naGrayLevelStatistic[pubData[j]]++;
		}
	}

	//* ����Ҷȼ�ֱ��ͼ,Ҳ����ǰ��ͳ�Ƴ����ĻҶȼ������ܶ�
	DOUBLE dblaGrayLevelProb[256];
	for (INT i = 0; i < sizeof(dblaGrayLevelProb) / sizeof(DOUBLE); i++)
	{
		dblaGrayLevelProb[i] = (DOUBLE)naGrayLevelStatistic[i] / dblPixelNum;
	}

	//* �Ҿ��ð�����ļ����Ϊ����Ҷȱ任ϵ����ֱ��һЩ��רҵ��˵�������ۻ��ֲ�����
	DOUBLE dblaDestGrayLevelProb[256];
	dblaDestGrayLevelProb[0] = dblaGrayLevelProb[0];
	for (INT i = 1; i < sizeof(dblaDestGrayLevelProb) / sizeof(DOUBLE); i++)
	{
		dblaDestGrayLevelProb[i] = dblaDestGrayLevelProb[i-1] + dblaGrayLevelProb[i];
	}

	//* ���ݼ���ĻҶȱ任ϵ���任�Ҷȵõ�Ŀ��ͼ
	for (INT i = 0; i < matInputGrayImg.rows; i++)
	{
		UCHAR *pubData = matInputGrayImg.ptr<UCHAR>(i);
		UCHAR *pubDstData = matDestImg.ptr<UCHAR>(i);
		for (INT j = 0; j < matInputGrayImg.cols; j++)
		{
			pubDstData[j] = 255 * dblaDestGrayLevelProb[pubData[j]];
		}
	}
}

//* ���öԱȶȾ����㷨��ǿͼ�����۵�ַ��
//* https://blog.csdn.net/wuzuyu365/article/details/51898714
//* ���У������õ���gammaУ��������������£�
//* https://blog.csdn.net/w450468524/article/details/51649651
IMGPREPROC void imgpreproc::ContrastEqualization(Mat& matInputGrayImg, Mat& matDestImg, DOUBLE dblGamma, DOUBLE dblPowerValue, DOUBLE dblNorm)
{
	FLOAT *pflData;

	//* ��������ת�ɸ�������
	matInputGrayImg.convertTo(matDestImg, CV_32F);

	Mat matTransit(Size(matDestImg.cols, matDestImg.rows), CV_32F, Scalar(0));	//* �����õĹ��ɾ���

	//* ������gammaֵΪ0��Ҳ����ǿ��������Ӱ����ĶԱȶ�
	if (fabs(dblGamma) < 1e-6)
		dblGamma = 0.2;
	for (INT i = 0; i < matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
			pflData[j] = pow(pflData[j], dblGamma);
	}

	//* ���Ϊ0.0��ֱ���������һ��
	DOUBLE dblTrim = fabs(dblNorm);
	if (dblTrim < 1e-6)
		goto __lblEnd;

	//* matDestImg����ָ�����㲢�����ָ
	DOUBLE dblMeanVal = 0.0;
	matTransit = cv::abs(matDestImg);
	for (INT i = 0; i<matDestImg.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
		{
			pflData[j] = pow(matTransit.ptr<FLOAT>(i)[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matDestImg.rows * matDestImg.cols);

	//* ��õľ�ֵָ���������������
	DOUBLE dblDivisor = cv::pow(dblMeanVal, 1 / dblPowerValue);
	for (INT i = 0; i < matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j<matDestImg.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* ��ȥ�������ֵ��Ȼ����һ�����ֵ
	dblMeanVal = 0.0;
	matTransit = cv::abs(matDestImg);
	for (INT i = 0; i<matDestImg.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
		{
			if (pflData[j] > dblTrim)
				pflData[j] = dblTrim;

			pflData[j] = pow(pflData[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matDestImg.rows * matDestImg.cols);

	//* ��һ�ν���ֵ����ָ���������������
	dblDivisor = cv::pow(dblMeanVal, 1 / dblPowerValue);
	for (INT i = 0; i < matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j<matDestImg.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* �������0.0
	if (dblNorm > 1e-6)
	{
		for (INT i = 0; i<matDestImg.rows; i++)
		{
			pflData = matDestImg.ptr<FLOAT>(i);
			for (INT j = 0; j<matDestImg.cols; j++)
				pflData[j] = dblTrim * tanh(pflData[j] / dblTrim);
		}
	}

__lblEnd:
	//* �ҳ��������Сֵ
	FLOAT flMin = matDestImg.ptr<FLOAT>(0)[0];
	for (INT i = 0; i <matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
		{
			if (pflData[j] < flMin)
				flMin = pflData[j];
		}
	}

	//* �������Сֵ
	for (INT i = 0; i <matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
			pflData[j] += flMin;
	}

	//* ��һ��
	normalize(matDestImg, matDestImg, 0, 255, NORM_MINMAX);
	matDestImg.convertTo(matDestImg, CV_8UC1);
}

//* �����˲���ģ�壺
//* http://blog.sina.com.cn/s/blog_6ac784290101e47s.html
IMGPREPROC void imgpreproc::ContrastEqualizationWithFilter(Mat& matInputGrayImg, Mat& matDestImg, Size size, FLOAT *pflaFilterKernel, 
																DOUBLE dblGamma, DOUBLE dblPowerValue, DOUBLE dblNorm)
{
	FLOAT *pflData;

	//* ��������ת�ɸ�������
	matInputGrayImg.convertTo(matDestImg, CV_32F);

	Mat matTransit(Size(matDestImg.cols, matDestImg.rows), CV_32F, Scalar(0));	//* �����õĹ��ɾ���

	//* ������gammaֵΪ0��Ҳ����ǿ��������Ӱ����ĶԱȶ�
	if (fabs(dblGamma) < 1e-6)
		dblGamma = 0.2;
	for (INT i = 0; i < matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
			pflData[j] = pow(pflData[j], dblGamma);
	}

	//* �˲�
	Mat matFilterKernel = Mat(size, CV_32FC1, pflaFilterKernel);
	filter2D(matDestImg, matDestImg, matDestImg.depth(), matFilterKernel);

	//* ���Ϊ0.0��ֱ���������һ��
	DOUBLE dblTrim = fabs(dblNorm);
	if (dblTrim < 1e-6)
		goto __lblEnd;

	//* matDestImg����ָ�����㲢�����ָ
	DOUBLE dblMeanVal = 0.0;
	matTransit = cv::abs(matDestImg);
	for (INT i = 0; i<matDestImg.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
		{
			pflData[j] = pow(matTransit.ptr<FLOAT>(i)[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matDestImg.rows * matDestImg.cols);

	//* ��õľ�ֵָ���������������
	DOUBLE dblDivisor = cv::pow(dblMeanVal, 1 / dblPowerValue);
	for (INT i = 0; i < matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j<matDestImg.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* ��ȥ�������ֵ��Ȼ����һ�����ֵ
	dblMeanVal = 0.0;
	matTransit = cv::abs(matDestImg);
	for (INT i = 0; i<matDestImg.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
		{
			if (pflData[j] > dblTrim)
				pflData[j] = dblTrim;

			pflData[j] = pow(pflData[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matDestImg.rows * matDestImg.cols);

	//* ��һ�ν���ֵ����ָ���������������
	dblDivisor = cv::pow(dblMeanVal, 1 / dblPowerValue);
	for (INT i = 0; i < matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j<matDestImg.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* �������0.0
	if (dblNorm > 1e-6)
	{
		for (INT i = 0; i<matDestImg.rows; i++)
		{
			pflData = matDestImg.ptr<FLOAT>(i);
			for (INT j = 0; j<matDestImg.cols; j++)
				pflData[j] = dblTrim * tanh(pflData[j] / dblTrim);
		}
	}

__lblEnd:
	//* �ҳ��������Сֵ
	FLOAT flMin = matDestImg.ptr<FLOAT>(0)[0];
	for (INT i = 0; i <matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
		{
			if (pflData[j] < flMin)
				flMin = pflData[j];
		}
	}

	//* �������Сֵ
	for (INT i = 0; i <matDestImg.rows; i++)
	{
		pflData = matDestImg.ptr<FLOAT>(i);
		for (INT j = 0; j < matDestImg.cols; j++)
			pflData[j] += flMin;
	}

	//* ��һ��
	normalize(matDestImg, matDestImg, 0, 255, NORM_MINMAX);
	matDestImg.convertTo(matDestImg, CV_8UC1);
}

//* Ԥ�����������ȱ�Ե��⡢Ȼ���ҳ�����������������������Ա��������鴦��
void ImgGroupedContour::Preprocess(Mat& matSrcImg, DOUBLE dblThreshold1, DOUBLE dblThreshold2, INT nApertureSize, 
									DOUBLE dblGamma, DOUBLE dblPowerValue, DOUBLE dblNorm)
{
	Mat matGrayImg, matContrastEqualImg;

	cvtColor(matSrcImg, matGrayImg, COLOR_BGR2GRAY);
	imgpreproc::ContrastEqualization(matGrayImg, matContrastEqualImg, dblGamma, dblPowerValue, dblNorm);

	//* ʹ��Canny�㷨���б�Ե��⣬���۵�ַ��
	//* https://blog.csdn.net/jia20003/article/details/41173767
	//* Opencv�ṩ��Canny()����˵��:
	//* https://www.cnblogs.com/mypsq/p/4983566.html
	Canny(matContrastEqualImg, matGrayImg, dblThreshold1, dblThreshold2, nApertureSize);

	//* Ѱ��������������ϣ�
	//* https://blog.csdn.net/dcrmg/article/details/51987348
	findContours(matGrayImg, vContours, vHierarchy, RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));

	pstMallocMemForLink = pstNotGroupedContourLink = (PST_CONTOUR_NODE)malloc(vContours.size() * sizeof(ST_CONTOUR_NODE));
	if(!pstMallocMemForLink)
	{
		string strError = string("Preprocess()����ִ�д����޷�Ϊ��������Ľ�������һ�鶯̬�ڴ棡");
		throw runtime_error(strError);
	}

	//* ������������
	for (int i = 0; i < vContours.size(); i++)
	{
		//vector<Point> vPoints = vContours[i];
		//for (int j = 0; j < vPoints.size(); j++)
		//{
		//	cout << i << "-" << j << ": (" << vPoints[j].x << ", " << vPoints[j].y << ")" << endl;
		//}
		//cout << endl << endl;

		pstNotGroupedContourLink[i].pvecContour = &vContours[i];
		pstNotGroupedContourLink[i].nIndex = i;

		if (i)
		{
			pstNotGroupedContourLink[i - 1].pstNextNode = pstNotGroupedContourLink + i;
			pstNotGroupedContourLink[i].pstPrevNode = pstNotGroupedContourLink + (i - 1);
			pstNotGroupedContourLink[i].pstNextNode = NULL;
		}
		else
			pstNotGroupedContourLink[i].pstPrevNode = NULL;
	}
}

//* ���ٽ�����������һ��
void ImgGroupedContour::GlueAdjacentContour(INT nContourGroupIndex, DOUBLE dblDistanceThreshold)
{
	PST_CONTOUR_NODE pstNextContourNode = pstNotGroupedContourLink, pstFoundContourNode;
	BOOL blIsFoundGroupNode = FALSE;

__lblLoop:
	if (NULL == pstNextContourNode)
	{
		if (blIsFoundGroupNode) //* ���¿�ʼ�ң������оۼ�����֮�������������Ϊ����ͨ��opencv��ȡ�����������п��ֲܷ�������������Ҫ��β��Ҳ���ȷ���ֲ�·��
		{
			blIsFoundGroupNode = FALSE;
			pstNextContourNode = pstNotGroupedContourLink;

			//* �����浱ǰһ������������һ�飬����δ�����������Ƿ������½��ϵ��������������
			//* ע����������Ǵ��ڵģ���Ϊ����������������������ǰ����������
			goto __lblLoop;
		}
		else
		{
			return;
		}
	}

	PST_CONTOUR_NODE pstGroupNode = vGroupContour[nContourGroupIndex].pstContourLink;
	while (NULL != pstGroupNode)
	{
		DOUBLE dblDistance = malib::ShortestDistance(pstGroupNode->pvecContour, pstNextContourNode->pvecContour);
		if (dblDistance < dblDistanceThreshold)	//* ����ɵľ��뷶Χ�ڣ���ô���ž�����
		{
			//* �ȴ�ԭʼ������ժ����
			if (pstNextContourNode->pstPrevNode)
				pstNextContourNode->pstPrevNode->pstNextNode = pstNextContourNode->pstNextNode;
			else
			{
				//* �����һ���ڵ�û����һ���ڵ㣬��ʱҪ���������׽ڵ�
				pstNotGroupedContourLink = pstNextContourNode->pstNextNode;
			}

			if (pstNextContourNode->pstNextNode)
				pstNextContourNode->pstNextNode->pstPrevNode = pstNextContourNode->pstPrevNode;

			pstFoundContourNode = pstNextContourNode;
			pstNextContourNode = pstNextContourNode->pstNextNode;

			//* Ȼ����������������
			pstFoundContourNode->pstNextNode = pstGroupNode->pstNextNode;
			pstFoundContourNode->pstPrevNode = pstGroupNode;
			if (pstGroupNode->pstNextNode)
				pstGroupNode->pstNextNode->pstPrevNode = pstFoundContourNode;
			pstGroupNode->pstNextNode = pstFoundContourNode;

			vGroupContour[nContourGroupIndex].nContourNum++;

			blIsFoundGroupNode = TRUE;

			goto __lblLoop;
		}

		pstGroupNode = pstGroupNode->pstNextNode;
	}

	pstNextContourNode = pstNextContourNode->pstNextNode;
	goto __lblLoop;
}

//* ���������飬���õ��㷨�ܼ򵥣���������ȼ����������ľ��룬ֻҪ����С�ڲ���dblDistanceThresholdָ��������ֵ������Ϊ�����������������ͬһ��
void ImgGroupedContour::GroupContours(DOUBLE dblDistanceThreshold)
{
	INT nContourGroupIndex = 0;
	ST_CONTOUR_GROUP stContourGroup;
	PST_CONTOUR_NODE pstNextContourNode = pstNotGroupedContourLink;

__lblLoop:
	if (NULL == pstNextContourNode)
		return;

	//* ������ʣ��ĵĵ�һ���ڵ���Ϊ��һ���������ʼ�ڵ�
	stContourGroup.pstContourLink = pstNextContourNode;
	stContourGroup.nContourNum = 1;
	vGroupContour.push_back(stContourGroup);
	pstNotGroupedContourLink = pstNextContourNode->pstNextNode;
	if (pstNotGroupedContourLink)
		pstNotGroupedContourLink->pstPrevNode = NULL;

	pstNextContourNode->pstNextNode = NULL;
	pstNextContourNode->pstPrevNode = NULL;

	GlueAdjacentContour(nContourGroupIndex, dblDistanceThreshold);

	nContourGroupIndex++;

	pstNextContourNode = pstNotGroupedContourLink;

	goto __lblLoop;
}

//* ��ȡ���������ĶԽǵ�
void ImgGroupedContour::GetDiagonalPointsOfGroupContours(INT nMinContourNumThreshold)
{
	for (INT i = 0; i < vGroupContour.size(); i++)
	{
		ST_CONTOUR_GROUP stContourGroup = vGroupContour[i];

		//cout << "Contour Num: " << stContourGroup.nContourNum << endl;

		if (stContourGroup.nContourNum < nMinContourNumThreshold)
			continue;

		INT x_left_top = 10000, y_left_top = 10000, x_right_bottom = 0, y_right_bottom = 0;

		//* �ҳ������������ϽǺ����½ǵ�����
		PST_CONTOUR_NODE pstGroupNode = vGroupContour[i].pstContourLink;
		while (pstGroupNode != NULL)
		{
			//cout << pstGroupNode->nIndex << " ";

			for (INT j = 0; j < (*pstGroupNode->pvecContour).size(); j++)
			{
				Point point = (*pstGroupNode->pvecContour)[j];
				if (point.x < x_left_top)
					x_left_top = point.x;
				if (point.y < y_left_top)
					y_left_top = point.y;

				if (point.x > x_right_bottom)
					x_right_bottom = point.x;
				if (point.y > y_right_bottom)
					y_right_bottom = point.y;
			}

			pstGroupNode = pstGroupNode->pstNextNode;
		}

		cout << endl;

		Point left(x_left_top, y_left_top);
		Point right(x_right_bottom, y_right_bottom);

		//cout << "rect: " << left << ", " << right << endl;

		ST_DIAGONAL_POINTS stDiagPoints;
		stDiagPoints.point_left = left;
		stDiagPoints.point_right = right;
		vDiagonalPointsOfGroupContour.push_back(stDiagPoints);
	}
}

//* ͬ��
void ImgGroupedContour::GetDiagonalPointsOfGroupContours(INT nMinContourNumThreshold, vector<ST_DIAGONAL_POINTS>& vOutputDiagonalPoints)
{
	GetDiagonalPointsOfGroupContours(nMinContourNumThreshold);

	vOutputDiagonalPoints = vDiagonalPointsOfGroupContour;
}

//* �þ��α�Ƿ�������
void ImgGroupedContour::RectMarkGroupContours(Mat& matMarkImg, BOOL blIsMergeOverlappingRect, Scalar scalar)
{
	vector<ST_DIAGONAL_POINTS> vRects;

	//* �Ƿ���Ҫ�ϲ��ص��ľ���
	if (blIsMergeOverlappingRect)
	{
		cv2shell::MergeOverlappingRect(vDiagonalPointsOfGroupContour, vRects);
	}
	else
		vRects = vDiagonalPointsOfGroupContour;

	for (INT i = 0; i < vRects.size(); i++)
	{
		ST_DIAGONAL_POINTS stRect = vRects[i];
		rectangle(matMarkImg, stRect.point_left, stRect.point_right, scalar, 1);
	}
}

