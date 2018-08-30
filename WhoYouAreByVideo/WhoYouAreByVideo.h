#pragma once

#define SHM_FACE_DB_SIZE_MAX	10				//* ����֧�ֱ�������������
#define SHM_FACE_DB_NAME		"SHM_FACE_DB"	//* ���̼乲���ڴ�����֮����

//* �����⡱ͷ�����ݽṹ
typedef struct _ST_SHM_FACE_DB_HDR_ {
	UINT unFrameROIWidth;	//* ��Ƶ֡ROI����Ŀ��
	UINT unFrameROIHeight;	//* ��Ƶ֡ROI����ĸ߶�
} ST_SHM_FACE_DB_HDR, *PST_SHM_FACE_DB_HDR;

//* �����⵽�ĵ���������Ϣ���������ꡢ��Χ������
typedef struct _ST_FACE_ {
	UINT unFaceID;			//* Ψһ�ı�ʶһ����

	UINT unFrameIdx;		//* �������ݶ�Ӧ��֡���

	string strPersonName;			//* Ԥ���������
	FLOAT flPredictConfidence;		//* Ԥ�����Ŷ�
	UINT unFrameIdxForPrediction;	//* ����Ԥ��ʱ��֡���
	
	Face objFace;					//* ����λ������
} ST_FACE, *PST_FACE;
