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
	"-I"
	, "dummy"
	"--ignore-config" 	
	//, "--rtsp-frame-buffer-size=1500000"	//* ��������������vlc���Զ�Ѱ��һ�����ʵģ����鲻���ã�ֻ��������ʱ����һЩ�������Ƶ�ķֱ��ʹ���Ļ�
	//, "--longhelp"						//* �������������Դ�VLC�Ĳ���˵�����������ݺܶ࣬����ڿ���̨�����ض���TXT�ļ��ٿ�
	//, "--advanced"
};

//* VLC�ص�����֮����֡���ݻ���������������pvCBInputParam�Ǵ����ص��������û��Զ������
//* pvCBInputParamָ������֡���ݵĻ��������׵�ַ
void *VLCVideoPlayer::FCBLock(void *pvCBInputParam, void **ppvFrameBuf)
{	
	VLCVideoPlayer *pobjVideoPlayer = (VLCVideoPlayer *)pvCBInputParam;
	//*ppvFrameBuf = pobjVideoPlayer->o_mVideoFrameRGB.data;
	*ppvFrameBuf = pobjVideoPlayer->o_pubaVideoFrameData;		

	return *ppvFrameBuf;
}

//* VLC�ص�����֮����֡���ݻ�������������ʵpvFrameData��ppvFrameBufָ�����ͬһ���ڴ棬��ֵ��FCBLock()��������
void VLCVideoPlayer::FCBUnlock(void *pvCBInputParam, void *pvFrameData, void* const *ppvFrameBuf)
{
	VLCVideoPlayer *pobjVideoPlayer = (VLCVideoPlayer *)pvCBInputParam;
	Mat mVideoFrameRGB = Mat(pobjVideoPlayer->o_unAdjustedHeight, pobjVideoPlayer->o_unAdjustedWidth, CV_8UC3, pobjVideoPlayer->o_pubaVideoFrameData);
	cvtColor(mVideoFrameRGB, pobjVideoPlayer->o_mVideoFrame, CV_RGB2BGR);

	pobjVideoPlayer->o_unNextFrameIdx++;	
}

//* VLC�ص�����֮��ʾ����
void VLCVideoPlayer::FCBDisplay(void *pvCBInputParam, void *pvFrameData)
{
	VLCVideoPlayer *pobjVideoPlayer = (VLCVideoPlayer *)pvCBInputParam;

	if (pobjVideoPlayer->o_pfcbDispPreprocessor)
	{		
		pobjVideoPlayer->o_pfcbDispPreprocessor(pobjVideoPlayer->o_mVideoFrame, pobjVideoPlayer->o_pvFunCBDispPreprocParam, pobjVideoPlayer->o_unNextFrameIdx);
		imshow(pobjVideoPlayer->o_strDisplayWinName, pobjVideoPlayer->o_mVideoFrame);
	}	
}

Mat VLCVideoPlayer::GetNextFrame(void)
{
	//* ������һ֡�����Ƿ��Ѿ�����
	if (o_unPrevFrameIdx != o_unNextFrameIdx)
	{				
		o_unPrevFrameIdx = o_unNextFrameIdx;
		
		return o_mVideoFrame;		
	}
	else
	{
		Mat mDummy;
		return mDummy;
	}
}

//* ��һ����Ƶ�ļ�
void VLCVideoPlayer::OpenVideoFromFile(const CHAR *pszFile, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor,
									const CHAR *pszDisplayWinName, ENUM_ASPECT_RATIO enumAspectRatio)
{
	UINT unArgc = sizeof(l_pbaVLCBaseArgs) / sizeof(l_pbaVLCBaseArgs[0]);
	CHAR **ppbaVLCArgs = new CHAR *[unArgc];
	for (UINT i = 0; i < unArgc; i++)	//* ���ڲ�����Ƶ�ļ���ʹ��ȱʡ��������
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
	o_unFrameDataBufSize = o_unAdjustedWidth * o_unAdjustedHeight * 3;	
	o_pubaVideoFrameData = new UCHAR[o_unFrameDataBufSize];	

	o_pstVLCInstance = libvlc_new(unArgc, ppbaVLCArgs);	
	delete[] ppbaVLCArgs;	//* ɾ���ղ�����Ĳ�����������libvlc_new()���ú�Ͳ�����

	libvlc_media_t* pstVLCMedia = libvlc_media_new_path(o_pstVLCInstance, pszFile);
	o_pstVLCMediaPlayer = libvlc_media_player_new_from_media(pstVLCMedia);
	libvlc_media_release(pstVLCMedia);	//* libvlc_media_player_new_from_media()������Ϻ��ͷžͿ�����

	libvlc_video_set_callbacks(o_pstVLCMediaPlayer, FCBLock, FCBUnlock, FCBDisplay, this);								//* �趨�ص�����
	libvlc_video_set_format(o_pstVLCMediaPlayer, "RV24", o_unAdjustedWidth, o_unAdjustedHeight, o_unAdjustedWidth * 3);	//* �趨���Ÿ�ʽ	

	if (pfcbDispPreprocessor)
	{
		o_strDisplayWinName = pszDisplayWinName;
		o_pfcbDispPreprocessor = pfcbDispPreprocessor;
	}
}

//* ��һ������RTSP����
void VLCVideoPlayer::OpenVideoFromeRtsp(const CHAR *pszURL, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor,
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
	o_unFrameDataBufSize = o_unAdjustedWidth * o_unAdjustedHeight * 3;	
	o_pubaVideoFrameData = new UCHAR[o_unFrameDataBufSize];

	//cout << o_unAdjustedWidth << " * " << o_unAdjustedHeight << endl;
	
	o_pstVLCInstance = libvlc_new(unArgc, ppbaVLCArgs);
	delete[] ppbaVLCArgs;	//* ɾ���ղ�����Ĳ�����������libvlc_new()���ú�Ͳ�����

	libvlc_media_t* pstVLCMedia = libvlc_media_new_location(o_pstVLCInstance, pszURL);
	o_pstVLCMediaPlayer = libvlc_media_player_new_from_media(pstVLCMedia);
	libvlc_media_release(pstVLCMedia);	//* libvlc_media_player_new_from_media()������Ϻ��ͷžͿ�����

	libvlc_video_set_callbacks(o_pstVLCMediaPlayer, FCBLock, FCBUnlock, FCBDisplay, this);								//* �趨�ص�����
	libvlc_video_set_format(o_pstVLCMediaPlayer, "RV24", o_unAdjustedWidth, o_unAdjustedHeight, o_unAdjustedWidth * 3);	//* �趨���Ÿ�ʽ

	if (pfcbDispPreprocessor)
	{
		o_strDisplayWinName = pszDisplayWinName;
		o_pfcbDispPreprocessor = pfcbDispPreprocessor;
	}	
}

//* ������ʾԤ���������������
void VLCVideoPlayer::SetDispPreprocessorInputParam(void *pvParam)
{
	o_pvFunCBDispPreprocParam = pvParam;
}

//* ��ͣ/�ָ���ǰ��Ƶ/����������Ĳ���
void VLCVideoPlayer::pause(BOOL blIsPaused)
{
	libvlc_media_player_set_pause(o_pstVLCMediaPlayer, blIsPaused);
}

//* ֹͣ���ŵ�ǰ��Ƶ/��������
void VLCVideoPlayer::stop(void)
{
	libvlc_media_player_stop(o_pstVLCMediaPlayer);

	//* �ȴ�����
	libvlc_state_t enumVLCPlayState;
	while (TRUE)
	{
		enumVLCPlayState = libvlc_media_player_get_state(o_pstVLCMediaPlayer);
		if (enumVLCPlayState == libvlc_Stopped || enumVLCPlayState == libvlc_Ended || enumVLCPlayState == libvlc_Error)
			break;
	}
}

BOOL VLCVideoPlayer::start(void)
{
	if (libvlc_media_player_play(o_pstVLCMediaPlayer) < 0)
	{
		cout << "libvlc_media_player_play()����ִ��ʧ�ܣ���ȷ����Ƶ�ļ���RTSP��ý���ַ��ȷ��" << endl;

		return FALSE;
	}		

	return TRUE;
}

//* ��Ƶ�ļ��Ƿ��Ѿ�������ϣ���������������������Ч
BOOL VLCVideoPlayer::IsPlayEnd(void)
{
	libvlc_state_t enumVLCPlayState = libvlc_media_player_get_state(o_pstVLCMediaPlayer);	
	if (enumVLCPlayState == libvlc_Ended || enumVLCPlayState == libvlc_Stopped)
		return TRUE;

	return FALSE;
}

//* �ͷ�����������Դ
VLCVideoPlayer::~VLCVideoPlayer() {
	if (libvlc_media_player_get_state(o_pstVLCMediaPlayer) != libvlc_Stopped)
		libvlc_media_player_stop(o_pstVLCMediaPlayer);

	libvlc_media_player_release(o_pstVLCMediaPlayer);
	libvlc_release(o_pstVLCInstance);

	//* ע�����˳��libvlc����stop֮ǰһֱ��ʹ�����ǣ����������stop������ͷ�	
	o_mVideoFrame.release();
	delete[] o_pubaVideoFrameData;

	cout << "VLC player has successfully quit!" << endl;
};
