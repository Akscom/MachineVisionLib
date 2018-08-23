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
#include "MVLVideo.h"
#include "WhoYouAre.h"

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

void __PredictThroughVideoData(cv::Mat &mVideoData, DWORD64 dw64InputParam)
{
	FaceDatabase *pface_db = (FaceDatabase*)dw64InputParam;
	
	pface_db->pvideo_predict->Predict(mVideoData);

	imshow("Camera video", mVideoData);
}

//* ͨ������ͷ����ƵԤ��
template <typename DType>
static void __VideoPredict(DType dtVideoSrc, BOOL blIsNeedToReplay)
{
	FaceDatabase face_db;
	face_db.pvideo_predict = new FaceDatabase::VideoPredict(&face_db);

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

	cv2shell::CV2ShowVideo(dtVideoSrc, __PredictThroughVideoData, (DWORD64)&face_db, blIsNeedToReplay);
}

static void __NetcamDispPreprocessor(Mat& mVideoFrame, void *pvParam, UINT unCurFrameIdx)
{	
	static UINT unFaceID = 0;

	//* ����Ԥѵ��ģ�͵�Size������ͼ�������300 x 300���󲿷�ͼ�������16��9��4��3�����ٵȱ����ģ�Ϊ�˱���ͼ����Σ�����ֻ�ý�ȡͼ���м�������������������ʶ������
	Mat mROI;
	if (mVideoFrame.cols > mVideoFrame.rows)
		mROI = mVideoFrame(Rect((mVideoFrame.cols - mVideoFrame.rows) / 2, 0, mVideoFrame.rows, mVideoFrame.rows));
	else if (mVideoFrame.cols < mVideoFrame.rows)
		mROI = mVideoFrame(Rect(0, (mVideoFrame.rows - mVideoFrame.cols) / 2, mVideoFrame.cols, mVideoFrame.cols));
	else
		mROI = mVideoFrame;	

	PST_VLCPLAYER_FCBDISPPREPROC_PARAM pstParam = (PST_VLCPLAYER_FCBDISPPREPROC_PARAM)pvParam;
	Mat &matFaces = cv2shell::FaceDetect(*pstParam->pobjDNNNet, mROI);
	if (matFaces.empty())
		return;	

	//cv2shell::MarkFaceWithRectangle(mROI, matFaces, 0.8);

	//* ȡ������������֮
	for (INT i = 0; i < matFaces.rows; i++)
	{ 
		FLOAT flConfidenceVal = matFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < FACE_DETECT_MIN_CONFIDENCE_THRESHOLD)
			continue;

		INT nCurLTX = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * mROI.cols) - 50;
		INT nCurLTY = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * mROI.rows) - 50;
		INT nCurRBX = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * mROI.cols) + 50;
		INT nCurRBY = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * mROI.rows) + 50;

		cout << nCurLTX << " " << nCurLTY << " " << nCurRBX << " " << nCurRBY << endl;

		//* ��������
		Rect rectObj(nCurLTX, nCurLTY, (nCurRBX - nCurLTX), (nCurRBY - nCurLTY));
		rectangle(mROI, rectObj, Scalar(0, 255, 0));

		//* ����
		unordered_map<UINT, ST_FACE>::iterator iterFace;
		for (iterFace = pstParam->pmapFaces->begin(); iterFace != pstParam->pmapFaces->end(); iterFace++)
		{
			INT nOldFaceLTX = iterFace->second.nLeftTopX;
			INT nOldFaceLTY = iterFace->second.nLeftTopY;
			INT nOldFaceRBX = iterFace->second.nRightBottomX;
			INT nOldFaceRBY = iterFace->second.nRightBottomY;

			//* ȫ�������������Ϊ��ͬһ����������Ҫ�������
			if (abs(nCurLTX - nOldFaceLTX) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
				abs(nCurLTY - nOldFaceLTY) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
				abs(nCurRBX - nOldFaceRBX) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
				abs(nCurRBY - nOldFaceRBY) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE && 
				(unCurFrameIdx - iterFace->second.unFrameIdx) < FACE_DISAPPEAR_FRAME_NUM)
			{
				iterFace->second.unFrameIdx = unCurFrameIdx;

				iterFace->second.nLeftTopX = nCurLTX;
				iterFace->second.nLeftTopY = nCurLTY;
				iterFace->second.nRightBottomX = nCurRBX;
				iterFace->second.nRightBottomY = nCurRBY;

				//* ������ͼ������ڴ�
				Mat mFace = mROI(Rect(nCurLTX, nCurLTY, nCurRBX - nCurLTX, nCurRBY - nCurLTY));
				EnterThreadMutex(pstParam->pthMutexMapFaces);
				{					
					mFace.copyTo(iterFace->second.mFace);
				}
				ExitThreadMutex(pstParam->pthMutexMapFaces);
			}
			else
			{
				ST_FACE stFace;

				stFace.unFaceID = unFaceID++;

				stFace.unFrameIdx = unCurFrameIdx;

				stFace.nLeftTopX = nCurLTX;
				stFace.nLeftTopY = nCurLTY;
				stFace.nRightBottomX = nCurRBX;
				stFace.nRightBottomY = nCurRBY;

				//* ������ͼ������ڴ�
				Mat mFace = mROI(Rect(nCurLTX, nCurLTY, nCurRBX - nCurLTX, nCurRBY - nCurLTY));
				mFace.copyTo(stFace.mFace);

				EnterThreadMutex(pstParam->pthMutexMapFaces);
				{
					(*pstParam->pmapFaces)[stFace.unFaceID] = stFace;
				}
				ExitThreadMutex(pstParam->pthMutexMapFaces);
			}
		}		
	}

	//* ������ɾ���Ѿ���ʧ����������
	EnterThreadMutex(pstParam->pthMutexMapFaces);
	{
		unordered_map<UINT, ST_FACE>::iterator iterFace;
		for (iterFace = pstParam->pmapFaces->begin(); iterFace != pstParam->pmapFaces->end();)
		{
			//* ��ʱ��ɾ��֮
			if ((unCurFrameIdx - iterFace->second.unFrameIdx) > FACE_DISAPPEAR_FRAME_NUM)
			{
				pstParam->pmapFaces->erase(iterFace++);	//* ע�⣬����Ҫɾ��Ԫ�أ����ֻ��������++����for��++�Ļ��ᵼ�µ�ǰԪ����ɾ�����޷�����++
														//* ���������ȱ���iterFace��ǰָ��Ȼ��++��Ȼ���ٽ�����ĵ�ǰָ�򴫵ݸ�erase()����
			}
			else
				iterFace++;	//* ��һ��
		}
	}
	ExitThreadMutex(pstParam->pthMutexMapFaces);


}

//* ͨ����������ͷʶ��
static void __NetcamFaceHandler(const CHAR *pszURL, BOOL blIsNeedToReplay)
{
	unordered_map<UINT, ST_FACE> mapFaces;	//* �����������⵽�Ķ���������Ϣ�洢��map��
	THMUTEX thMutexMapFaces;

	VLCVideoPlayer objVideoPlayer;

	cvNamedWindow(pszURL, CV_WINDOW_AUTOSIZE);
	objVideoPlayer.OpenVideoFromeRtsp(pszURL, __NetcamDispPreprocessor, pszURL);

	if (!objVideoPlayer.start())
	{
		cout << "start rtsp stream failed!" << endl;
		return;
	}

	//* ��ʼ��DNN�����������
	Net dnnNet = cv2shell::InitFaceDetectDNNet();

	//* ���ò�������ʾԤ����������ڲ���
	ST_VLCPLAYER_FCBDISPPREPROC_PARAM stFCBDispPreprocParam;
	stFCBDispPreprocParam.pobjDNNNet = &dnnNet;
	stFCBDispPreprocParam.pmapFaces = &mapFaces;
	InitThreadMutex(&thMutexMapFaces);	//* ��mapFaces�ķ��ʱ�����б�������Ϊ���������̲߳�������
	stFCBDispPreprocParam.pthMutexMapFaces = &thMutexMapFaces;
	objVideoPlayer.SetDispPreprocessorInputParam(&stFCBDispPreprocParam);
	
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
			cout << "����������ͷ�����ӶϿ����޷�������ȡʵʱ��Ƶ����" << endl;
			break;
		}	

		//* ʶ������
		Mat mFrame = objVideoPlayer.GetNextFrame();
		if (mFrame.empty())				
			continue;		
	}

	//* ��ʵ�������������Ҳ���ԣ�VLCVideoPlayer��������������������õ�
	objVideoPlayer.stop();

	UninitThreadMutex(&thMutexMapFaces);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//unordered_map<UINT, string> mapTest;
	//mapTest[0] = "0-test";
	//mapTest[2] = "2-test";
	//mapTest[1] = "1-test";	
	//mapTest[3] = "3-test";
	//mapTest[4] = "4-test";
	//unordered_map<UINT, string>::iterator iter;
	//for (iter = mapTest.begin(); iter != mapTest.end(); iter++)
	//{
	//	cout << iter->first << " " << iter->second << endl;
	//}

	//cout << "Before Del: " << mapTest.size() << endl;

	//for (iter = mapTest.begin(); iter != mapTest.end(); /*iter++*/)
	//{
	//	mapTest.erase(iter++);
	//}

	//cout << "After Del: " << mapTest.size() << endl;

	//exit(-1);

	if (argc != 3 && argc != 4 && argc != 2)
	{
		cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
		cout << argv[0] << " predict [Img Path]" << endl;
		cout << argv[0] << " video [camera number, If not specified, the default value is 0]" << endl;
		cout << argv[0] << " video [video file path]" << endl;
		cout << argv[0] << " netcam [rtsp url]" << endl;

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
	else if (string("camera") == strOptType.toLowerCase())
	{
		INT nCameraID = 0;
		if (argc == 3)
			nCameraID = atoi(argv[2]);

		__VideoPredict(nCameraID, TRUE);
	}
	else if (string("video") == strOptType.toLowerCase())
	{
		if(argc != 3)
			goto __lblUsage;

		CHAR szVideoFile[MAX_PATH];
		sprintf_s(szVideoFile, "%s", argv[2]);

		__VideoPredict((const CHAR*)szVideoFile, FALSE);
	}
	else if (string("netcam") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__NetcamFaceHandler((const CHAR*)szURL, FALSE);
	}
	else
		goto __lblUsage;

	return 0;

__lblUsage:	
	cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
	cout << argv[0] << " predict [Img Path]" << endl;
	cout << argv[0] << " video [camera number, If not specified, the default value is 0]" << endl;
	cout << argv[0] << " video [video file path]" << endl;
	cout << argv[0] << " netcam [rtsp url]" << endl;

    return 0;
}

