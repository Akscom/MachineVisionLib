// WhoYouAreByVideo.cpp : �������̨Ӧ�ó������ڵ㡣
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
#include "WhoYouAreByVideo.h"

static HANDLE hSHMFaceDB;
static CHAR *pbSHMFaceDB;
static HANDLE hMutexFaceDB;
static PST_SHM_FACE_DB_HDR pstFaceDBHdr;
static UCHAR *pubFDBFrameROIData;
static PST_FACE pstFaceDB;

static BOOL blIsRunning;
BOOL WINAPI ConsoleCtrlHandler(DWORD dwEvent)
{
	blIsRunning = FALSE;
	printf("The process receives the CTL+C signal and will exit.\r\n");
	return TRUE;
}

//* ���һ�š�����������
static void __PutNodeToFaceLink(USHORT *pusNewLink, USHORT usFaceNode, BOOL blIsExistFace)
{
	PST_FACE pstFace = pstFaceDB;

	//* ���FaceLink����������������ժ��֮
	if (blIsExistFace)
	{
		if (pstFace[usFaceNode].usPrevNode != INVALID_FACE_NODE_INDEX)
			pstFace[pstFace[usFaceNode].usPrevNode].usNextNode = pstFace[usFaceNode].usNextNode;
		//* ��Ȼ������ͷ��
		else 
			pstFaceDBHdr->usFaceLink = pstFace[usFaceNode].usNextNode;

		if (pstFace[usFaceNode].usNextNode != INVALID_FACE_NODE_INDEX)
			pstFace[pstFace[usFaceNode].usNextNode].usPrevNode = pstFace[usFaceNode].usPrevNode;
	}

	pstFace[usFaceNode].usPrevNode = INVALID_FACE_NODE_INDEX;
	if ((*pusNewLink) == INVALID_FACE_NODE_INDEX)
	{		
		pstFace[usFaceNode].usNextNode = INVALID_FACE_NODE_INDEX;		

		*pusNewLink = usFaceNode;

		return;
	}

	pstFace[*pusNewLink].usPrevNode = usFaceNode;
	pstFace[usFaceNode].usNextNode = *pusNewLink;	
	*pusNewLink = usFaceNode;
}

//* ��ȡһ�����нڵ�
static USHORT __GetNodeFromFreeLink(void)
{
	PST_FACE pstFace = pstFaceDB;

	//* ���п��нڵ�
	if (pstFaceDBHdr->usFreeLink != INVALID_FACE_NODE_INDEX)
	{
		USHORT usFreeNode = pstFaceDBHdr->usFreeLink;
		if (pstFace[usFreeNode].usNextNode != INVALID_FACE_NODE_INDEX)
			pstFace[pstFace[usFreeNode].usNextNode].usPrevNode = INVALID_FACE_NODE_INDEX;

		
		pstFaceDBHdr->usFreeLink = pstFace[usFreeNode].usNextNode;
		pstFace[usFreeNode].usNextNode = INVALID_FACE_NODE_INDEX;

		return usFreeNode;
	}

	return INVALID_FACE_NODE_INDEX;
}

//* �ͷ�ռ�õĽڵ�
static void __FreeFaceNode(USHORT usFaceNode)
{
	PST_FACE pstFace = pstFaceDB;

	//* �ȴ�FaceLink��ժ��֮
	//* ==========================================================================
	if (pstFace[usFaceNode].usPrevNode != INVALID_FACE_NODE_INDEX)
		pstFace[pstFace[usFaceNode].usPrevNode].usNextNode = pstFace[usFaceNode].usNextNode;
	//* ��Ȼ������ͷ��
	else
		pstFaceDBHdr->usFaceLink = pstFace[usFaceNode].usNextNode;

	if (pstFace[usFaceNode].usNextNode != INVALID_FACE_NODE_INDEX)
		pstFace[pstFace[usFaceNode].usNextNode].usPrevNode = pstFace[usFaceNode].usPrevNode;
	//* ==========================================================================

	pstFace[usFaceNode].usPrevNode = INVALID_FACE_NODE_INDEX;
	if (pstFaceDBHdr->usFreeLink == INVALID_FACE_NODE_INDEX)
	{
		pstFace[usFaceNode].usNextNode = INVALID_FACE_NODE_INDEX;

		pstFaceDBHdr->usFreeLink = usFaceNode;

		return;
	}

	pstFace[pstFaceDBHdr->usFreeLink].usPrevNode = usFaceNode;
	pstFace[usFaceNode].usNextNode = pstFaceDBHdr->usFreeLink;
	pstFaceDBHdr->usFreeLink = usFaceNode;
}

//* �ͷ�FaceLink���нڵ㣬ע���������û�м�������Ҫ�ϲ���ú��������ſ�
static void __FreeFaceLink(void)
{
	PST_FACE pstFace = pstFaceDB;
	USHORT usFaceNode;
	while ((usFaceNode = pstFaceDBHdr->usFaceLink) != INVALID_FACE_NODE_INDEX)
	{
		__FreeFaceNode(usFaceNode);
	}
}

//* VLC������֮�ص�����������
static void __FCBVLCPlayerFaceHandler(Mat& mVideoFrame, void *pvParam, UINT unCurFrameIdx)
{
	PST_PLAYER_FCBDISPPREPROC_PARAM pstParam = (PST_PLAYER_FCBDISPPREPROC_PARAM)pvParam;
	static UINT unFaceID = 0;
	
	Mat mROI = mVideoFrame(Rect(pstFaceDBHdr->stROIRect.x, pstFaceDBHdr->stROIRect.y, pstFaceDBHdr->stROIRect.unWidth, pstFaceDBHdr->stROIRect.unHeight));
	Mat &matFaces = cv2shell::FaceDetect(*pstParam->pobjDNNNet, mROI);
	if (matFaces.empty())
		return;

	Mat mFaceGray;
	cvtColor(mROI, mFaceGray, CV_BGR2GRAY);
	
	//* ȡ��ÿ�������������"����"
	IPCEnterCriticalSection(hMutexFaceDB);
	{
		BOOL blIsNotFound;
		USHORT usNewFaceLink = INVALID_FACE_NODE_INDEX;
		for (INT i = 0; i < matFaces.rows; i++)
		{
			FLOAT flConfidenceVal = matFaces.at<FLOAT>(i, 2);
			if (flConfidenceVal < FACE_DETECT_MIN_CONFIDENCE_THRESHOLD)
				continue;

			INT nCurLTX = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * mROI.cols);
			INT nCurLTY = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * mROI.rows);
			INT nCurRBX = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * mROI.cols);
			INT nCurRBY = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * mROI.rows);

			blIsNotFound = TRUE;
			PST_FACE pstFace = pstFaceDB;
			USHORT usCurNode = pstFaceDBHdr->usFaceLink;
			while (usCurNode != INVALID_FACE_NODE_INDEX)
			{
				INT nPrevFaceLTX = pstFace[usCurNode].nLeftTopX;
				INT nPrevFaceLTY = pstFace[usCurNode].nLeftTopY;
				INT nPrevFaceRBX = pstFace[usCurNode].nRightBottomX;
				INT nPrevFaceRBY = pstFace[usCurNode].nRightBottomY;

				//* ȫ�������������Ϊ��ͬһ�������������ݼ��ɣ�����Ҫ�������
				if (abs(nCurLTX - nPrevFaceLTX) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
					abs(nCurLTY - nPrevFaceLTY) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
					abs(nCurRBX - nPrevFaceRBX) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE &&
					abs(nCurRBY - nPrevFaceRBY) < MIN_PIXEL_DISTANCE_FOR_NEW_FACE)
				{
					//* ������������
					pstFace[usCurNode].nLeftTopX = nCurLTX;
					pstFace[usCurNode].nLeftTopY = nCurLTY;
					pstFace[usCurNode].nRightBottomX = nCurRBX;
					pstFace[usCurNode].nRightBottomY = nCurRBY;

					//* ע�����һ����������
					pstFaceDB[usCurNode].unFrameIdx = unCurFrameIdx;

					//* ��������
					__PutNodeToFaceLink(&usNewFaceLink, usCurNode, TRUE);

					blIsNotFound = FALSE;
					break;
				}

				//* ������һ��
				usCurNode = pstFace[usCurNode].usNextNode;
			}

			//* û���ҵ������֮
			if (blIsNotFound)
			{
				USHORT usFreeNode = __GetNodeFromFreeLink();
				if (usFreeNode == INVALID_FACE_NODE_INDEX)	//* û�п��нڵ��ˣ�ʣ�µľͲ�������
				{
					cout << "Warning: Insufficient face database space!" << endl;					
					break;
				}

				//* ������������
				pstFaceDB[usFreeNode].nLeftTopX = nCurLTX;
				pstFaceDB[usFreeNode].nLeftTopY = nCurLTY;
				pstFaceDB[usFreeNode].nRightBottomX = nCurRBX;
				pstFaceDB[usFreeNode].nRightBottomY = nCurRBY;

				pstFaceDB[usFreeNode].flPredictConfidence = 0.0f;

				pstFaceDB[usFreeNode].unFrameIdx = unCurFrameIdx;
				pstFaceDB[usFreeNode].unFaceID = unFaceID++;

				//* ��������
				__PutNodeToFaceLink(&usNewFaceLink, usFreeNode, FALSE);
			}
		}

		//* �ȹ黹�Ѿ�����ʹ�õĽڵ�
		__FreeFaceLink();

		//* ��������
		pstFaceDBHdr->usFaceLink = usNewFaceLink;

		//* ����֡����		
		memcpy(pubFDBFrameROIData, mFaceGray.data, pstFaceDBHdr->unFrameROIDataBytes);
		//Mat mROIGray = Mat(pstFaceDBHdr->stROIRect.unHeight, pstFaceDBHdr->stROIRect.unWidth, CV_8UC1, pubFDBFrameROIData);
		//imshow("Gray-Main", mROIGray);		
	}
	IPCExitCriticalSection(hMutexFaceDB);

	//* ��ʾʶ����
	IPCEnterCriticalSection(hMutexFaceDB);
	{
		PST_FACE pstFace = pstFaceDB;
		USHORT usFaceNode = pstFaceDBHdr->usFaceLink;
		while (usFaceNode != INVALID_FACE_NODE_INDEX)
		{
			//* �н���ˣ���ʾ����
			if (pstFace[usFaceNode].flPredictConfidence > 0.0f)
			{
				//* ��������
				Rect objRect(pstFace[usFaceNode].nLeftTopX, pstFace[usFaceNode].nLeftTopY, (pstFace[usFaceNode].nRightBottomX - pstFace[usFaceNode].nLeftTopX), (pstFace[usFaceNode].nRightBottomY - pstFace[usFaceNode].nLeftTopY));
				rectangle(mROI, objRect, Scalar(0, 255, 0));

				INT nBaseLine = 0;
				String strPersonLabel;
				string strConfidenceLabel;
				Rect rect;

				//* ��������Ϳ��Ŷ�
				strPersonLabel = "Name: " + pstFace[usFaceNode].strPersonName;
				strConfidenceLabel = "Confidence: " + static_cast<ostringstream*>(&(ostringstream() << pstFace[usFaceNode].flPredictConfidence))->str();

				Size personLabelSize = getTextSize(strPersonLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
				Size confidenceLabelSize = getTextSize(strConfidenceLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
				rect = Rect(Point(pstFace[usFaceNode].nLeftTopX, pstFace[usFaceNode].nLeftTopY - (personLabelSize.height + confidenceLabelSize.height + 3)),
					Size(personLabelSize.width > confidenceLabelSize.width ? personLabelSize.width : confidenceLabelSize.width,
						personLabelSize.height + confidenceLabelSize.height + nBaseLine + 3));
				rectangle(mROI, rect, Scalar(255, 255, 255), CV_FILLED);
				putText(mROI, strPersonLabel, Point(pstFace[usFaceNode].nLeftTopX, pstFace[usFaceNode].nLeftTopY - confidenceLabelSize.height - 3), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
				putText(mROI, strConfidenceLabel, Point(pstFace[usFaceNode].nLeftTopX, pstFace[usFaceNode].nLeftTopY), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
			}

			//* ��һ��
			usFaceNode = pstFace[usFaceNode].usNextNode;
		}
	}
	IPCExitCriticalSection(hMutexFaceDB);
}

//* �����̵���ڴ�����
static void __MainProcHandler(const CHAR *pszVideoPath, DWORD& dwSubprocID)
{
	BOOL blIsRTSPStream;

	//* ��ʼ��DNN�����������
	Net objDNNNet = cv2shell::InitFaceDetectDNNet();

	//* ����һ��VLC�����������������Ŵ���
	VLCVideoPlayer objVideoPlayer;
	cvNamedWindow(pszVideoPath, CV_WINDOW_AUTOSIZE);
	//cvNamedWindow("Gray-Main", CV_WINDOW_AUTOSIZE);

	if ((CHAR)toupper((INT)pszVideoPath[0]) == 'R'
		&& (CHAR)toupper((INT)pszVideoPath[1]) == 'T'
		&& (CHAR)toupper((INT)pszVideoPath[2]) == 'S'
		&& (CHAR)toupper((INT)pszVideoPath[3]) == 'P'
		&& (CHAR)toupper((INT)pszVideoPath[4]) == ':')
	{
		blIsRTSPStream = TRUE;
		objVideoPlayer.OpenVideoFromeRtsp(pszVideoPath, __FCBVLCPlayerFaceHandler, pszVideoPath, 1000);
	}
	else
	{
		blIsRTSPStream = FALSE;
		objVideoPlayer.OpenVideoFromFile(pszVideoPath, __FCBVLCPlayerFaceHandler, pszVideoPath);
	}		

	//* ���ò�������ʾԤ����������ڲ���
	ST_PLAYER_FCBDISPPREPROC_PARAM stFCBDispPreprocParam;
	stFCBDispPreprocParam.pobjDNNNet = &objDNNNet;	
	objVideoPlayer.SetDispPreprocessorInputParam(&stFCBDispPreprocParam);

	//* ��ʼ����
	if (!objVideoPlayer.start())
	{
		cout << "start rtsp stream failed!" << endl;
		return;
	}

	CHAR bKey;
	BOOL blIsPaused = FALSE;
	while (blIsRunning)
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
	}

	StopProcess(dwSubprocID);

	//* ��ʵ�������������Ҳ���ԣ�VLCVideoPlayer��������������������õ�
	objVideoPlayer.stop();

	destroyAllWindows();
}

//* �жϵ�ǰFaceID�Ƿ񻹴��ڣ���������ھ�û��ҪԤ����߸��������ˣ�����ֵΪ��ID�������еı���λ��
static USHORT __IsTheFaceIDExist(UINT unFaceID)
{
	PST_FACE pstFace = pstFaceDB;
	USHORT usFaceNode = pstFaceDBHdr->usFaceLink;
	while (usFaceNode != INVALID_FACE_NODE_INDEX)
	{
		if (unFaceID == pstFace[usFaceNode].unFaceID)
			return usFaceNode;

		//* ��һ��
		usFaceNode = pstFace[usFaceNode].usNextNode;
	}

	return INVALID_FACE_NODE_INDEX;
}

//* ͬ�ϣ�ǰ׺S��������Ǵ��������ĺ���������Ҫ�ϲ�����߶ԡ����⡱����
static USHORT __S_IsTheFaceIDExist(UINT unFaceID)
{
	USHORT usFaceNode;

	IPCEnterCriticalSection(hMutexFaceDB);
	{
		usFaceNode = __IsTheFaceIDExist(unFaceID);
	}
	IPCExitCriticalSection(hMutexFaceDB);

	return usFaceNode;
}

//* �ӽ��̵���ڴ�����
static void __SubProcHandler(void)
{
	FaceDatabase objFaceDB;

	//* ��ʼ������ʶ��ģ��
	//* =======================================================================================
	if (!objFaceDB.LoadFaceData())
	{
		cout << "Load face data failed, the process will be exited!" << endl;
		return;
	}

	if (!objFaceDB.LoadDLIB68FaceLandmarksModel("C:\\OpenCV3.4\\dlib-19.10\\models\\shape_predictor_68_face_landmarks.dat"))
		return;

	if (!objFaceDB.LoadCaffeVGGNet("C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE_extract_deploy.prototxt",
		"C:\\windows\\system32\\models\\vgg_face_caffe\\VGG_FACE.caffemodel"))
	{
		cout << "Load Failed, the process will be exited!" << endl;
		return;
	}
	objFaceDB.o_pobjVideoPredict = new FaceDatabase::VideoPredict(&objFaceDB);
	//* =======================================================================================


	ST_FACE staFace[SHM_FACE_DB_SIZE_MAX];
	UCHAR *pubaROIGrayData = new UCHAR[pstFaceDBHdr->unFrameROIDataBytes];
	Mat mROIGray = Mat(pstFaceDBHdr->stROIRect.unHeight, pstFaceDBHdr->stROIRect.unWidth, CV_8UC1, pubaROIGrayData);
	while (blIsRunning)
	{
		UINT unFaceNum = 0;		

		//* ȡ����������
		IPCEnterCriticalSection(hMutexFaceDB);
		{
			PST_FACE pstFace = pstFaceDB;
			USHORT usFaceNode = pstFaceDBHdr->usFaceLink;
			while (usFaceNode != INVALID_FACE_NODE_INDEX)
			{
				staFace[unFaceNum++] = pstFace[usFaceNode];

				//* ��һ��
				usFaceNode = pstFace[usFaceNode].usNextNode;
			}							

			
			memcpy(pubaROIGrayData, pubFDBFrameROIData, pstFaceDBHdr->unFrameROIDataBytes);											
		}		
		IPCExitCriticalSection(hMutexFaceDB);
		

		//* Ԥ��		
		for (UINT i = 0; i < unFaceNum; i++)
		{
			staFace[i].blIsPredicted = FALSE;

			//* ֡����ͬ�����Ѿ�Ԥ�������δ����֡���ݣ����������Ѿ���ʧ������Ԥ�⣬�Ͼ�Ԥ��̫��ʱ
			if (staFace[i].unFrameIdx == staFace[i].unFrameIdxForPrediction || 
					INVALID_FACE_NODE_INDEX == __S_IsTheFaceIDExist(staFace[i].unFaceID))
			{
				continue;
			}

			Face objFace(staFace[i].nLeftTopX, staFace[i].nLeftTopY, staFace[i].nRightBottomX, staFace[i].nRightBottomY);			
			DOUBLE dblConfidenceVal = objFaceDB.o_pobjVideoPredict->Predict(mROIGray, objFace, objFaceDB.o_objShapePredictor, staFace[i].strPersonName);
			if (dblConfidenceVal > 0.8f)
			{
				staFace[i].flPredictConfidence = dblConfidenceVal;
				staFace[i].blIsPredicted = TRUE;
				staFace[i].unFrameIdxForPrediction = staFace[i].unFrameIdx;				
			}						
		}

		//* ��Ԥ�������롰���⡱
		IPCEnterCriticalSection(hMutexFaceDB);
		{
			for (UINT i = 0; i < unFaceNum; i++)
			{
				USHORT usFaceNode;

				//* δ����Ԥ�����δ�ҵ�ƥ���¼�򲻸���
				if (!staFace[i].blIsPredicted)
					continue;

				usFaceNode = __IsTheFaceIDExist(staFace[i].unFaceID);
				if (usFaceNode != INVALID_FACE_NODE_INDEX)
				{										
					pstFaceDB[usFaceNode].strPersonName = staFace[i].strPersonName;
					pstFaceDB[usFaceNode].flPredictConfidence = staFace[i].flPredictConfidence;					
				}
			}
		}
		IPCExitCriticalSection(hMutexFaceDB);		
	}

	delete[] pubaROIGrayData;
}

//* �����̳�ʼ��
static BOOL __MainProcInit(DWORD& dwSubprocID)
{
	Rect objROIRect;
	UINT unFrameROIDataBytes;

	UINT unFrameWidth = DEFAULT_VIDEO_FRAME_WIDTH;
	UINT unFrameHeight = (unFrameWidth / 16) * 9;
	if (unFrameWidth > unFrameHeight)
		objROIRect = Rect((unFrameWidth - unFrameHeight) / 2, 0, unFrameHeight, unFrameHeight);
	else if (unFrameWidth < unFrameHeight)
		objROIRect = Rect(0, (unFrameHeight - unFrameWidth) / 2, unFrameWidth, unFrameWidth);
	else
		objROIRect = Rect(0, 0, unFrameWidth, unFrameHeight);

	unFrameROIDataBytes = objROIRect.width * objROIRect.height * sizeof(UCHAR);	

	//* �������֯�ṹΪ��ST_SHM_FACE_DB_HDR + mROIGray.data + FaceDB
	if (NULL == (pbSHMFaceDB = IPCCreateSHM(SHM_FACE_DB_NAME, sizeof(ST_SHM_FACE_DB_HDR) + unFrameROIDataBytes + SHM_FACE_DB_SIZE_MAX * sizeof(ST_FACE), &hSHMFaceDB)))
		return FALSE;

	//* ����������
	hMutexFaceDB = IPCCreateCriticalSection(IPC_MUTEX_FACEDB_NAME);
	if (hMutexFaceDB == INVALID_HANDLE_VALUE)
	{
		IPCCloseSHM(pbSHMFaceDB, hSHMFaceDB);
		return FALSE;
	}

	//* �����⡱��ص�ַ
	pstFaceDBHdr = (PST_SHM_FACE_DB_HDR)pbSHMFaceDB;											//* ����ͷ�����ݽṹ
	pubFDBFrameROIData = (UCHAR*)(pbSHMFaceDB + sizeof(ST_SHM_FACE_DB_HDR));					//* ��Ƶ֡ROI����������"����"�е��׵�ַ
	pstFaceDB = (PST_FACE)(pbSHMFaceDB + sizeof(ST_SHM_FACE_DB_HDR) + unFrameROIDataBytes);		//* ÿ���������꼰�������

	//* ��������	
	//* =========================================================================
	PST_FACE pstFace = pstFaceDB;
	pstFace->usPrevNode = INVALID_FACE_NODE_INDEX;
	for (INT i = 1; i < SHM_FACE_DB_SIZE_MAX; i++)
	{
		pstFace[i].usPrevNode = i - 1;
		pstFace[i].usNextNode = INVALID_FACE_NODE_INDEX;
		pstFace[i - 1].usNextNode = i;
	}
	//* ��������ָ���һ���ڵ㼴�ɣ�Face������δ���κ�����
	pstFaceDBHdr->usFreeLink = 0;
	pstFaceDBHdr->usFaceLink = INVALID_FACE_NODE_INDEX;
	//* =========================================================================	

	//* ����ROI�����������λ�ü�����
	pstFaceDBHdr->stROIRect.x = objROIRect.x;
	pstFaceDBHdr->stROIRect.y = objROIRect.y;
	pstFaceDBHdr->stROIRect.unWidth = objROIRect.width;
	pstFaceDBHdr->stROIRect.unHeight = objROIRect.height;
	pstFaceDBHdr->unFrameROIDataBytes = unFrameROIDataBytes;

	//* �����ӽ���
	dwSubprocID = StartProcess("WhoYouAreByVideo.exe", NULL);
	if (INVALID_PROC_ID == dwSubprocID)
	{
		IPCDelCriticalSection(hMutexFaceDB);
		IPCCloseSHM(pbSHMFaceDB, hSHMFaceDB);

		cout << "Sub process startup failure for face recognition, the process exit!" << endl;
		return FALSE;
	}

	return TRUE;
}

static BOOL __SubProcInit(void)
{
	//* ������
	if (NULL == (pbSHMFaceDB = IPCOpenSHM(SHM_FACE_DB_NAME, &hSHMFaceDB)))
		return FALSE;

	//* ��������
	hMutexFaceDB = IPCOpenCriticalSection(IPC_MUTEX_FACEDB_NAME);
	if (hMutexFaceDB == INVALID_HANDLE_VALUE)
	{
		IPCCloseSHM(pbSHMFaceDB, hSHMFaceDB);
		return FALSE;
	}

	//* ������ʵ�ַ	
	pstFaceDBHdr = (PST_SHM_FACE_DB_HDR)pbSHMFaceDB;
	pubFDBFrameROIData = (UCHAR*)(pbSHMFaceDB + sizeof(ST_SHM_FACE_DB_HDR));
	pstFaceDB = (PST_FACE)(pbSHMFaceDB + sizeof(ST_SHM_FACE_DB_HDR) + pstFaceDBHdr->unFrameROIDataBytes);		

	return TRUE;
}

//* ȥ��ʼ�����ͷ�����ռ�õĸ�����Դ
static void __Uninit(void)
{
	//* �ͷ����������رա����⡱
	IPCDelCriticalSection(hMutexFaceDB);
	IPCCloseSHM(pbSHMFaceDB, hSHMFaceDB);
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dwSubprocID;

	if (argc != 1 && argc != 2)
	{
		cout << "Usage: " << argv[0] << " [rtsp address]" << endl;
		cout << "Usage: " << argv[0] << " [video file path]" << endl;

		return 0;
	}

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler, TRUE);

	blIsRunning = TRUE;

	//* ���������⡱
	if (argc == 2)
	{		
		if (!__MainProcInit(dwSubprocID))
		{
			cout << "Main process initialization failed" << endl;
			return 0;
		}		

		__MainProcHandler(argv[1], dwSubprocID);
	}
	else
	{			
		if (!__SubProcInit())
		{
			cout << "Subprocess initialization failed" << endl;
			return 0;
		}

		__SubProcHandler();		
	}

	__Uninit();

    return 0;
}

