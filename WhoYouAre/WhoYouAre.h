#pragma once

#include <map>
#include <unordered_map>

#define FACE_DETECT_MIN_CONFIDENCE_THRESHOLD	0.8f	//* ȷ����һ����������Ϳ��Ŷ���ֵ

//* ����VLC������֮��ʾԤ�������Ĳ���
typedef struct _ST_VLCPLAYER_FCBDISPPREPROC_PARAM_ {
	Net *pobjDNNNet;
	FaceDatabase *pobjFaceDB;
} ST_VLCPLAYER_FCBDISPPREPROC_PARAM, *PST_VLCPLAYER_FCBDISPPREPROC_PARAM;