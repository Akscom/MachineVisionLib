#pragma once
#include <vlc/vlc.h>

#pragma comment(lib,"libvlc.lib")
#pragma comment(lib,"libvlccore.lib")

//* �����߱ȣ����������
typedef enum {
	AR_16_9 = 0,	//* 16:9
	AR_4_3			//* 4:3
} ENUM_ASPECT_RATIO;

class MVLVideo {
public:
	MVLVideo() : o_unVideoWidth(0), 
				 o_unVideoHeight(0), 				 
				 o_unAdjustedWidth(0),				 
				 o_pstVLCInstance(NULL), 
				 o_pstVLCMediaPlayer(NULL) 
	{
		InitThreadMutex(&o_thMutex);
	};

	//* �ֶ�ָ���̶����
	MVLVideo(UINT unAdjustedWidth) : o_unVideoWidth(0), 
									 o_unVideoHeight(0), 									 
									 o_pstVLCInstance(NULL), 
									 o_pstVLCMediaPlayer(NULL)
	{
		o_unAdjustedWidth = unAdjustedWidth;
		InitThreadMutex(&o_thMutex);
	};

	//* �ͷ�����������Դ
	~MVLVideo() {
		o_mVideoFrame.release();
		libvlc_media_player_stop(o_pstVLCMediaPlayer);
		libvlc_media_player_release(o_pstVLCMediaPlayer);
		libvlc_release(o_pstVLCInstance);

		UninitThreadMutex(&o_thMutex);
	};

	//* ����unCachingTimeΪVLC���Ż����ʱ�䣬Ҳ���ǻ���һ��ʱ�����Ƶ�ٿ�ʼ���ţ���λΪ����
	BOOL OpenVideoFromFile(string strFile, ENUM_ASPECT_RATIO enumAspectRatio = AR_16_9);
	BOOL OpenVideoFromeRtsp(string strURL, UINT unNetCachingTime = 200, BOOL blIsUsedTCP = FALSE, ENUM_ASPECT_RATIO enumAspectRatio = AR_16_9);

private:
	void *FCBLock(void *pvCBInputParam, void **ppvFrameBuf);							//* VLC�ص�����֮����֡���ݻ���������������pvCBInputParam�Ǵ����ص��������û��Զ������
	void FCBUnlock(void *pvCBInputParam, void *pvFrameData, void *const *ppvFrameBuf);	//* VLC�ص�����֮����֡���ݻ�������������ʵpvFrameData��ppvFrameBufָ�����ͬһ���ڴ棬��ֵ��FCBLock()��������	

	libvlc_instance_t *o_pstVLCInstance;			//* VLCʵ��
	libvlc_media_player_t *o_pstVLCMediaPlayer;		//* VLC������

	THMUTEX o_thMutex;

	Mat o_mVideoFrame;			//* ��ȡ������Ƶ֡	

	UINT o_unVideoWidth;		//* ��Ƶ֡��ԭʼ���
	UINT o_unVideoHeight;		//* ��Ƶ֡��ԭʼ�߶�

	UINT o_unAdjustedWidth;		//* ������ָ��Ҫ��������ͼ���ȣ�ϵͳ���ݴ˵���ͼ��ķֱ��ʵ��ÿ�ȣ�ͼ��߶Ȱ�ԭ�б�������
	UINT o_unAdjustedHeight;	

	//* ����VLC��������ϸ˵���μ���https://blog.csdn.net/jxbinwd/article/details/72722833
	CHAR *o_pbaVLCArgs[] =
	{
		"-I",
		"dummy",
		"--ignore-config"
		//"--longhelp",		//* �������������Դ�VLC�Ĳ���˵�����������ݺܶ࣬����ڿ���̨�����ض���TXT�ļ��ٿ�
		//"--advanced",
		//"--rtsp-frame-buffer-size=1500000",	//* ��������������vlc���Զ�Ѱ��һ�������ʣ����鲻���ã�ֻ��������ʱ����һЩ�������Ƶ�ķֱ��ʹ���Ļ�
	};
};