#pragma once

#define FACE_DETECT_MIN_CONFIDENCE_THRESHOLD	0.5f	//* ȷ����һ����������Ϳ��Ŷ���ֵ

//* ����������֮��ʾԤ�������Ĳ���
typedef struct _ST_PLAYER_FCBDISPPREPROC_PARAM_ {
	Net *pobjDNNNet;
	FaceDatabase *pobjFaceDB;
} ST_PLAYER_FCBDISPPREPROC_PARAM, *PST_PLAYER_FCBDISPPREPROC_PARAM;