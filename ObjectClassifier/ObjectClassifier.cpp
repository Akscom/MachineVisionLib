// ObjectClassifier.cpp : �������̨Ӧ�ó������ڵ㡣
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

//* VGGģ�ͷ�����
static void __VGGModelClassifier(const CHAR *pszFile, BOOL blIsPicture)
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

	}
	else
	{
		//* ������ʵʱ��Ƶ��������Ƶ�ļ��������ļ���ǰ׺�жϼ���
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
		__VGGModelClassifier(strFile.c_str(), (BOOL)objCmdLineParser.get<bool>("picture"));
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

