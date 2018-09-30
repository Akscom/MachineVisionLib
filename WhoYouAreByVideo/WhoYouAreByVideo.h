#pragma once

#include <map>
#include <unordered_map>

#define SHM_FACE_DB_SIZE_MAX	30						//* ����֧�ֱ�������������
#define SHM_FACE_DB_NAME		"SHM_FACE_DB"			//* ���̼乲���ڴ�����֮����
#define IPC_MUTEX_FACEDB_NAME	"IPC_MUTEX_FACEDB"		//* �����д��

#define INVALID_FACE_NODE_INDEX	0xFFFF					//* ��Ч�������ڵ�����

#define MIN_PIXEL_DISTANCE_FOR_NEW_FACE			50		//* ��֮֡��ֻҪ�������겻��������ֵ����Ϊ����ԭ������
#define FACE_DETECT_MIN_CONFIDENCE_THRESHOLD	0.5f	//* ȷ����һ����������Ϳ��Ŷ���ֵ

//* ����������֮��ʾԤ�������Ĳ���
typedef struct _ST_PLAYER_FCBDISPPREPROC_PARAM_ {
	Net *pobjDNNNet;	
} ST_PLAYER_FCBDISPPREPROC_PARAM, *PST_PLAYER_FCBDISPPREPROC_PARAM;

//* �����⡱ͷ�����ݽṹ
typedef struct _ST_SHM_FACE_DB_HDR_ {
	USHORT usFaceLink;				//* ������˫������
	USHORT usFreeLink;				//* ��������
	struct {
		UINT x;
		UINT y;
		UINT unWidth;
		UINT unHeight;
	} stROIRect;
	UINT unFrameROIDataBytes;		//* ���浱ǰ��Ƶ֡ROI�������ݵĹ����ڴ�Ĵ�С
	BOOL blIsSubProcInitOK;			//* �ӽ����Ƿ��ѳ�ʼ�����
} ST_SHM_FACE_DB_HDR, *PST_SHM_FACE_DB_HDR;

//* �����⵽�ĵ���������Ϣ���������ꡢ��Χ������
typedef struct _ST_FACE_ {
	USHORT usPrevNode;		//* ������������˫��������Ҫ��ǰ��ڵ��ַ
	USHORT usNextNode;

	UINT unFaceID;			//* Ψһ�ı�ʶһ����

	UINT unFrameIdx;		//* �������ݶ�Ӧ��֡���

	string strPersonName;			//* Ԥ���������
	FLOAT flPredictConfidence;		//* Ԥ�����Ŷ�
	UINT unFrameIdxForPrediction;	//* ����Ԥ��ʱ��֡���
	BOOL blIsPredicted;				//* �Ƿ�������Ԥ��
									
	//* ����λ������
	INT nLeftTopX;
	INT nLeftTopY;
	INT nRightBottomX;
	INT nRightBottomY;
} ST_FACE, *PST_FACE;
