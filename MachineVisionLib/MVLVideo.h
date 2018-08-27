#pragma once
#ifdef MACHINEVISIONLIB_EXPORTS
#define MVLVIDEO __declspec(dllexport)
#else
#define MVLVIDEO __declspec(dllimport)
#endif

#include <vlc/vlc.h>

#pragma comment(lib,"libvlc.lib")
#pragma comment(lib,"libvlccore.lib")

#define DEFAULT_VIDEO_FRAME_WIDTH	1280	//* ȱʡ֡��

typedef void(*PFCB_DISPLAY_PREPROCESSOR)(Mat& mVideoFrame, void *pvParam, UINT unCurFrameIdx);

//* �����߱ȣ����������
typedef enum {
	AR_16_9 = 0,	//* 16:9
	AR_4_3			//* 4:3
} ENUM_ASPECT_RATIO;

class MVLVIDEO VLCVideoPlayer {
public:
	VLCVideoPlayer() : o_unAdjustedWidth(0),
					   o_pstVLCInstance(NULL), 
					   o_pstVLCMediaPlayer(NULL), 
					   o_unNextFrameIdx(0), 
					   o_unPrevFrameIdx(0)
	{
	};

	//* �ֶ�ָ���̶����
	VLCVideoPlayer(UINT unAdjustedWidth) : o_pstVLCInstance(NULL),
										   o_pstVLCMediaPlayer(NULL), 
										   o_unNextFrameIdx(0), 
										   o_unPrevFrameIdx(0)
	{
		o_unAdjustedWidth = unAdjustedWidth;		
	};

	//* �ͷ�����������Դ
	~VLCVideoPlayer();
	
	void OpenVideoFromFile(const CHAR *pszFile, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor, const CHAR *pszDisplayWinName, ENUM_ASPECT_RATIO enumAspectRatio = AR_16_9);
	void OpenVideoFromeRtsp(const CHAR *pszURL, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor,
							const CHAR *pszDisplayWinName, UINT unNetCachingTime = 200, 
							BOOL blIsUsedTCP = FALSE, ENUM_ASPECT_RATIO enumAspectRatio = AR_16_9);	//* ����unNetCachingTimeΪVLC���Ż����ʱ�䣬Ҳ���ǻ���һ��ʱ�����Ƶ�ٿ�ʼ���ţ���λΪ����

	void SetDispPreprocessorInputParam(void *pvParam);

	Mat GetNextFrame(void);
	UINT GetCurFrameIndex(void)
	{
		return o_unNextFrameIdx;
	}
	
	BOOL start(void);				//* ��ʼ����
	void pause(BOOL blIsPaused);	//* ��ͣ����
	void stop(void);				//* �ر���Ƶ����
	BOOL IsPlayEnd(void);			//* �Ƿ������

private:
	static void *FCBLock(void *pvCBInputParam, void **ppvFrameBuf);								//* VLC�ص�����֮����֡���ݻ���������������pvCBInputParam�Ǵ����ص��������û��Զ������
	static void FCBUnlock(void *pvCBInputParam, void *pvFrameData, void *const *ppvFrameBuf);	//* VLC�ص�����֮����֡���ݻ�������������ʵpvFrameData��ppvFrameBufָ�����ͬһ���ڴ棬��ֵ��FCBLock()��������
	static void FCBDisplay(void *pvCBInputParam, void *pvFrameData);							//* VLC�ص�����֮��ʾ����

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
};