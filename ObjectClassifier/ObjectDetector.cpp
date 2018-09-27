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
	"{model   m   | VGG | which model to use, value is VGG��Yolo2��Yolo2-Tiny��Yolo2-VOC��Yolo2-Tiny-VOC��MobileNetSSD}"
	"{picture p   |     | image file detector, non video detector}"
};

//* ��ʾ��⵽���壨ͼƬ��
static void __ShowDetectedObjectsOfPicture(Mat mShowImg, vector<RecogCategory>& vObjects, Net& objDNNNet)
{
	//* ʶ��ķѵ�ʱ��	
	cout << "detection time: " << GetTimeSpentInNetDetection(objDNNNet) << " ms" << endl;

	MarkObjectWithRectangle(mShowImg, vObjects);

	Mat mDstImg = mShowImg;

	INT nPCScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	INT nPCScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	if (mShowImg.cols > nPCScreenWidth || mShowImg.rows > nPCScreenHeight)
	{
		resize(mShowImg, mDstImg, Size((mShowImg.cols * 2) / 3, (mShowImg.rows * 2) / 3), 0, 0, INTER_AREA);
	}

	imshow("Object Detector", mDstImg);
	waitKey(0);
}

//* ��ʾ��⵽���壨��Ƶ��
static void __ShowDetectedObjectsOfVideo(Mat mShowFrame, const string& strShowWindowsName, vector<RecogCategory>& vObjects, Net& objDNNNet)
{
	DOUBLE dblTimeSpent = GetTimeSpentInNetDetection(objDNNNet);
	ostringstream oss;
	oss << "The time spent: " << dblTimeSpent << " ms";
	putText(mShowFrame, oss.str(), Point(mShowFrame.cols - 250, 20), 0, 0.5, Scalar(0, 0, 255), 2);

	MarkObjectWithRectangle(mShowFrame, vObjects);

	imshow(strShowWindowsName, mShowFrame);
}

//* ������Ƶ
static BOOL __PlayVideo(VLCVideoPlayer& objVideoPlayer, const string& strFile, BOOL blIsVideoFile)
{
	//* ����һ��VLC�����������������Ŵ���	
	cvNamedWindow(strFile.c_str(), CV_WINDOW_AUTOSIZE);

	if (blIsVideoFile)
		objVideoPlayer.OpenVideoFromFile(strFile.c_str(), NULL, strFile.c_str());
	else
		objVideoPlayer.OpenVideoFromeRtsp(strFile.c_str(), NULL, strFile.c_str(), 500);

	//* ��ʼ����
	if (!objVideoPlayer.start())
	{
		cout << "Video playback failed!" << endl;
		return FALSE;
	}

	return TRUE;
}

//* VGGģ��֮ͼƬ������
static void __VGGModelDetectorOfPicture(Net& objDNNNet, vector<string>& vClassNames, const string& strFile)
{
	Mat mSrcImg = imread(strFile);

	vector<RecogCategory> vObjects;
	ObjectDetect(mSrcImg, objDNNNet, vClassNames, vObjects);

	__ShowDetectedObjectsOfPicture(mSrcImg, vObjects, objDNNNet);
}

//* VGGģ�ͷ�����
static void __VGGModelDetector(string& strFile, BOOL blIsPicture)
{
	vector<string> vClassNames;
	Net objDNNNet = InitLightDetector(vClassNames);
	if (objDNNNet.empty())
	{
		cout << "The initialization lightweight classifier failed, probably because the model file or voc.names file was not found." << endl;
		return;
	}
	
	if (blIsPicture)
	{
		__VGGModelDetectorOfPicture(objDNNNet, vClassNames, strFile);
	}
	else
	{
		cout << "VGG model does not support video classifier." << endl;
	}
}

//* Yolo2ģ��֮ͼƬ������
static void __Yolo2ModelDetectorOfPicture(Net& objDNNNet, vector<string>& vClassNames, const string& strFile)
{
	Mat mSrcImg = imread(strFile);

	vector<RecogCategory> vObjects;
	Yolo2ObjectDetect(mSrcImg, objDNNNet, vClassNames, vObjects);

	__ShowDetectedObjectsOfPicture(mSrcImg, vObjects, objDNNNet);
}

//* Yolo2ģ��֮��Ƶ������������blIsVideoFileΪ"��"������ζ����һ����Ƶ�ļ���"��"����ζ����rtsp��
static void __Yolo2ModelDetectorOfVideo(Net& objDNNNet, vector<string>& vClassNames, const string& strFile, BOOL blIsVideoFile)
{
	//* ����һ��VLC������������������
	VLCVideoPlayer objVideoPlayer;
	if (!__PlayVideo(objVideoPlayer, strFile, blIsVideoFile))
		return;

	CHAR bKey;
	BOOL blIsPaused = FALSE;
	while (TRUE)
	{
		bKey = waitKey(10);
		if (27 == bKey)
			break;

		//* �ո���ͣ�������ͣ������ͣ���������Ỻ��һ��ʱ���ʵʱ��Ƶ��������֮����ָ����ź���Ƶ�뵱ǰʱ�̴���һ����ʱ���
		if (' ' == bKey)
		{
			blIsPaused = !blIsPaused;
			objVideoPlayer.pause(blIsPaused);
		}

		if (objVideoPlayer.IsPlayEnd())
		{
			if (!blIsVideoFile)
				cout << "The connection with the RTSP stream is disconnected, unable to get the next frame, the process will exit." << endl;

			break;
		}

		Mat mSrcFrame = objVideoPlayer.GetNextFrame();
		if (mSrcFrame.empty())
			continue;

		vector<RecogCategory> vObjects;
		Yolo2ObjectDetect(mSrcFrame, objDNNNet, vClassNames, vObjects);

		__ShowDetectedObjectsOfVideo(mSrcFrame, strFile, vObjects, objDNNNet);
	}	

	//* ��ʵ�������������Ҳ���ԣ�VLCVideoPlayer��������������������õ�
	objVideoPlayer.stop();

	cv::destroyAllWindows();
}

static void __Yolo2ModelDetector(string& strFile, BOOL blIsPicture, ENUM_YOLO2_MODEL_TYPE enumYolo2Type)
{
	vector<string> vClassNames;
	Net objDNNNet = InitYolo2Detector(vClassNames, enumYolo2Type);
	if (objDNNNet.empty())
	{
		cout << "The initialization yolo2 classifier failed, probably because the model file or calss names file was not found." << endl;
		return;
	}

	if (blIsPicture)
	{
		__Yolo2ModelDetectorOfPicture(objDNNNet, vClassNames, strFile);
	}
	else
	{
		//* ������ʵʱ��Ƶ��������Ƶ�ļ��������ļ���ǰ׺�жϼ���
		std::transform(strFile.begin(), strFile.end(), strFile.begin(), ::tolower);		
		if (strFile.find("rtsp:", 0) == 0)
		{					
			__Yolo2ModelDetectorOfVideo(objDNNNet, vClassNames, strFile, FALSE);
		}
		else
		{			
			__Yolo2ModelDetectorOfVideo(objDNNNet, vClassNames, strFile, TRUE);
		}
	}
}

//* MobileNetSSDԤѵ��ģ�ͼ����
static void __MobNetSSDModelDetector(string& strFile, BOOL blIsPicture)
{

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
	} catch(Exception& ex) {
		cout << ex.what() << endl;
	}	
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
		__VGGModelDetector(strFile, (BOOL)objCmdLineParser.get<bool>("picture"));
	}
	else if (strModel == "Yolo2")
	{
		__Yolo2ModelDetector(strFile, (BOOL)objCmdLineParser.get<bool>("picture"), YOLO2);
	}
	else if (strModel == "Yolo2-Tiny")
	{
		__Yolo2ModelDetector(strFile, (BOOL)objCmdLineParser.get<bool>("picture"), YOLO2_TINY);
	}
	else if (strModel == "Yolo2-VOC")
	{
		__Yolo2ModelDetector(strFile, (BOOL)objCmdLineParser.get<bool>("picture"), YOLO2_VOC);
	}
	else if (strModel == "Yolo2-Tiny-VOC")
	{
		__Yolo2ModelDetector(strFile, (BOOL)objCmdLineParser.get<bool>("picture"), YOLO2_TINY_VOC);
	}
	else if (strModel == "MobileNetSSD")
	{
		__MobNetSSDModelDetector(strFile, (BOOL)objCmdLineParser.get<bool>("picture"));
	}
	else 
	{
		cout << "Invalid pre train model name: " << strModel.c_str() << endl;
	}

    return 0;
}

