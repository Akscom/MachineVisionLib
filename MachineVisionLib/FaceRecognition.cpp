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
Mat FaceDatabase::FaceChipsHandle(Mat& matFaceChips, DOUBLE dblGamma, DOUBLE dblSigma0, DOUBLE dblSigma1, DOUBLE dblNorm)
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
	       ||    ����ǰ��   ||
	       ||    ͼ������   ||
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
			//����
			else if (i >= im.rows + b&&i<rows + 2 * b&&j<b)
				pData1[j] = im.ptr<float>(rows - 1)[0];
			//����
			else if (i >= im.rows + b&&j >= im.cols + b)
				pData1[j] = im.ptr<float>(im.rows - 1)[im.cols - 1];
			//�Ϸ�
			else if (i<b&&j >= b&&j<im.cols + b)
				pData1[j] = im.ptr<float>(0)[j - b];
			//�·�
			else if (i >= im.rows + b&&j >= b&&j<im.cols + b)
				pData1[j] = im.ptr<float>(im.rows - 1)[j - b];
			//��
			else if (j<b&&i >= b&&i<im.rows + b)
				pData1[j] = im.ptr<float>(i - b)[0];
			//�ҷ�
			else if (j >= im.cols + b&&i >= b&&i<im.rows + b)
				pData1[j] = im.ptr<float>(i - b)[im.cols - 1];
		}
	}

__lblDoNormalizes:
	//* ���
	if (fabs(dblNorm) < 1e-6)
		goto __lblEbd;

__lblEbd:
	for (INT i = 0; i <matFloat.rows; i++)
	{

	}

	return matFloat;
}

