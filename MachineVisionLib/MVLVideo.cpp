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

	//* ��ȡ��Ƶ�ߴ�
	if (!pobjVideoPlayer->o_blIsInited)
	{
		UINT unWidth, unHeight;
		libvlc_video_get_size(pobjVideoPlayer->o_pstVLCMediaPlayer, 0, &unWidth, &unHeight);
		pobjVideoPlayer->o_objOriginalResolution = Size(unWidth, unHeight);

		cout << "Video Size: " << unWidth << " x " << unHeight << " : " << pobjVideoPlayer->o_objOriginalResolution.empty() << endl;
	}	

	return *ppvFrameBuf;
}

//* VLC�ص�����֮����֡���ݻ�������������ʵpvFrameData��ppvFrameBufָ�����ͬһ���ڴ棬��ֵ��FCBLock()��������
void VLCVideoPlayer::FCBUnlock(void *pvCBInputParam, void *pvFrameData, void* const *ppvFrameBuf)
{
	VLCVideoPlayer *pobjVideoPlayer = (VLCVideoPlayer *)pvCBInputParam;

	if (!pobjVideoPlayer->o_blIsInited)
		return;
	
	Mat mVideoFrameRGB = Mat(pobjVideoPlayer->o_unAdjustedHeight, pobjVideoPlayer->o_unAdjustedWidth, CV_8UC3, pobjVideoPlayer->o_pubaVideoFrameData);
	cvtColor(mVideoFrameRGB, pobjVideoPlayer->o_mVideoFrame, CV_RGB2BGR);

	pobjVideoPlayer->o_unNextFrameIdx++;	
}

//* VLC�ص�����֮��ʾ����
void VLCVideoPlayer::FCBDisplay(void *pvCBInputParam, void *pvFrameData)
{
	VLCVideoPlayer *pobjVideoPlayer = (VLCVideoPlayer *)pvCBInputParam;

	if (!pobjVideoPlayer->o_blIsInited)
		return;

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

//* ��ȡ��Ƶ��ԭʼ�ߴ�
BOOL VLCVideoPlayer::__GetOriginalResolution(const CHAR *pszDisplayWinName)
{
	if (!o_pstVLCMediaPlayer)
		return FALSE;

	if(pszDisplayWinName)
		cv2shell::ShowImageWindow(pszDisplayWinName, FALSE);

#define DUMMY_BORDER_LEN	10
	UCHAR ubaDummy[DUMMY_BORDER_LEN * DUMMY_BORDER_LEN * 3];
	o_pubaVideoFrameData = ubaDummy;
	libvlc_video_set_format(o_pstVLCMediaPlayer, "RV24", DUMMY_BORDER_LEN, DUMMY_BORDER_LEN, DUMMY_BORDER_LEN * 3);
	if (libvlc_media_player_play(o_pstVLCMediaPlayer) < 0)
	{
		cout << "The libvlc_media_player_play() function failed, Please confirm that the video address is correct." << endl;

		return FALSE;
	}

	//* ��ȴ�20�룬���ڻ����Ƶԭʼ������
	UINT unWaitTimeOut = 0;
	while (unWaitTimeOut < 2000)
	{
		if (!o_objOriginalResolution.empty())
		{
			if (o_objOriginalResolution.width && o_objOriginalResolution.height)
			{
				//* ֹͣ����
				libvlc_media_player_stop(o_pstVLCMediaPlayer);				

				return TRUE;
			}
		}

		Sleep(10);
		unWaitTimeOut++;
	}
#undef DUMMY_BORDER_LEN

	cout << "The VLCVideoPlayer::GetOriginalResolution() function timeout." << endl;

	return FALSE;
}

//* ������Ƶ�Ĳ��ųߴ�
void VLCVideoPlayer::__SetPlaybackResolution(const CHAR *pszDisplayWinName)
{
	ENUM_ASPECT_RATIO enumAspectRatio;	

	//* ������Ƶ�����
	if ((o_objOriginalResolution.width / 16) == (o_objOriginalResolution.height / 9))
		enumAspectRatio = AR_16_9;
	else if((o_objOriginalResolution.width / 4) == (o_objOriginalResolution.height / 3))
		enumAspectRatio = AR_4_3;
	else if ((o_objOriginalResolution.width / 5) == (o_objOriginalResolution.height / 3))
		enumAspectRatio = AR_5_3;
	else
		enumAspectRatio = AR_OTHER;

	//* Ȼ�󿴿��û��Ƿ�ָ����Ҫ�����ĳߴ�
	if (o_unAdjustedWidth)
	{
		switch (enumAspectRatio)
		{
		case AR_16_9:
			o_unAdjustedHeight = (o_unAdjustedWidth / 16) * 9;
			break;

		case AR_4_3:
			o_unAdjustedHeight = (o_unAdjustedWidth / 4) * 3;
			break;

		case AR_5_3:
			o_unAdjustedHeight = (o_unAdjustedWidth / 5) * 3;
			break;

		default:
			o_unAdjustedHeight = (UINT)(((FLOAT)o_objOriginalResolution.height) * (((FLOAT)o_unAdjustedWidth) / ((FLOAT)o_objOriginalResolution.width)));
			break;
		}		
	}
	else
	{
		//* ��ȡ��Ļ��С
		INT nPCWidth = GetSystemMetrics(SM_CXSCREEN);
		INT nPCHeight = GetSystemMetrics(SM_CYSCREEN);

		//* ��Ƶ�߶ȴ�����Ļ�߶ȣ�����С����Ļ�߶ȣ�����ɽ��ͨPC����16��9��4��3������5��3�ģ�����ֻҪ����Ƶ�߶��Ƿ񳬳����ɣ��߶Ȳ���������ȿ϶�������
		if (o_objOriginalResolution.height > nPCHeight)
		{
			o_unAdjustedHeight = nPCHeight;

			switch (enumAspectRatio)
			{
			case AR_16_9:
				o_unAdjustedWidth = (o_unAdjustedHeight / 9) * 16;
				break;

			case AR_4_3:
				o_unAdjustedWidth = (o_unAdjustedHeight / 3) * 4;
				break;

			case AR_5_3:
				o_unAdjustedWidth = (o_unAdjustedHeight / 3) * 5;
				break;

			default:
				o_unAdjustedWidth = (UINT)(((FLOAT)o_objOriginalResolution.width) * (((FLOAT)o_unAdjustedHeight) / ((FLOAT)o_objOriginalResolution.height)));
				break;
			}
		}
		else
		{
			o_unAdjustedWidth = o_objOriginalResolution.width;
			o_unAdjustedHeight = o_objOriginalResolution.height;
		}
	}

	//* ������Ƶ���ݽ��ջ�����
	o_unFrameDataBufSize = o_unAdjustedWidth * o_unAdjustedHeight * 3;
	o_pubaVideoFrameData = new UCHAR[o_unFrameDataBufSize];
	libvlc_video_set_format(o_pstVLCMediaPlayer, "RV24", o_unAdjustedWidth, o_unAdjustedHeight, o_unAdjustedWidth * 3);	//* �趨�����Ĳ��Ÿ�ʽ

	if (pszDisplayWinName)
		cv2shell::ShowImageWindow(pszDisplayWinName, TRUE);

	o_blIsInited = TRUE;
}

//* ��һ����Ƶ�ļ�
BOOL VLCVideoPlayer::OpenVideoFromFile(const CHAR *pszFile, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor, const CHAR *pszDisplayWinName)
{
	UINT unArgc = sizeof(l_pbaVLCBaseArgs) / sizeof(l_pbaVLCBaseArgs[0]);
	CHAR **ppbaVLCArgs = new CHAR *[unArgc];
	for (UINT i = 0; i < unArgc; i++)	//* ���ڲ�����Ƶ�ļ���ʹ��ȱʡ��������
	{
		ppbaVLCArgs[i] = l_pbaVLCBaseArgs[i];
	}

	o_pstVLCInstance = libvlc_new(unArgc, ppbaVLCArgs);	
	delete[] ppbaVLCArgs;	//* ɾ���ղ�����Ĳ�����������libvlc_new()���ú�Ͳ�����

	libvlc_media_t* pstVLCMedia = libvlc_media_new_path(o_pstVLCInstance, pszFile);
	o_pstVLCMediaPlayer = libvlc_media_player_new_from_media(pstVLCMedia);
	libvlc_media_release(pstVLCMedia);	//* libvlc_media_player_new_from_media()������Ϻ��ͷžͿ�����

	libvlc_video_set_callbacks(o_pstVLCMediaPlayer, FCBLock, FCBUnlock, FCBDisplay, this);	//* �趨�ص�����

	//* ��ȡ��Ƶ��ԭʼ�����ȣ��ߴ磩
	if (!__GetOriginalResolution(pszDisplayWinName))
		return FALSE;
	
	//* �����û�ָ���Ŀ�Ȼ���ʾ���ߴ��Զ�������Ƶ���ųߴ�
	__SetPlaybackResolution(pszDisplayWinName);

	if (pfcbDispPreprocessor)
	{
		o_strDisplayWinName = pszDisplayWinName;
		o_pfcbDispPreprocessor = pfcbDispPreprocessor;
	}

	return TRUE;
}

//* ��һ������RTSP����
BOOL VLCVideoPlayer::OpenVideoFromeRtsp(const CHAR *pszURL, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor,
									const CHAR *pszDisplayWinName, UINT unNetCachingTime, BOOL blIsUsedTCP)
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
	
	o_pstVLCInstance = libvlc_new(unArgc, ppbaVLCArgs);
	delete[] ppbaVLCArgs;	//* ɾ���ղ�����Ĳ�����������libvlc_new()���ú�Ͳ�����

	libvlc_media_t* pstVLCMedia = libvlc_media_new_location(o_pstVLCInstance, pszURL);
	o_pstVLCMediaPlayer = libvlc_media_player_new_from_media(pstVLCMedia);
	libvlc_media_release(pstVLCMedia);	//* libvlc_media_player_new_from_media()������Ϻ��ͷžͿ�����

	libvlc_video_set_callbacks(o_pstVLCMediaPlayer, FCBLock, FCBUnlock, FCBDisplay, this);	//* �趨�ص�����																													//* ��ȡ��Ƶ��ԭʼ�����ȣ��ߴ磩
	
	//* ��ȡ��Ƶ��ԭʼ�����ȣ��ߴ磩
	if (!__GetOriginalResolution(pszDisplayWinName))
		return FALSE;

	//* �����û�ָ���Ŀ�Ȼ���ʾ���ߴ��Զ�������Ƶ���ųߴ�
	__SetPlaybackResolution(pszDisplayWinName);

	if (pfcbDispPreprocessor)
	{
		o_strDisplayWinName = pszDisplayWinName;
		o_pfcbDispPreprocessor = pfcbDispPreprocessor;
	}

	return TRUE;
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
