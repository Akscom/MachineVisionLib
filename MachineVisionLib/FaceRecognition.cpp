// FaceRecognition.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include <facedetect-dll.h>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "FaceRecognition.h"

BOOL FaceDatabase::LoadCaffeVGGNet(string strCaffePrototxtFile, string strCaffeModelFile)
{
	caflNet = caffe2shell::LoadNet<FLOAT>(strCaffePrototxtFile, strCaffeModelFile, caffe::TEST);

	if (caflNet)
		return TRUE;
	else
		return FALSE;
}

BOOL FaceDatabase::AddFace(Mat& matFace)
{
	return FALSE;
}

BOOL FaceDatabase::AddFace(const CHAR *pszImgName)
{
	Mat matFaceChips = cv2shell::ExtractFaceChips(pszImgName);

	imshow("pic", matFaceChips);
	waitKey(0);

	FaceChipsHandle(matFaceChips, 0);

	return AddFace(matFaceChips);
}

//FACE_PROCESSING_EXT Mat FaceProcessing(const Mat &img_, double gamma = 0.8, double sigma0 = 0, double sigma1 = 0, double mask = 0, double do_norm = 8);
Mat FaceDatabase::FaceChipsHandle(Mat& matFaceChips, DOUBLE dblPowerValue, 
									DOUBLE dblGamma, DOUBLE dblSigma0, DOUBLE dblSigma1, DOUBLE dblNorm)
{
	FLOAT *pflData;

	//* ��������ת�ɸ�������
	Mat matFloat;
	matFaceChips.convertTo(matFloat, CV_32F);
	
	INT nExtendedBoundaryLen = floor(3 * abs(dblSigma1));	//* ͼ�α߽�Ҫ����ĳߴ�
	Mat matExtended(Size(matFloat.cols + 2 * nExtendedBoundaryLen, matFloat.rows + 2 * nExtendedBoundaryLen), CV_32F, Scalar(0));	//* ����ԭͼ�ߴ罨��һ���ĸ��߽�Ⱦ���չ�ľ������ڴ洢�����������
	Mat matTransit(Size(matFloat.cols, matFloat.rows), CV_32F, Scalar(0));	//* �����õĹ��ɾ���
	
	//* ������gammaֵΪ0��Ҳ����ǿ��������Ӱ����ĶԱȶ�
	if (fabs(dblGamma) < 1e-6)
		dblGamma = 0.2;
	for (INT i = 0; i < matFloat.rows; i++)
	{
		for (INT j = 0; j < matFloat.cols; j++)
			matFloat.ptr<FLOAT>(i)[j] = pow(matFloat.ptr<float>(i)[j], dblGamma);		
	}

	//* ���û������߽磬��ֱ�ӽ�����һ������
	if (fabs(dblSigma1) < 1e-6)
		goto __lblDoNormalizes;

	//* ���ж�ȡͼ�����ݵ���չ�������մ������ͼ��ʾ��Ч����
	/*
	             �Ϸ��߽�
	���Ͻ� =================== ���Ͻ�
	       ||               ||
	     ��||    ����ǰ��   ||��
	     ��||    ͼ������   ||��
	     ��||               ||��
		   ||               ||
	���½� =================== ���½�
	             �·��߽�
	*/
	for (INT i = 0; i < matExtended.rows; i++)
	{
		pflData = matExtended.ptr<FLOAT>(i);
		for (INT j = 0; j < matExtended.cols; j++)
		{
			//* ��ȡͼ��ԭ����������Ҳ��������߽��Χ������ʵ��ͼ��������������ǰ��ͼ�����ݣ�
			if (i >= nExtendedBoundaryLen && 
				i < matFloat.rows + nExtendedBoundaryLen && 
				j >= nExtendedBoundaryLen && 
				j < matFloat.cols + nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(i - nExtendedBoundaryLen)[j - nExtendedBoundaryLen];
			//* ���Ͻǣ���ԭʼͼ�����Ͻǵ��������
			else if (i < nExtendedBoundaryLen && j < nExtendedBoundaryLen)	
				pflData[j] = matFloat.ptr<FLOAT>(0)[0];
			//* ���Ͻǣ���ԭʼͼ�����Ͻǵ��������
			else if (i < nExtendedBoundaryLen && j >= matFloat.cols + nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(0)[matFloat.cols - 1];
			//* ���½ǣ���ԭʼͼ�����½ǵ��������
			else if (i >= matFloat.rows + nExtendedBoundaryLen && j < nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(matFloat.rows - 1)[0];
			//* ���½ǣ���ԭʼͼ�����½ǵ��������
			else if (i >= matFloat.rows + nExtendedBoundaryLen && j >= matFloat.cols + nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(matFloat.rows - 1)[matFloat.cols - 1];
			//* ������߽磬��ԭʼͼ�����һ�е��������
			else if (i < nExtendedBoundaryLen && j >= nExtendedBoundaryLen && j < matFloat.cols + nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(0)[j - nExtendedBoundaryLen];
			//* ������߽�,��ԭʼͼ����׶�һ�е��������
			else if (i >= matFloat.rows + nExtendedBoundaryLen && j >= nExtendedBoundaryLen && j < matFloat.cols + nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(matFloat.rows - 1)[j - nExtendedBoundaryLen];
			//* ������߽磬��ԭʼͼ�������һ�е��������
			else if (j < nExtendedBoundaryLen && i >= nExtendedBoundaryLen && i < matFloat.rows + nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(i - nExtendedBoundaryLen)[0];
			//* ������߽磬��ԭʼͼ�����ұ�һ�е��������
			else if (j >= matFloat.cols + nExtendedBoundaryLen && i >= nExtendedBoundaryLen && i < matFloat.rows + nExtendedBoundaryLen)
				pflData[j] = matFloat.ptr<FLOAT>(i - nExtendedBoundaryLen)[matFloat.cols - 1];
			else;
		}
	}

__lblDoNormalizes:
	//* ���Ϊ0.0��ֱ���������һ��
	DOUBLE dblTrim = fabs(dblNorm);
	if (dblTrim < 1e-6)
		goto __lblEnd;
	
	//* matFloat����ָ�����㲢�����ָ
	DOUBLE dblMeanVal = 0.0;
	matTransit = cv::abs(matFloat);
	for (INT i = 0; i<matFloat.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)
		{
			pflData[j] = pow(matTransit.ptr<FLOAT>(i)[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matFloat.rows * matFloat.cols);

	//* ��õľ�ֵָ���������������
	DOUBLE dblDivisor = cv::pow(dblMeanVal, 1/ dblPowerValue);
	for (INT i = 0; i < matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j<matFloat.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* ��ȥ�������ֵ��Ȼ����һ�����ֵ
	dblMeanVal = 0.0;
	matTransit = cv::abs(matFloat);
	for (INT i = 0; i<matFloat.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)
		{
			if (pflData[j] > dblTrim)
				pflData[j] = dblTrim;

			pflData[j] = pow(pflData[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matFloat.rows * matFloat.cols);

	//* ��һ�ν���ֵ����ָ���������������
	dblDivisor = cv::pow(dblMeanVal, 1 / dblPowerValue);
	for (INT i = 0; i < matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j<matFloat.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* �������0.0
	if(dblNorm > 1e-6)
	{ 
		for (INT i = 0; i<matFloat.rows; i++)
		{
			pflData = matFloat.ptr<FLOAT>(i);
			for (INT j = 0; j<matFloat.cols; j++)
				pflData[j] = dblTrim * tanh(pflData[j] / dblTrim);
		}
	}

__lblEnd:
	//* �ҳ��������Сֵ
	FLOAT flMin = matFloat.ptr<FLOAT>(0)[0];
	for (INT i = 0; i <matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)
		{
			if (pflData[j] < flMin)
				flMin = pflData[j];
		}
	}

	//* �������Сֵ
	for (INT i = 0; i <matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)	
				pflData[j] += flMin;
	}

	//* ��һ��
	normalize(matFloat, matFloat, 0, 255, NORM_MINMAX);
	matFloat.convertTo(matFloat, CV_8UC1);

	return matFloat;
}

