// ConveyorBeltTearingDetector.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <io.h>
#include <vector>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MathAlgorithmLib.h"
#include "ImagePreprocess.h"

#if NEED_GPU
#pragma comment(lib,"cublas.lib")
#pragma comment(lib,"cuda.lib")
#pragma comment(lib,"cudart.lib")
#pragma comment(lib,"curand.lib")
#pragma comment(lib,"cudnn.lib")
#endif

#define NEED_DEBUG_CONSOLE	1	//* �Ƿ���Ҫ����̨�������

#if !NEED_DEBUG_CONSOLE
#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " [image path]" << endl;
		exit(-1);
	}

	Mat matGrayImg, matPreprocImg;

	Mat matSrcImg = imread(argv[1]);
	if (matSrcImg.empty())
	{
		cout << "ͼƬ�ļ���" << argv[1] << "�������ڻ��߸�ʽ����ȷ�������˳�!" << endl;
		exit(-1);
	}

	//* ����ͼƬ�ɼ��Ĳ����ã���Ҫ��Ϊָ��ROI��ȥ���ذ����������
	matPreprocImg = matSrcImg(Rect(10, 50, matSrcImg.cols - 130, matSrcImg.rows - 100));

	//* ԭʼͼƬ̫������Сһ��	
	pyrDown(matPreprocImg, matSrcImg, Size(matPreprocImg.cols / 2, matPreprocImg.rows / 2));

	//* �����Ƚϵͺͽϸߵĵ�ͼ�������ƽ����
	imgpreproc::AdjustBrightnessMean(matSrcImg, matPreprocImg, 160);
	imshow("���ȵ������", matPreprocImg);
	waitKey(0);

	//* �ڶ�ͼ������ѷ���֮ǰ�ȴ���һ�£�ʵ��ԱȶȾ����㷨Ч������	
	cvtColor(matPreprocImg, matGrayImg, COLOR_BGR2GRAY);
	imgpreproc::ContrastEqualization(matGrayImg, matPreprocImg);

	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));	
	morphologyEx(matPreprocImg, matGrayImg, MORPH_ERODE, element);
	morphologyEx(matGrayImg, matPreprocImg, MORPH_OPEN, element);

	imshow("Ԥ������", matPreprocImg);
	waitKey(0);

	try {
		//* �Լ�⵽���������飬�ҳ��ѷ�
		ImgGroupedContour img_grp_contour = ImgGroupedContour(matPreprocImg, 90, 90 * 2, 3, TRUE);
		img_grp_contour.GroupContours(20);
		img_grp_contour.GetDiagonalPointsOfGroupContours(5);
		img_grp_contour.RectMarkGroupContours(matSrcImg);

		imshow("�ѷ�����", matSrcImg);
		waitKey(0);
	}
	catch (runtime_error err) {
		cout << err.what() << endl;
	}	

    return 0;
}

