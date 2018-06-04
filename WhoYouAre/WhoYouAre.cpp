// WhoYouAre.cpp : �������̨Ӧ�ó������ڵ㡣
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
#include "FaceRecognition.h"

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

//* �������
static BOOL __AddFace(const CHAR *pszFaceImgFile, const CHAR *pezPersonName)
{
	FaceDatabase face_db;

	//* �����Ƿ��Ѿ���ӵ����ݿ���
	if (face_db.IsPersonAdded(pezPersonName))
	{
		cout << pezPersonName << " has been added to the face database." << endl;

		return TRUE;
	}


	//* ����������ȡ����
	if (!face_db.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;

		return FALSE;
	}

	if (face_db.AddFace(pszFaceImgFile, pezPersonName))
	{
		cout << pezPersonName << " was successfully added to the face database." << endl;

		return TRUE;
	}		

	return FALSE;
}

//* ͨ��ͼƬԤ������
static void __PicturePredict(const CHAR *pszFaceImgFile)
{
	FaceDatabase face_db;

	if (!face_db.LoadFaceData())
	{
		cout << "Load face data failed, the process will be exited!" << endl;
		return;
	}

	if (!face_db.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;
		return;
	}

	cout << "Start find ..." << endl;
	string strPersonName;
	DOUBLE dblSimilarity = face_db.Predict(pszFaceImgFile, strPersonName);
	if (dblSimilarity > 0.55)
	{
		cout << "Found a matched face: ��" << strPersonName << "��, the similarity is " << dblSimilarity << endl;
	}
	else
		cout << "Not found matched face." << endl;

	cout << "Stop find." << endl;
}

void __PredictThroughVideoData(cv::Mat &mVideoData, UINT unInputParam)
{
	//cv2shell::MarkFaceWithRectangle(mVideoData);

	cout << "1.4#####################unInputParam: " << unInputParam << endl;
	FaceDatabase *pface_db = (FaceDatabase*)unInputParam;
	cout << "1.5#####################pface_db " << pface_db << endl;

	string strPersonName;
	cout << "2#####################pface_db->pvideo: " << pface_db->pvideo << endl;
	cin.get();
	DOUBLE dblSimilarity = pface_db->pvideo->Predict(mVideoData, strPersonName);
	if (dblSimilarity > 0.85)
	{
		cout << "Found a matched face: ��" << strPersonName << "��, the similarity is " << dblSimilarity << endl;
	}
	else
		cout << "Not found matched face." << endl;

	imshow("Camera video", mVideoData);
}

//* ͨ��ʵʱ��ƵԤ��
static void __VideoPredict(INT nCameraID)
{
	FaceDatabase face_db;

	if (!face_db.LoadFaceData())
	{
		cout << "Load face data failed, the process will be exited!" << endl;
		return;
	}

	if (!face_db.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;
		return;
	}

	FaceDatabase *pface_db = &face_db;
	cout << "1.1#####################pface_db: " << pface_db << " " << (UINT)&face_db << endl;
	cout << "1.2#####################pface_db->pvideo: " << pface_db->pvideo << endl;

	//cv2shell::CV2ShowVideo(nCameraID, __PredictThroughVideoData, (UINT)&face_db);
	cv2shell::CV2ShowVideo("C:\\OpenCV3.4\\opencv\\sources\\samples\\data\\Megamind.avi", __PredictThroughVideoData, (UINT)&face_db);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3 && argc != 4 && argc != 2)
	{
		cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
		cout << argv[0] << " predict [Img Path]" << endl;
		cout << argv[0] << " video [camera number, If not specified, the default value is 0]" << endl;

		return -1;
	}

	String strOptType(argv[1]);
	if (String("add") == strOptType.toLowerCase())
	{
		return __AddFace(argv[2], argv[3]);		
	}
	else if (string("predict") == strOptType.toLowerCase())
	{
		__PicturePredict(argv[2]);		
	}
	else if (string("video") == strOptType.toLowerCase())
	{
		INT nCameraID = 0;
		if(argc == 3)
			nCameraID = atoi(argv[2]);

		__VideoPredict(nCameraID);
	}
	else
	{
		cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
		cout << argv[0] << " predict [Img Path]" << endl;
		cout << argv[0] << " video [camera number, If not specified, the default value is 0]" << endl;
	}

    return 0;
}

