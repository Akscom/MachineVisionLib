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

//* 
void ImgGroupedContour::Preprocess(Mat& matSrcImg, DOUBLE dblThreshold1, DOUBLE dblThreshold2, INT nApertureSize, DOUBLE dblGamma, DOUBLE dblPowerValue, DOUBLE dblNorm)
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
	findContours(matGrayImg, vContours, vHierarchy, RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
}

