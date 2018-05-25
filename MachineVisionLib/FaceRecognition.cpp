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
	pcaflNet = caffe2shell::LoadNet<FLOAT>(strCaffePrototxtFile, strCaffeModelFile, caffe::TEST);

	if (pcaflNet)
	{
		pflMemDataLayer = (caffe::MemoryDataLayer<FLOAT> *)pcaflNet->layers()[0].get();
		return TRUE;
	}
	else
		return FALSE;
}

//FACE_PROCESSING_EXT Mat FaceProcessing(const Mat &img_, double gamma = 0.8, double sigma0 = 0, double sigma1 = 0, double mask = 0, double do_norm = 8);
Mat FaceDatabase::FaceChipsHandle(Mat& matFaceChips, DOUBLE dblPowerValue, DOUBLE dblGamma, DOUBLE dblNorm)
{
	FLOAT *pflData;

	//* ��������ת�ɸ�������
	Mat matFloat;
	matFaceChips.convertTo(matFloat, CV_32F);
		
	Mat matTransit(Size(matFloat.cols, matFloat.rows), CV_32F, Scalar(0));	//* �����õĹ��ɾ���
	
	//* ������gammaֵΪ0��Ҳ����ǿ��������Ӱ����ĶԱȶ�
	if (fabs(dblGamma) < 1e-6)
		dblGamma = 0.2;
	for (INT i = 0; i < matFloat.rows; i++)
	{
		for (INT j = 0; j < matFloat.cols; j++)
			matFloat.ptr<FLOAT>(i)[j] = pow(matFloat.ptr<float>(i)[j], dblGamma);		
	}

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

BOOL FaceDatabase::AddFace(Mat& matImg)
{
	Mat matFaceChips = cv2shell::ExtractFaceChips(matImg);
	if (matFaceChips.empty())
	{
		cout << "No face was detected." << endl;
		return FALSE;
	}

	imshow("pic", matFaceChips);
	waitKey(0);

	//* ROI(region of interest)
	Mat matFaceROI = FaceChipsHandle(matFaceChips);

	vector<FLOAT> vImgFeature;
	caffe2shell::ExtractFeature(pcaflNet, pflMemDataLayer, matFaceROI, vImgFeature, FACE_FEATURE_DIMENSION, FACE_FEATURE_LAYER_NAME);


	return TRUE;
}

BOOL FaceDatabase::AddFace(const CHAR *pszImgName)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "AddFace() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return FALSE;
	}

	return AddFace(matImg);
}

