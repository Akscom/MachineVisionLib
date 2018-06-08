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

//* ��ǿͼ�������ȣ�ע������ͼ��һ���ǻҶ�ͼ�����ڻҶȼ��Ͷ�̬��Χ�ĸ��
//* https://blog.csdn.net/hit2015spring/article/details/50448025
//* ֱ��ͼ���⻯��ǿ�����۵�ַ��ע�⣬�ĵ����L���ǻҶ�ֵ������Ҳ����256���Ҷ�ֵ��0-255����
//* https://www.cnblogs.com/newpanderking/articles/2950242.html
//* http://lib.csdn.net/article/aiframework/52656
IMGPREPROC_API void imgpreproc::HistogramEnhancedDefinition(Mat& matInputGrayImg, Mat& matDestImg)
{
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
	DOUBLE dblaGrayLevelRatio[256];
	for (INT i = 0; i < sizeof(dblaGrayLevelRatio) / sizeof(DOUBLE); i++)
	{
		dblaGrayLevelRatio[i] = (DOUBLE)naGrayLevelStatistic[i] / dblPixelNum;
	}

	//* ����Ҷȱ任ϵ��������˵�ۻ��ֲ�����
	DOUBLE dblaDestGrayLevelRatio[256];
	dblaDestGrayLevelRatio[0] = dblaGrayLevelRatio[0];
	for (INT i = 1; i < sizeof(dblaDestGrayLevelRatio) / sizeof(DOUBLE); i++)
	{
		dblaDestGrayLevelRatio[i] = dblaDestGrayLevelRatio[i-1] + dblaGrayLevelRatio[i];
	}

	//* ���ݼ���ĻҶȱ任ϵ���任�Ҷȵõ�Ŀ��ͼ
	for (INT i = 0; i < matInputGrayImg.rows; i++)
	{
		UCHAR *pubData = matInputGrayImg.ptr<UCHAR>(i);
		UCHAR *pubDstData = matDestImg.ptr<UCHAR>(i);
		for (INT j = 0; j < matInputGrayImg.cols; j++)
		{
			pubDstData[j] = 255 * dblaDestGrayLevelRatio[pubData[j]];
		}
	}
}

