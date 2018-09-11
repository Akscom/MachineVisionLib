// ObjectClassifier.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <io.h>
#include <algorithm>
#include <vector>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MVLVideo.h"

using namespace common_lib;
using namespace cv2shell;

//* Usage��ʹ��˵��
static const CHAR *pszCmdLineArgs = {
	"{help    h   |     | print help message}"
	"{@file       |     | input data source, image file path or rtsp stream address}"
	"{model   m   | VGG | which model to use, value is VGG��Yolo2 or Yolo3}"
	"{picture p   |     | image file classification, non video calssification}"
};

//* VGGģ��֮ͼƬ������
static void __VGGModelClassifierOfPicture(Net& objDNNNet, vector<string>& vClassNames, const string& strFile)
{
	Mat mSrcImg = imread(strFile);

	vector<RecogCategory> vObjects;
	ObjectDetect(mSrcImg, objDNNNet, vClassNames, vObjects);

	MarkObjectWithRectangle(mSrcImg, vObjects);

	Mat mDstImg;
	resize(mSrcImg, mDstImg, Size(mSrcImg.cols / 2, mSrcImg.rows / 2), 0, 0, INTER_AREA);
	imshow("Object Classifier", mDstImg);
	waitKey(0);
}

//* VGGģ��֮��Ƶ������������blIsVideoFileΪ"��"������ζ����һ����Ƶ�ļ���"��"����ζ����rtsp��
static void __VGGModelClassifierOfVideo(Net& objDNNNet, vector<string>& vClassNames, const string& strFile, BOOL blIsVideoFile)
{

}

//* VGGģ�ͷ�����
static void __VGGModelClassifier(string& strFile, BOOL blIsPicture)
{
	vector<string> vClassNames;
	Net objDNNNet = InitLightClassifier(vClassNames);
	if (objDNNNet.empty())
	{
		cout << "The initialization lightweight classifier failed, probably because the model file or voc.names file was not found." << endl;
		return;
	}

	
	if (blIsPicture)
	{
		__VGGModelClassifierOfPicture(objDNNNet, vClassNames, strFile);
	}
	else
	{
		//* ������ʵʱ��Ƶ��������Ƶ�ļ��������ļ���ǰ׺�жϼ���
		std::transform(strFile.begin(), strFile.end(), strFile.begin(), ::tolower);
		if (strFile.find("rtsp:", 0) == 0)
		{
			__VGGModelClassifierOfVideo(objDNNNet, vClassNames, strFile, FALSE);
		}
		else
			__VGGModelClassifierOfVideo(objDNNNet, vClassNames, strFile, TRUE);
	}
}

//* ͨ�÷���������ͼ�����Ƶ����������࣬����ģ�Ͳ���Ԥѵ��ģ�ͺ�����ض�������ѵ��ģ��
int _tmain(int argc, _TCHAR* argv[])
{
	CommandLineParser objCmdLineParser(argc, argv, pszCmdLineArgs);

	if (!IsCommandLineArgsValid(pszCmdLineArgs, argc, argv) || objCmdLineParser.has("help"))
	{
		cout << "This is a classifier program, using the pre training model to detect and classify objects in the image. " << endl;
		objCmdLineParser.printMessage();

		return 0;
	}

	//* ��ȡ�����ļ���
	string strFile = "";
	try {
		strFile = objCmdLineParser.get<string>(0);				
	}catch(Exception& ex){}	
	if (strFile.empty())
	{
		cout << "Please input the image name or rtsp stream address." << endl;

		return 0;
	}
	
	//* ��ȡģ������
	string strModel = "VGG";
	if (objCmdLineParser.has("model"))
	{
		strModel = objCmdLineParser.get<string>("model");
	}

	//* ����ģ�����õ��ò�ͬ�ĺ������д���
	if (strModel == "VGG")
	{	
		__VGGModelClassifier(strFile, (BOOL)objCmdLineParser.get<bool>("picture"));
	}
	else if (strModel == "Yolo2")
	{
		
	}
	else if (strModel == "Yolo3")
	{
		
	}
	else 
	{
		cout << "Invalid pre train model name: " << strModel.c_str() << endl;
	}

    return 0;
}

