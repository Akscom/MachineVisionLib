// WhoYouAre.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <io.h>
#include <vector>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
//#include <dlib/image_io.h>
//#include <dlib/opencv/cv_image.h>
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

//* ���������������⵽��������������
static BOOL __MVLVideoSaveFace(Mat& mFace, UINT unFaceID)
{
	ostringstream oss;
	oss << "FaceID: " << unFaceID;
	string strWinName(oss.str());

	imshow(strWinName, mFace);

	CHAR bKey = waitKey(1000);

	//* ���ڻس����򱣴�֮
	if (bKey == 13)
	{
		oss.str("");
		oss << "Face-" << unFaceID << ".jpg";
		cout << oss.str() << endl;
		imwrite(oss.str(), mFace);
		destroyWindow(strWinName);
		return TRUE;
	}

	return FALSE;
}

#if VLCPLAYER_DISPLAY_PREDICT_RESULT
//* ���������ʵʱԤ������
void __MVLVideoPredict(Mat& mFace, UINT unFaceID, FaceDatabase *pobjFaceDB, unordered_map<UINT, ST_FACE> *pmapFaces, THMUTEX *pthMutexMapFaces)
{
	string strPersonName;
	DOUBLE dblSimilarity = pobjFaceDB->Predict(mFace, strPersonName);
	if (dblSimilarity > 0.8)
	{		
		(*pmapFaces)[unFaceID].strPersonName = strPersonName;
		(*pmapFaces)[unFaceID].flPredictConfidence = (FLOAT)dblSimilarity;		
	}
}
#endif

static void __MVLVideoDispPreprocessor(Mat& mVideoFrame, void *pvParam, UINT unCurFrameIdx)
{	
	static UINT unFaceID = 0;
	BOOL blIsNotFound;	

#if FACE_DETECT_USE_DNNNET
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
#endif

#if VLCPLAYER_DISPLAY_PREDICT_RESULT
	//* ����Ƶ����ʾԤ����������������ʾ�ٸ������⣬�Լ��ٲ���Ҫ�Ĵ�����������ӵ�������Ҫ��ʾ����Ϊ��δԤ�⣩
	EnterThreadMutex(pstParam->pthMutexMapFaces);
	{
		unordered_map<UINT, ST_FACE>::iterator iterFace;
		for (iterFace = pstParam->pmapFaces->begin(); iterFace != pstParam->pmapFaces->end(); iterFace++)
		{
			//* �н���ˣ���ʾ����
			if (iterFace->second.flPredictConfidence > 0.0f)
			{
				//* ��������
				Rect rectObj(iterFace->second.nLeftTopX, iterFace->second.nLeftTopY, (iterFace->second.nRightBottomX - iterFace->second.nLeftTopX), (iterFace->second.nRightBottomY - iterFace->second.nLeftTopY));
				rectangle(mROI, rectObj, Scalar(0, 255, 0));

				INT nBaseLine = 0;
				String strPersonLabel;
				string strConfidenceLabel;
				Rect rect;

				//* ��������Ϳ��Ŷ�
				strPersonLabel = "Name: " + iterFace->second.strPersonName;
				strConfidenceLabel = "Confidence: " + static_cast<ostringstream*>(&(ostringstream() << iterFace->second.flPredictConfidence))->str();

				Size personLabelSize = getTextSize(strPersonLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
				Size confidenceLabelSize = getTextSize(strConfidenceLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
				rect = Rect(Point(iterFace->second.nLeftTopX, iterFace->second.nLeftTopY - (personLabelSize.height + confidenceLabelSize.height + 3)),
							Size(personLabelSize.width > confidenceLabelSize.width ? personLabelSize.width : confidenceLabelSize.width,
							personLabelSize.height + confidenceLabelSize.height + nBaseLine + 3));
				rectangle(mROI, rect, Scalar(255, 255, 255), CV_FILLED);
				putText(mROI, strPersonLabel, Point(iterFace->second.nLeftTopX, iterFace->second.nLeftTopY - confidenceLabelSize.height - 3), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
				putText(mROI, strConfidenceLabel, Point(iterFace->second.nLeftTopX, iterFace->second.nLeftTopY), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
			}
		}
	}
	ExitThreadMutex(pstParam->pthMutexMapFaces);
#else	
	#if FACE_DETECT_USE_DNNNET
		cv2shell::MarkFaceWithRectangle(mROI, matFaces, 0.8);
	#endif
#endif

#if VLCPLAYER_DISPLAY_PREDICT_RESULT	
	//* ȡ������������֮
	for (INT i = 0; i < matFaces.rows; i++)
	{ 
		FLOAT flConfidenceVal = matFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < FACE_DETECT_MIN_CONFIDENCE_THRESHOLD)
			continue;

		blIsNotFound = TRUE;

		INT nCurLTX = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * mROI.cols) - 40;
		INT nCurLTY = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * mROI.rows) - 30;
		INT nCurRBX = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * mROI.cols) + 40;
		INT nCurRBY = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * mROI.rows) + 30;

		//* ȷ������û�г���ROI��Χ
		nCurLTX = nCurLTX < 0 ? 0 : nCurLTX;
		nCurLTY = nCurLTY < 0 ? 0 : nCurLTY;
		nCurRBX = nCurRBX > mROI.cols ? mROI.cols : nCurRBX;
		nCurRBY = nCurRBY > mROI.rows ? mROI.rows : nCurRBY;

		//* �����������ݿ⣬��ǰ���Ƿ��ѽ�����ӵ����У�û�е�����ӣ��е���������꼰����ͼ������
		EnterThreadMutex(pstParam->pthMutexMapFaces);
		{
			unordered_map<UINT, ST_FACE>::iterator iterFace;
			for (iterFace = pstParam->pmapFaces->begin(); iterFace != pstParam->pmapFaces->end(); iterFace++)
			{
				INT nOldFaceLTX = iterFace->second.nLeftTopX;
				INT nOldFaceLTY = iterFace->second.nLeftTopY;
				INT nOldFaceRBX = iterFace->second.nRightBottomX;
				INT nOldFaceRBY = iterFace->second.nRightBottomY;

				//* ȫ�������������Ϊ��ͬһ�������������ݼ��ɣ�����Ҫ�������
				if (abs(nCurLTX - nOldFaceLTX) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
					abs(nCurLTY - nOldFaceLTY) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
					abs(nCurRBX - nOldFaceRBX) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
					abs(nCurRBY - nOldFaceRBY) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
					(unCurFrameIdx - iterFace->second.unFrameIdx) < FACE_DISAPPEAR_FRAME_NUM)
				{
					//* �����������ݣ�����ǰ���δ���֮ǰҲ����֮��
					iterFace->second.nLeftTopX = nCurLTX;
					iterFace->second.nLeftTopY = nCurLTY;
					iterFace->second.nRightBottomX = nCurRBX;
					iterFace->second.nRightBottomY = nCurRBY;

					//* ������ͼ������ڴ�
					Mat mFace = mROI(Rect(nCurLTX, nCurLTY, nCurRBX - nCurLTX, nCurRBY - nCurLTY));
					mFace.copyTo(iterFace->second.mFace);		

					//* ע�����һ�������������ݴ������£�������ȷ�����������Ѿ��������ڴ棬�����̲߳���õ����ڵ���������
					iterFace->second.unFrameIdx = unCurFrameIdx;

					blIsNotFound = FALSE;
					break;
				}
			}

			//* û���ҵ������֮
			if (blIsNotFound)
			{
				ST_FACE stFace;

				//* ������������
				stFace.nLeftTopX = nCurLTX;
				stFace.nLeftTopY = nCurLTY;
				stFace.nRightBottomX = nCurRBX;
				stFace.nRightBottomY = nCurRBY;
				stFace.flPredictConfidence = 0.0f;
				stFace.unFrameIdxForPrediction = 0;				

				//* ������ͼ������ڴ�
				Mat mFace = mROI(Rect(nCurLTX, nCurLTY, nCurRBX - nCurLTX, nCurRBY - nCurLTY));
				mFace.copyTo(stFace.mFace);
				stFace.unFrameIdx = unCurFrameIdx;

				stFace.unFaceID = unFaceID++;

				//* ��ӵ�����
				(*pstParam->pmapFaces)[stFace.unFaceID] = stFace;
			}
		}
		ExitThreadMutex(pstParam->pthMutexMapFaces);		
	}
#endif
}

using namespace dlib;

//* ͨ����������ͷʶ��
static void __MVLVideoFaceHandler(const CHAR *pszURL, BOOL blIsCatchFace, BOOL blIsRTSPStream)
{
#if VLCPLAYER_DISPLAY_PREDICT_RESULT
	unordered_map<UINT, ST_FACE> mapFaces;	//* �����������⵽�Ķ���������Ϣ�洢��map��
	unordered_map<UINT, ST_FACE>::iterator iterFace;
	THMUTEX thMutexMapFaces;
#endif

	FaceDatabase objFaceDB;
	if (!blIsCatchFace)	//* Ԥ�⣬�����Ԥ������
	{
		objFaceDB.pvideo_predict = new FaceDatabase::VideoPredict(&objFaceDB);

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
	}

	VLCVideoPlayer objVideoPlayer;

#if VLCPLAYER_DISPLAY_EN
	cvNamedWindow(pszURL, CV_WINDOW_AUTOSIZE);
#endif	

	if (blIsRTSPStream)
	{
		objVideoPlayer.OpenVideoFromeRtsp(pszURL,
#if VLCPLAYER_DISPLAY_EN
			__MVLVideoDispPreprocessor,
#else
			NULL,
#endif
			pszURL);
	}
	else
	{
		objVideoPlayer.OpenVideoFromFile(pszURL,
#if VLCPLAYER_DISPLAY_EN
			__MVLVideoDispPreprocessor,
#else
			NULL, 
#endif
			pszURL);
	}

	if (!objVideoPlayer.start())
	{
		cout << "start rtsp stream failed!" << endl;
		return;
	}

#if FACE_DETECT_USE_DNNNET
	//* ��ʼ��DNN�����������
	Net dnnNet = cv2shell::InitFaceDetectDNNet();
#endif

	//* ���ò�������ʾԤ����������ڲ���
	ST_VLCPLAYER_FCBDISPPREPROC_PARAM stFCBDispPreprocParam;
#if FACE_DETECT_USE_DNNNET
	stFCBDispPreprocParam.pobjDNNNet = &dnnNet;
#endif
#if VLCPLAYER_DISPLAY_PREDICT_RESULT
	stFCBDispPreprocParam.pmapFaces = &mapFaces;
	InitThreadMutex(&thMutexMapFaces);	//* ��mapFaces�ķ��ʱ�����б�������Ϊ���������̲߳�������
	stFCBDispPreprocParam.pthMutexMapFaces = &thMutexMapFaces;
#endif

#if VLCPLAYER_DISPLAY_EN
	objVideoPlayer.SetDispPreprocessorInputParam(&stFCBDispPreprocParam);
#endif
	
	frontal_face_detector detector = get_frontal_face_detector();
	shape_predictor pose_model;
	deserialize("C:\\OpenCV3.4\\dlib-19.10\\models\\shape_predictor_68_face_landmarks.dat") >> pose_model;

	CascadeClassifier a;
	a.load("C:\\OpenCV3.4\\opencv\\build\\etc\\haarcascades\\haarcascade_frontalface_alt2.xml");

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
				cout << "����������ͷ�����ӶϿ����޷�������ȡʵʱ��Ƶ����" << endl;

			break;
		}	

#if !VLCPLAYER_DISPLAY_PREDICT_RESULT
		Mat mSrcFrame = objVideoPlayer.GetNextFrame();
		if (mSrcFrame.empty())
			continue;

		Mat mFrame = mSrcFrame.clone();
		Mat mGray;
		cvtColor(mFrame, mGray, CV_BGR2GRAY);		
		std::vector<cv::Rect> faces;

		a.detectMultiScale(mGray, faces, 1.1, 3, 0, Size(50, 50), Size(500, 500));


		//cv_image<bgr_pixel> cimg(mFrame);
		// Detect faces 
		//std::vector<dlib::rectangle> faces = detector(cimg);

		std::vector<full_object_detection> shapes;

		cout << faces.size() << endl;
		
		for (unsigned long i = 0; i < faces.size(); ++i)
		{
			dlib::rectangle rect(faces[i].tl().x, faces[i].tl().y, faces[i].br().x, faces[i].br().y);
			shapes.push_back(pose_model(dlib::cv_image<uchar>(mGray), rect));
		}

		if (!shapes.empty()) {
			for (int i = 0; i < 68; i++) {
				circle(mFrame, cvPoint(shapes[0].part(i).x(), shapes[0].part(i).y()), 3, cv::Scalar(0, 0, 255), -1);
				//	shapes[0].part(i).x();//68��
			}
		}


		//objFaceDB.pvideo_predict->Predict(mFrame);

		imshow("Predict Result", mFrame);
#else
		//* ʶ������
		for (iterFace = mapFaces.begin(); iterFace != mapFaces.end(); iterFace++)
		{
			//* ��ǰ֡��δ��������֮
			if (iterFace->second.unFrameIdxForPrediction != iterFace->second.unFrameIdx)
			{
				Mat mFace;
				UINT unCurFrameIdx;
				EnterThreadMutex(&thMutexMapFaces);
				{
					iterFace->second.mFace.copyTo(mFace);
					unCurFrameIdx = iterFace->second.unFrameIdx;	//* ���������︳ֵ�������п�����ʾ�߳��������if(����unFrameIdxForPrediction != ����unFrameIdx)֮����¸�ֵ
				}
				ExitThreadMutex(&thMutexMapFaces);

				if (blIsCatchFace)
				{
					//* ���Ϊ�棬������Ѿ��洢��һ������������ֱ���˳�����
					if (__MVLVideoSaveFace(mFace, iterFace->second.unFaceID))
						goto __lblStop;											
				}
				else
					__MVLVideoPredict(mFace, iterFace->second.unFaceID, &objFaceDB, &mapFaces, &thMutexMapFaces);
					
				mapFaces[iterFace->second.unFaceID].unFrameIdxForPrediction = unCurFrameIdx;
			}
		}

		//* ������ɾ���Ѿ���ʧ����������
		EnterThreadMutex(&thMutexMapFaces);
		{
			UINT unCurFrameIdx = objVideoPlayer.GetCurFrameIndex();			
			for (iterFace = mapFaces.begin(); iterFace != mapFaces.end();)
			{
				//* ��ʱ��ɾ��֮
				if ((unCurFrameIdx - iterFace->second.unFrameIdx) > FACE_DISAPPEAR_FRAME_NUM)
				{
					mapFaces.erase(iterFace++);	//* ע�⣬����Ҫɾ��Ԫ�أ����ֻ��������++����for��++�Ļ��ᵼ�µ�ǰԪ����ɾ�����޷�����++
												//* ���������ȱ���iterFace��ǰָ��Ȼ��++��Ȼ���ٽ�����ĵ�ǰָ�򴫵ݸ�erase()����
				}
				else
					iterFace++;	//* ��һ��
			}
		}
		ExitThreadMutex(&thMutexMapFaces);
#endif
	}


__lblStop:
	//* ��ʵ�������������Ҳ���ԣ�VLCVideoPlayer��������������������õ�
	objVideoPlayer.stop();	

#if VLCPLAYER_DISPLAY_PREDICT_RESULT
	UninitThreadMutex(&thMutexMapFaces);
#endif

	destroyAllWindows();
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3 && argc != 4 && argc != 2)
	{
		cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
		cout << argv[0] << " predict [Img Path]" << endl;
		cout << argv[0] << " video [camera number, If not specified, the default value is 0]" << endl;
		cout << argv[0] << " video [video file path]" << endl;
		cout << argv[0] << " mvlvideo_rtsp_predict [rtsp url] - ʹ��MVLVideoģ���ȡRTSP����ʶ������" << endl;
		cout << argv[0] << " mvlvideo_predict [video file path] - ʹ��MVLVideoģ���ȡ��Ƶ�ļ�ʶ������" << endl;
		cout << argv[0] << " mvlvideo_catchface [rtsp url] - ʹ��MVLVideoģ���ȡRTSP����Ⲣ��������" << endl;

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
	else if (string("mvlvideo_rtsp_predict") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__MVLVideoFaceHandler((const CHAR*)szURL, FALSE, TRUE);
	}
	else if (string("mvlvideo_catchface") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__MVLVideoFaceHandler((const CHAR*)szURL, TRUE, TRUE);
	}
	else if (string("mvlvideo_predict") == strOptType.toLowerCase())
	{
		if (argc != 3)
			goto __lblUsage;

		CHAR szURL[MAX_PATH];
		sprintf_s(szURL, "%s", argv[2]);

		__MVLVideoFaceHandler((const CHAR*)szURL, FALSE, FALSE);
	}
	else
		goto __lblUsage;

	return 0;

__lblUsage:	
	cout << "Usage: " << endl << argv[0] << " add [Img Path] [Person Name]" << endl;
	cout << argv[0] << " predict [Img Path]" << endl;
	cout << argv[0] << " video [camera number, If not specified, the default value is 0]" << endl;
	cout << argv[0] << " video [video file path]" << endl;
	cout << argv[0] << " mvlvideo_rtsp_predict [rtsp url] - ʹ��MVLVideoģ���ȡRTSP����ʶ������" << endl;
	cout << argv[0] << " mvlvideo_predict [video file path] - ʹ��MVLVideoģ���ȡ��Ƶ�ļ�ʶ������" << endl;
	cout << argv[0] << " mvlvideo_catchface [rtsp url] - ʹ��MVLVideoģ���ȡRTSP����Ⲣ��������" << endl;

    return 0;
}

