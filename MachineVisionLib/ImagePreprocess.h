#pragma once
#ifdef MACHINEVISIONLIB_EXPORTS
#define IMGPREPROC_API __declspec(dllexport)
#else
#define IMGPREPROC_API __declspec(dllimport)
#endif

//* ͼ��Ԥ�����
namespace imgpreproc {
	IMGPREPROC_API void HistogramEnhancedDefinition(Mat& matInputGrayImg, Mat& matDestImg);	//* ��ǿͼ��ĶԱȶȺ�������
};

