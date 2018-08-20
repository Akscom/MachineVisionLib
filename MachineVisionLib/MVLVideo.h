#pragma once
#ifdef MACHINEVISIONLIB_EXPORTS
#define MVLVIDEO __declspec(dllexport)
#else
#define MVLVIDEO __declspec(dllimport)
#endif

#include <vlc/vlc.h>

#pragma comment(lib,"libvlc.lib")
#pragma comment(lib,"libvlccore.lib")

#define DEFAULT_VIDEO_FRAME_WIDTH	960	//* ȱʡ֡��

typedef void(*PFCB_DISPLAY_PREPROCESSOR)(Mat& mVideoFrame);

//* �����߱ȣ����������
typedef enum {
	AR_16_9 = 0,	//* 16:9
	AR_4_3			//* 4:3
} ENUM_ASPECT_RATIO;

class MVLVIDEO MVLVideo {
public:
	MVLVideo() : o_unAdjustedWidth(0),				 
				 o_pstVLCInstance(NULL), 
				 o_pstVLCMediaPlayer(NULL), 
				 o_unNextFrameIdx(0), 
				 o_unPrevFrameIdx(0)
	{
	};

	//* �ֶ�ָ���̶����
	MVLVideo(UINT unAdjustedWidth) : o_pstVLCInstance(NULL), 
									 o_pstVLCMediaPlayer(NULL), 
									 o_unNextFrameIdx(0), 
									 o_unPrevFrameIdx(0)
	{
		o_unAdjustedWidth = unAdjustedWidth;		
	};

	//* �ͷ�����������Դ
	~MVLVideo() {
		o_mVideoFrame.release();
		libvlc_media_player_stop(o_pstVLCMediaPlayer);
		libvlc_media_player_release(o_pstVLCMediaPlayer);
		libvlc_release(o_pstVLCInstance);		
	};
	
	BOOL OpenVideoFromFile(const CHAR *pszFile, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor, const CHAR *pszDisplayWinName, ENUM_ASPECT_RATIO enumAspectRatio = AR_16_9);
	BOOL OpenVideoFromeRtsp(const CHAR *pszURL, PFCB_DISPLAY_PREPROCESSOR pfcbDispPreprocessor,
							const CHAR *pszDisplayWinName, UINT unNetCachingTime = 200, 
							BOOL blIsUsedTCP = FALSE, ENUM_ASPECT_RATIO enumAspectRatio = AR_16_9);	//* ����unNetCachingTimeΪVLC���Ż����ʱ�䣬Ҳ���ǻ���һ��ʱ�����Ƶ�ٿ�ʼ���ţ���λΪ����

	Mat GetNextFrame(void);
	

private:
	static void *FCBLock(void *pvCBInputParam, void **ppvFrameBuf);								//* VLC�ص�����֮����֡���ݻ���������������pvCBInputParam�Ǵ����ص��������û��Զ������
	static void FCBUnlock(void *pvCBInputParam, void *pvFrameData, void *const *ppvFrameBuf);	//* VLC�ص�����֮����֡���ݻ�������������ʵpvFrameData��ppvFrameBufָ�����ͬһ���ڴ棬��ֵ��FCBLock()��������
	static void FCBDisplay(void *pvCBInputParam, void *pvFrameData);							//* VLC�ص�����֮��ʾ����

	libvlc_instance_t *o_pstVLCInstance;			//* VLCʵ��
	libvlc_media_player_t *o_pstVLCMediaPlayer;		//* VLC������

	Mat o_mVideoFrame;			//* ��ȡ������Ƶ֡
	Mat o_mDisplayFrame;		//* ������ʾ��֡����

	PFCB_DISPLAY_PREPROCESSOR o_pfcbDispPreprocessor;
	string o_strDisplayWinName;	//* ��ʾ��ʾ���ڵ�����

	UINT o_unNextFrameIdx;
	UINT o_unPrevFrameIdx;

	UINT o_unAdjustedWidth;		//* ������ָ��Ҫ��������ͼ���ȣ�ϵͳ���ݴ˵���ͼ��ķֱ��ʵ��ÿ�ȣ�ͼ��߶Ȱ�ָ�������������
	UINT o_unAdjustedHeight;
};