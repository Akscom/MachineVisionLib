#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MathAlgorithmLib.h"
#include "ImagePreprocess.h"
#include "MVLVideo.h"

//* ����VLC��������ϸ˵���μ���https://blog.csdn.net/jxbinwd/article/details/72722833
static CHAR* const l_pbaVLCBaseArgs[] =	//* constȷ��o_pbaVLCArg�����ֵҲ���ǵ�ַ���ᱻ�޸�	
{
	"-I",
	"dummy",
	"--ignore-config"
	//"--longhelp",		//* �������������Դ�VLC�Ĳ���˵�����������ݺܶ࣬����ڿ���̨�����ض���TXT�ļ��ٿ�
	//"--advanced",
	//"--rtsp-frame-buffer-size=1500000",	//* ��������������vlc���Զ�Ѱ��һ�����ʵģ����鲻���ã�ֻ��������ʱ����һЩ�������Ƶ�ķֱ��ʹ���Ļ�
};

//* VLC�ص�����֮����֡���ݻ���������������pvCBInputParam�Ǵ����ص��������û��Զ������
//* pvCBInputParamָ������֡���ݵĻ��������׵�ַ
void *MVLVideo::FCBLock(void *pvCBInputParam, void **ppvFrameBuf)
{	
	MVLVideo *pobjMVLVideo = (MVLVideo *)pvCBInputParam;
	*ppvFrameBuf = pobjMVLVideo->o_mVideoFrameRGB.data;

	return *ppvFrameBuf;
}

//* VLC�ص�����֮����֡���ݻ�������������ʵpvFrameData��ppvFrameBufָ�����ͬһ���ڴ棬��ֵ��FCBLock()��������
void MVLVideo::FCBUnlock(void *pvCBInputParam, void *pvFrameData, void* const *ppvFrameBuf)
{
	MVLVideo *pobjMVLVideo = (MVLVideo *)pvCBInputParam;

	cvtColor(pobjMVLVideo->o_mVideoFrameRGB, pobjMVLVideo->o_mVideoFrameBGR, CV_RGB2BGR);

	pobjMVLVideo->o_mVideoFrameBGR.copyTo(pobjMVLVideo->o_mDisplayFrame);
	pobjMVLVideo->o_unNextFrameIdx++;
}

//* VLC�ص�����֮��ʾ����
void MVLVideo::FCBDisplay(void *pvCBInputParam, void *pvFrameData)
{
	MVLVideo *pobjMVLVideo = (MVLVideo *)pvCBInputParam;

	if (pobjMVLVideo->o_pfcbDispPreprocessor)
	{
		pobjMVLVideo->o_pfcbDispPreprocessor(pobjMVLVideo->o_mDisplayFrame);
		imshow(pobjMVLVideo->o_strDisplayWinName, pobjMVLVideo->o_mDisplayFrame);
	}
}

Mat MVLVideo::GetNextFrame(void)
{
	//* ������һ֡�����Ƿ��Ѿ�����
	if (o_unPrevFrameIdx != o_unNextFrameIdx)
	{
		o_unPrevFrameIdx = o_unNextFrameIdx;
		return o_mVideoFrameBGR;
	}
	else
	{
		Mat mDummy;
		return mDummy;
	}
}

//* ��һ����Ƶ�ļ�
void MVLVideo::OpenVideoFromFile(const CHAR *pszFile, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor, 
									const CHAR *pszDisplayWinName, ENUM_ASPECT_RATIO enumAspectRatio)
{
	UINT unArgc = sizeof(l_pbaVLCBaseArgs) / sizeof(l_pbaVLCBaseArgs[0]);
	CHAR **ppbaVLCArgs = new CHAR *[unArgc];
	for (UINT i = 0; i < unArgc; i++)	//* ���ڲ�����Ƶ�ļ���ʹ��ȱʡ��������
	{
		ppbaVLCArgs[i] = l_pbaVLCBaseArgs[i];
	}
	
	//* ��ʼ����Ƶ֡Matrix
	if (o_unAdjustedWidth)	
		o_unAdjustedWidth = DEFAULT_VIDEO_FRAME_WIDTH;
	if (enumAspectRatio == AR_16_9)	
		o_unAdjustedHeight = (o_unAdjustedWidth / 16) * 9;
	else
		o_unAdjustedHeight = (o_unAdjustedWidth / 4) * 3;
	o_mVideoFrameRGB = Mat(Size(o_unAdjustedWidth, o_unAdjustedHeight), CV_8UC3);

	o_pstVLCInstance = libvlc_new(unArgc, ppbaVLCArgs);	
	delete[] ppbaVLCArgs;	//* ɾ���ղ�����Ĳ�����������libvlc_new()���ú�Ͳ�����

	libvlc_media_t* pstVLCMedia = libvlc_media_new_path(o_pstVLCInstance, pszFile);
	o_pstVLCMediaPlayer = libvlc_media_player_new_from_media(pstVLCMedia);
	libvlc_media_release(pstVLCMedia);	//* libvlc_media_player_new_from_media()������Ϻ��ͷžͿ�����

	libvlc_video_set_callbacks(o_pstVLCMediaPlayer, FCBLock, FCBUnlock, FCBDisplay, NULL);								//* �趨�ص�����
	libvlc_video_set_format(o_pstVLCMediaPlayer, "RV24", o_unAdjustedWidth, o_unAdjustedHeight, o_unAdjustedWidth * 3);	//* �趨���Ÿ�ʽ	

	o_strDisplayWinName = pszDisplayWinName;
	o_pfcbDispPreprocessor = pfcbDispPreprocessor;
}

//* ��һ������RTSP����
void MVLVideo::OpenVideoFromeRtsp(const CHAR *pszURL, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor,
									const CHAR *pszDisplayWinName, UINT unNetCachingTime, 
									BOOL blIsUsedTCP, ENUM_ASPECT_RATIO enumAspectRatio)
{
	UINT unBaseArgc = sizeof(l_pbaVLCBaseArgs) / sizeof(l_pbaVLCBaseArgs[0]);
	UINT unArgc;
	CHAR szNetCachingTime[30];
	CHAR **ppbaVLCArgs = new CHAR *[unBaseArgc + 2];

	sprintf_s(szNetCachingTime, "--network-caching=%d", unNetCachingTime);
	ppbaVLCArgs[unBaseArgc] = szNetCachingTime;
	if (blIsUsedTCP)
	{
		ppbaVLCArgs[unBaseArgc + 1] = "--rtsp-tcp";
		unArgc = unBaseArgc + 2;
	}
	else
		unArgc = unBaseArgc + 1;

	for (UINT i = 0; i < unBaseArgc; i++)	//* ���ڲ�����Ƶ�ļ���ʹ��ȱʡ��������
	{
		ppbaVLCArgs[i] = l_pbaVLCBaseArgs[i];
	}


	//* ��ʼ����Ƶ֡Matrix
	if (!o_unAdjustedWidth)
		o_unAdjustedWidth = DEFAULT_VIDEO_FRAME_WIDTH;
	if (enumAspectRatio == AR_16_9)
		o_unAdjustedHeight = (o_unAdjustedWidth / 16) * 9;
	else
		o_unAdjustedHeight = (o_unAdjustedWidth / 4) * 3;
	o_mVideoFrameRGB = Mat(Size(o_unAdjustedWidth, o_unAdjustedHeight), CV_8UC3);

	//cout << o_unAdjustedWidth << " * " << o_unAdjustedHeight << endl;

	o_pstVLCInstance = libvlc_new(unArgc, ppbaVLCArgs);
	delete[] ppbaVLCArgs;	//* ɾ���ղ�����Ĳ�����������libvlc_new()���ú�Ͳ�����

	libvlc_media_t* pstVLCMedia = libvlc_media_new_location(o_pstVLCInstance, pszURL);
	o_pstVLCMediaPlayer = libvlc_media_player_new_from_media(pstVLCMedia);
	libvlc_media_release(pstVLCMedia);	//* libvlc_media_player_new_from_media()������Ϻ��ͷžͿ�����

	libvlc_video_set_callbacks(o_pstVLCMediaPlayer, FCBLock, FCBUnlock, FCBDisplay, this);								//* �趨�ص�����
	libvlc_video_set_format(o_pstVLCMediaPlayer, "RV24", o_unAdjustedWidth, o_unAdjustedHeight, o_unAdjustedWidth * 3);	//* �趨���Ÿ�ʽ

	o_strDisplayWinName = pszDisplayWinName;
	o_pfcbDispPreprocessor = pfcbDispPreprocessor;
}

//* ��ͣ/�ָ���ǰ��Ƶ/����������Ĳ���
void MVLVideo::pause(BOOL blIsPaused)
{
	libvlc_media_player_set_pause(o_pstVLCMediaPlayer, blIsPaused);
}

//* ֹͣ���ŵ�ǰ��Ƶ/��������
void MVLVideo::stop(void)
{
	libvlc_media_player_stop(o_pstVLCMediaPlayer);
}

BOOL MVLVideo::start(void)
{
	if (libvlc_media_player_play(o_pstVLCMediaPlayer) < 0)
	{
		cout << "libvlc_media_player_play()����ִ��ʧ�ܣ���ȷ����Ƶ�ļ���RTSP��ý���ַ��ȷ��" << endl;

		return FALSE;
	}		

	return TRUE;
}

//* ��Ƶ�ļ��Ƿ��Ѿ�������ϣ���������������������Ч
BOOL MVLVideo::IsPlayEnd(void)
{
	libvlc_state_t enumVLCPlayState = libvlc_media_player_get_state(o_pstVLCMediaPlayer);	
	if (enumVLCPlayState == libvlc_Ended || enumVLCPlayState == libvlc_Stopped)
		return TRUE;

	return FALSE;
}
