#pragma once

#include <map>
#include <unordered_map>

#define VLCPLAYER_DISPLAY_PREDICT_RESULT		0		//* �Ƿ��ڲ�������ʾ������ʾԤ��������ʵ������ʾ��Ԥ�������̷ֱ߳�������������
#define FACE_DETECT_USE_DNNNET					0		//* ʹ��DNN����������
#define VLCPLAYER_DISPLAY_EN					1		//* VLC��������ʾʹ��

#define FACE_DISAPPEAR_FRAME_NUM				60		//* �ж������Ѿ���ʧ��֡������������ͷ���������ռ�ݵ��ڴ�
#define MIN_PIXEL_DISTANCE_FOR_NEW_FACE			50		//* ��֮֡��ֻҪ�������겻��������ֵ����Ϊ����ԭ������
#define FACE_DETECT_MIN_CONFIDENCE_THRESHOLD	0.8f	//* ȷ����һ����������Ϳ��Ŷ���ֵ

//* �����⵽�ĵ���������Ϣ���������ꡢ��Χ������
typedef struct _ST_FACE_ {
	UINT unFaceID;			//* Ψһ�ı�ʶһ����

	UINT unFrameIdx;		//* �������ݶ�Ӧ��֡���

	//* ��������λ��
	INT nLeftTopX;
	INT nLeftTopY;
	INT nRightBottomX;
	INT nRightBottomY;
	
	string strPersonName;			//* Ԥ���������
	FLOAT flPredictConfidence;		//* Ԥ�����Ŷ�
	UINT unFrameIdxForPrediction;	//* ����Ԥ��ʱ��֡���

	Mat mFace;
} ST_FACE, *PST_FACE;

//* ����VLC������֮��ʾԤ�������Ĳ���
typedef struct _ST_VLCPLAYER_FCBDISPPREPROC_PARAM_ {
	Net *pobjDNNNet;
	unordered_map<UINT, ST_FACE> *pmapFaces;
	THMUTEX *pthMutexMapFaces;
} ST_VLCPLAYER_FCBDISPPREPROC_PARAM, *PST_VLCPLAYER_FCBDISPPREPROC_PARAM;