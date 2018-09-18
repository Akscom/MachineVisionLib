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
	FaceDatabase objFaceDB;

	//* �����Ƿ��Ѿ���ӵ����ݿ���
	if (objFaceDB.IsPersonAdded(pezPersonName))
	{
		cout << pezPersonName << " has been added to the face database." << endl;

		return TRUE;
	}


	//* ����������ȡ����
	if (!objFaceDB.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;

		return FALSE;
	}

	if (objFaceDB.AddFace(pszFaceImgFile, pezPersonName))
	{
		cout << pezPersonName << " was successfully added to the face database." << endl;

		return TRUE;
	}		

	return FALSE;
}

//* ͨ��ͼƬԤ������
static void __PictureFacePredict(const CHAR *pszFaceImgFile)
{
	PerformanceTimer objPerformTimer;	//* ��ʱ��
	FaceDatabase objFaceDB;

	if (!objFaceDB.LoadFaceData())
	{
		cout << "Load face data failed, the process will be exited!" << endl;
		return;
	}

	if (!objFaceDB.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;
		return;
	}

	cout << "Start find ..." << endl;
	string strPersonName;	
	objPerformTimer.start();	//* ��ʼ��ʱ
	DOUBLE dblSimilarity = objFaceDB.Predict(pszFaceImgFile, strPersonName);	
	cout << "Time spent " << objPerformTimer.end() / 1000 << "ms." << endl;
	if (dblSimilarity > 0.55)
	{
		cout << "Found a matched face: ��" << strPersonName << "��, the similarity is " << dblSimilarity << endl;
	}
	else
		cout << "Not found matched face." << endl;

	cout << "Stop find." << endl;
}

static void __VideoFacePredict(Mat& mVideoFrame, PST_PLAYER_FCBDISPPREPROC_PARAM pstParam)
{
	//* ����Ԥѵ��ģ�͵�Size������ͼ�������300 x 300���󲿷�ͼ�������16��9��4��3�����ٵȱ����ģ�Ϊ�˱���ͼ����Σ�����ֻ�ý�ȡͼ���м�������������������ʶ������
	Mat mROI;
	if (mVideoFrame.cols > mVideoFrame.rows)
		mROI = mVideoFrame(Rect((mVideoFrame.cols - mVideoFrame.rows) / 2, 0, mVideoFrame.rows, mVideoFrame.rows));
	else if (mVideoFrame.cols < mVideoFrame.rows)
		mROI = mVideoFrame(Rect(0, (mVideoFrame.rows - mVideoFrame.cols) / 2, mVideoFrame.cols, mVideoFrame.cols));
	else
		mROI = mVideoFrame;

	Mat &matFaces = cv2shell::FaceDetect(*pstParam->pobjDNNNet, mROI);
	if (matFaces.empty())
		return;

	Mat mFaceGray;
	cvtColor(mROI, mFaceGray, CV_BGR2GRAY);

	//* ʶ��ÿ������
	for (INT i = 0; i < matFaces.rows; i++)
	{
		FLOAT flConfidenceVal = matFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < FACE_DETECT_MIN_CONFIDENCE_THRESHOLD)
			continue;

		INT nCurLTX = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * mROI.cols);
		INT nCurLTY = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * mROI.rows);
		INT nCurRBX = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * mROI.cols);
		INT nCurRBY = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * mROI.rows);

		Face objFace(nCurLTX, nCurLTY, nCurRBX, nCurRBY);
		string strPersonName;
		DOUBLE dblConfidence = pstParam->pobjFaceDB->o_pobjVideoPredict->Predict(mFaceGray, objFace, pstParam->pobjFaceDB->o_objShapePredictor, strPersonName);
		if (dblConfidence > 0.8f)
		{
			//* ��������
			Rect objRect(nCurLTX, nCurLTY, nCurRBX - nCurLTX, nCurRBY - nCurLTY);
			rectangle(mROI, objRect, Scalar(0, 255, 0));

			INT nBaseLine = 0;
			String strPersonLabel;
			string strConfidenceLabel;
			Rect rect;

			//* ��������Ϳ��Ŷ�
			strPersonLabel = "Name: " + strPersonName;
			strConfidenceLabel = "Confidence: " + static_cast<ostringstream*>(&(ostringstream() << dblConfidence))->str();

			Size personLabelSize = getTextSize(strPersonLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
			Size confidenceLabelSize = getTextSize(strConfidenceLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
			rect = Rect(Point(nCurLTX, nCurLTY - (personLabelSize.height + confidenceLabelSize.height + 3)),
				Size(personLabelSize.width > confidenceLabelSize.width ? personLabelSize.width : confidenceLabelSize.width,
					personLabelSize.height + confidenceLabelSize.height + nBaseLine + 3));
			cv::rectangle(mROI, rect, Scalar(255, 255, 255), CV_FILLED);
			putText(mROI, strPersonLabel, Point(nCurLTX, nCurLTY - confidenceLabelSize.height - 3), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
			putText(mROI, strConfidenceLabel, Point(nCurLTX, nCurLTY), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
		}
	}
}

static void __FCBOCVPlayerFacePredict(Mat& mVideoFrame, DWORD64 dw64InputParam)
{
	PST_PLAYER_FCBDISPPREPROC_PARAM pstParam= (PST_PLAYER_FCBDISPPREPROC_PARAM)dw64InputParam;
	PerformanceTimer objPerformTimer;
	objPerformTimer.start();
	__VideoFacePredict(mVideoFrame, pstParam);
	cout << "Time spent of predict: " << objPerformTimer.end() / 1000 << "ms" << endl;

	imshow("Opencv Player", mVideoFrame);
}

//* ͨ��OpenCV�������Ի�ȡ��������ͷʵʱ��Ƶ����Ƶ�ļ�����Ԥ��
template <typename DType>
static void __OCVPlayerFacePredict(DType dtVideoSrc, BOOL blIsNeedToReplay)
{
	FaceDatabase objFaceDB;

	if (!objFaceDB.LoadDLIB68FaceLandmarksModel("C:\\MVLThirdPartyLib\\dlib-19.10\\models\\shape_predictor_68_face_landmarks.dat"))
		return;

	if (!objFaceDB.LoadFaceData())
	{
		cout << "Load face data failed, the process will be exited!" << endl;
		return;
	}

	if (!objFaceDB.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;
		return;
	}

	Net objDNNNet = cv2shell::InitFaceDetectDNNet();

	objFaceDB.o_pobjVideoPredict = new FaceDatabase::VideoPredict(&objFaceDB, TRUE);

	ST_PLAYER_FCBDISPPREPROC_PARAM stParam;
	stParam.pobjDNNNet = &objDNNNet;
	stParam.pobjFaceDB = &objFaceDB;
	cv2shell::CV2ShowVideo(dtVideoSrc, __FCBOCVPlayerFacePredict, (DWORD64)&stParam, blIsNeedToReplay);
}

//* ��VLC���������ŵ���Ƶ���ҵ�����
static Mat __VLCPlayerFindFace(Net& objDNNNet, Mat& mFrame, Face& objFace)
{
	//* ����Ԥ��ģ��Ҫ���ȡһ��ȱߵľ�������
	Mat mROI;
	if (mFrame.cols > mFrame.rows)
		mROI = mFrame(Rect((mFrame.cols - mFrame.rows) / 2, 0, mFrame.rows, mFrame.rows));
	else if (mFrame.cols < mFrame.rows)
		mROI = mFrame(Rect(0, (mFrame.rows - mFrame.cols) / 2, mFrame.cols, mFrame.cols));
	else
		mROI = mFrame;

	//* �������
	Mat &matFaces = cv2shell::FaceDetect(objDNNNet, mROI);
	if (matFaces.empty())
		return mROI;

	//* ��Ȼ��ѭ��������Ȼ��ֻȡ��һ����Ч����
	FLOAT flConfidenceVal;
	for (INT i = 0; i < matFaces.rows; i++)	
	{ 
		flConfidenceVal = matFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < FACE_DETECT_MIN_CONFIDENCE_THRESHOLD)
			continue;

		objFace.o_nLeftTopX = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * mROI.cols);
		objFace.o_nLeftTopY = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * mROI.rows);
		objFace.o_nRightBottomX = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * mROI.cols);
		objFace.o_nRightBottomY = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * mROI.rows);
		objFace.o_flConfidenceVal = flConfidenceVal;
		
		break;
	}

	return mROI;
}

//* ��__VLCPlayerFindFace()�����ҵ���������ӵ�������
static void __VLCPlayerAddFace(Mat& mROI, Face& objFace, FaceDatabase& objFaceDB)
{
	static UINT unFaceID = 0;

	ostringstream oss, oss_name;
	oss << "Face-" << unFaceID << ".jpg";
	oss_name << "Face-" << unFaceID;

	Mat mFace = mROI(Rect(objFace.o_nLeftTopX, objFace.o_nLeftTopY, objFace.o_nRightBottomX - objFace.o_nLeftTopX, objFace.o_nRightBottomY - objFace.o_nLeftTopY));
	imshow(oss.str(), mFace);

	if (13 == waitKey(0))	//* �����س������֮������ȡ��
	{
		imwrite(oss.str(), mFace);

		if (objFaceDB.IsPersonAdded(oss_name.str()))
			cout << oss_name.str() << " has been added to the face database." << endl;
		else
		{
			Mat mFaceGray;
			cvtColor(mROI, mFaceGray, CV_BGR2GRAY);

			if (objFaceDB.AddFace(mFaceGray, objFace, oss_name.str()))
				cout << oss_name.str() << " was successfully added to the face database." << endl;
			else
				cout << oss_name.str() << " was not added to the face database. " << endl;
		}

		unFaceID++;
	}

	destroyWindow(oss.str());
}

//* VLC������֮�ص�����������ʶ��ģ��
static void __FCBVLCPlayerFacePredict(Mat& mVideoFrame, void *pvParam, UINT unCurFrameIdx)
{	
	PST_PLAYER_FCBDISPPREPROC_PARAM pstParam = (PST_PLAYER_FCBDISPPREPROC_PARAM)pvParam;
	__VideoFacePredict(mVideoFrame, pstParam);
}

//* VLC��������������ģ����ں���
static void __VLCPlayerFaceHandler(const CHAR *pszURL, BOOL blIsCatchFace, BOOL blIsRTSPStream)
{
	FaceDatabase objFaceDB;
	
	if (!blIsCatchFace)
	{
		if (!objFaceDB.LoadFaceData())
		{
			cout << "Load face data failed, the process will be exited!" << endl;
			return;
		}
	}

	if (!objFaceDB.LoadDLIB68FaceLandmarksModel("C:\\MVLThirdPartyLib\\dlib-19.10\\models\\shape_predictor_68_face_landmarks.dat"))
		return;

	if (!objFaceDB.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;
		return;
	}

	objFaceDB.o_pobjVideoPredict = new FaceDatabase::VideoPredict(&objFaceDB);

	//* ����һ��VLC������
	VLCVideoPlayer objVideoPlayer;

	cvNamedWindow(pszURL, CV_WINDOW_AUTOSIZE);

	if (blIsRTSPStream)
	{
		if(!blIsCatchFace)
			objVideoPlayer.OpenVideoFromeRtsp(pszURL, __FCBVLCPlayerFacePredict, pszURL, 1000);
		else
			objVideoPlayer.OpenVideoFromeRtsp(pszURL, NULL, NULL, 500);
	}
	else
	{
		if(!blIsCatchFace)
			objVideoPlayer.OpenVideoFromFile(pszURL, __FCBVLCPlayerFacePredict, pszURL);
		else
			objVideoPlayer.OpenVideoFromFile(pszURL, NULL, NULL);
	}

	//* ��ʼ��DNN�����������
	Net objDNNNet = cv2shell::InitFaceDetectDNNet();

	if (!blIsCatchFace)
	{
		//* ���ò�������ʾԤ����������ڲ���
		ST_PLAYER_FCBDISPPREPROC_PARAM stFCBDispPreprocParam;
		stFCBDispPreprocParam.pobjDNNNet = &objDNNNet;
		stFCBDispPreprocParam.pobjFaceDB = &objFaceDB;
		objVideoPlayer.SetDispPreprocessorInputParam(&stFCBDispPreprocParam);
	}

	//* ��ʼ����
	if (!objVideoPlayer.start())
	{
		cout << "start rtsp stream failed!" << endl;
		return;
	}
	
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
			if (blIsRTSPStream)
				cout << "The connection with the RTSP stream is disconnected, unable to get the next frame, the process will exit." << endl;

			break;
		}	

		if (blIsCatchFace)
		{
			Mat mSrcFrame = objVideoPlayer.GetNextFrame();
			if (mSrcFrame.empty())
				continue;

			Mat mFrame = mSrcFrame.clone();

			Face objFace;
			objFace.o_flConfidenceVal = 0.0f;
			Mat mROI = __VLCPlayerFindFace(objDNNNet, mFrame, objFace);
			if (objFace.o_flConfidenceVal > 0.0f)
			{
				//* �س��������
				if (bKey == 13)				
					__VLCPlayerAddFace(mROI, objFace, objFaceDB);				

				//* �������
				if (objFace.o_flConfidenceVal > FACE_DETECT_MIN_CONFIDENCE_THRESHOLD)
				{
					Rect objRect(objFace.o_nLeftTopX, objFace.o_nLeftTopY, objFace.o_nRightBottomX - objFace.o_nLeftTopX, objFace.o_nRightBottomY - objFace.o_nLeftTopY);
					rectangle(mROI, objRect, Scalar(0, 255, 0));
				}
			}

			imshow(pszURL, mFrame);
		}		
	}

	//* ��ʵ�������������Ҳ���ԣ�VLCVideoPlayer��������������������õ�
	objVideoPlayer.stop();		

	destroyAllWindows();
}

int _tmain(int argc, _TCHAR* argv[])
{	
	if (argc != 3 && argc != 4 && argc != 2)
	{
		cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
		cout << argv[0] << " predict [Img Path]" << endl;
		cout << argv[0] << " ocvcamera [camera number, If not specified, the default value is 0] - use opencv player" << endl;
		cout << argv[0] << " ocvvideo [video file path] - use opencv player" << endl;
		cout << argv[0] << " vlcvideo_rtsp_predict [rtsp url] - Use the VLCVideoPlayer module to read the RTSP flow to identify faces" << endl;
		cout << argv[0] << " vlcvideo_predict [video file path] - Use the VLCVideoPlayer module to read the video file to identify faces" << endl;
		cout << argv[0] << " vlcvideo_rtsp_catchface [rtsp url] - Using VLCVideoPlayer module to read RTSP flow registration face" << endl;
		cout << argv[0] << " vlcvideo_catchface [video file path] - Using VLCVideoPlayer module to read the video file registration face" << endl;

		return -1;
	}	

	String strOptType(argv[1]);
	if (String("add") == strOptType.toLowerCase())
	{
		return __AddFace(argv[2], argv[3]);
	}
	else if (string("predict") == strOptType.toLowerCase())
	{
		__PictureFacePredict(argv[2]);
	}
	else if (string("ocvcamera") == strOptType.toLowerCase())
	{
		INT nCameraID = 0;
		if (argc == 3)
			nCameraID = atoi(argv[2]);

		__OCVPlayerFacePredict(nCameraID, TRUE);
	}
	else if (string("ocvvideo") == strOptType.toLowerCase())
	{
		if(argc != 3)
			goto __lblUsage;

		CHAR szVideoFile[MAX_PATH];
		sprintf_s(szVideoFile, "%s", argv[2]);

		__OCVPlayerFacePredict((const CHAR*)szVideoFile, FALSE);
	}
	else if (string("vlcvideo_rtsp_predict") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__VLCPlayerFaceHandler((const CHAR*)szURL, FALSE, TRUE);
	}
	else if (string("vlcvideo_predict") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__VLCPlayerFaceHandler((const CHAR*)szURL, FALSE, FALSE);
	}
	else if (string("vlcvideo_rtsp_catchface") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__VLCPlayerFaceHandler((const CHAR*)szURL, TRUE, TRUE);
	}	
	else if (string("vlcvideo_catchface") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__VLCPlayerFaceHandler((const CHAR*)szURL, TRUE, FALSE);
	}
	else
		goto __lblUsage;

	return 0;

__lblUsage:	
	cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
	cout << argv[0] << " predict [Img Path]" << endl;
	cout << argv[0] << " ocvvideo [camera number, If not specified, the default value is 0] - use opencv player" << endl;
	cout << argv[0] << " ocvvideo [video file path] - use opencv player" << endl;
	cout << argv[0] << " vlcvideo_rtsp_predict [rtsp url] - Use the VLCVideoPlayer module to read the RTSP flow to identify faces" << endl;
	cout << argv[0] << " vlcvideo_predict [video file path] - Use the VLCVideoPlayer module to read the video file to identify faces" << endl;
	cout << argv[0] << " vlcvideo_rtsp_catchface [rtsp url] - Using VLCVideoPlayer module to read RTSP flow registration face" << endl;
	cout << argv[0] << " vlcvideo_catchface [video file path] - Using VLCVideoPlayer module to read the video file registration face" << endl;

    return 0;
}

