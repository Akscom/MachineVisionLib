#pragma once
#ifdef MACHINEVISIONLIB_EXPORTS
#define MVLVIDEO __declspec(dllexport)
#else
#define MVLVIDEO __declspec(dllimport)
#endif

#include <vlc/vlc.h>

#pragma comment(lib,"libvlc.lib")
#pragma comment(lib,"libvlccore.lib")

//* �����߱ȣ����������
typedef enum {
	AR_16_9 = 0,	//* 16:9
	AR_4_3, 		//* 4:3
	AR_5_3,			//* 5:3
	AR_OTHER
} ENUM_ASPECT_RATIO;

typedef void(*PFCB_DISPLAY_PREPROCESSOR)(Mat& mVideoFrame, void *pvParam, UINT unCurFrameIdx);

class MVLVIDEO VLCVideoPlayer {
public:
	VLCVideoPlayer() : o_unAdjustedWidth(0),
					   o_pstVLCInstance(NULL), 
					   o_pstVLCMediaPlayer(NULL), 
					   o_unNextFrameIdx(0), 
					   o_unPrevFrameIdx(0), 
					   o_pfcbDispPreprocessor(NULL), 
					   o_blIsInited(FALSE)
	{		
	};

	//* �ֶ�ָ���̶����
	VLCVideoPlayer(UINT unAdjustedWidth) : o_pstVLCInstance(NULL),
										   o_pstVLCMediaPlayer(NULL), 
										   o_unNextFrameIdx(0), 
										   o_unPrevFrameIdx(0), 
										   o_pfcbDispPreprocessor(NULL), 
										   o_blIsInited(FALSE), 
										   o_unAdjustedWidth(unAdjustedWidth)
	{
	};

	//* �ͷ�����������Դ
	~VLCVideoPlayer();
	BOOL OpenVideoFromFile(const CHAR *pszFile, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor, const CHAR *pszDisplayWinName);
	BOOL OpenVideoFromeRtsp(const CHAR *pszURL, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor,
							const CHAR *pszDisplayWinName, UINT unNetCachingTime = 200, BOOL blIsUsedTCP = FALSE);	//* ����unNetCachingTimeΪVLC���Ż����ʱ�䣬Ҳ���ǻ���һ��ʱ�����Ƶ�ٿ�ʼ���ţ���λΪ����

	void SetDispPreprocessorInputParam(void *pvParam);

	Mat GetNextFrame(void);
	UINT GetCurFrameIndex(void) const
	{
		return o_unNextFrameIdx;
	}

	Size GetVideoResolution(void) const
	{
		return Size(o_unAdjustedWidth, o_unAdjustedHeight);
	}
	
	BOOL start(void);				//* ��ʼ����
	void pause(BOOL blIsPaused);	//* ��ͣ����
	void stop(void);				//* �ر���Ƶ����
	BOOL IsPlayEnd(void);			//* �Ƿ������

private:
	static void *FCBLock(void *pvCBInputParam, void **ppvFrameBuf);								//* VLC�ص�����֮����֡���ݻ���������������pvCBInputParam�Ǵ����ص��������û��Զ������
	static void FCBUnlock(void *pvCBInputParam, void *pvFrameData, void *const *ppvFrameBuf);	//* VLC�ص�����֮����֡���ݻ�������������ʵpvFrameData��ppvFrameBufָ�����ͬһ���ڴ棬��ֵ��FCBLock()��������
	static void FCBDisplay(void *pvCBInputParam, void *pvFrameData);							//* VLC�ص�����֮��ʾ����

	BOOL __GetOriginalResolution(const CHAR *pszDisplayWinName);	//* ��ȡ��Ƶ��ԭʼ�ߴ�
	void __SetPlaybackResolution(const CHAR *pszDisplayWinName);	//* ������Ƶ�Ĳ��ųߴ�

	libvlc_instance_t *o_pstVLCInstance;			//* VLCʵ��
	libvlc_media_player_t *o_pstVLCMediaPlayer;		//* VLC������

	UCHAR *o_pubaVideoFrameData;	//* ��Ƶ֡���ݻ�����
	UINT o_unFrameDataBufSize;		//* ֡���ݻ���������

	Mat o_mVideoFrame;				//* Opencv��Ƶ֡

	PFCB_DISPLAY_PREPROCESSOR o_pfcbDispPreprocessor;	//* ��Ƶ��ʾǰ��Ԥ������
	string o_strDisplayWinName;							//* ��ʾ��ʾ���ڵ�����
	void *o_pvFunCBDispPreprocParam;					//* ���ݸ�Ԥ�������Ĳ���

	UINT o_unNextFrameIdx;
	UINT o_unPrevFrameIdx;

	UINT o_unAdjustedWidth;		//* ������ָ��Ҫ��������ͼ���ȣ�ϵͳ���ݴ˵���ͼ��ķֱ��ʵ��ÿ�ȣ�ͼ��߶Ȱ�ָ�������������
	UINT o_unAdjustedHeight;	

	Size o_objOriginalResolution;	//* ��Ƶ�ߴ�

	BOOL o_blIsInited;				//* �Ƿ��ѳ�ʼ�����
};