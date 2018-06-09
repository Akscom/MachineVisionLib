#pragma once
#ifdef MACHINEVISIONLIB_EXPORTS
#define IMGPREPROC __declspec(dllexport)
#else
#define IMGPREPROC __declspec(dllimport)
#endif

//* �����ڵ�
typedef struct _ST_CONTOUR_NODE_ {
	_ST_CONTOUR_NODE_ *pstPrevNode;
	_ST_CONTOUR_NODE_ *pstNextNode;

	INT nIndex;
	vector<Point> *pvecContour;
} ST_CONTOUR_NODE, *PST_CONTOUR_NODE;


typedef struct _ST_CONTOUR_GROUP_ {
	PST_CONTOUR_NODE pstContourLink;
	INT nContourNum;
} ST_CONTOUR_GROUP, *PST_CONTOUR_GROUP;

//* ͼ��������
class IMGPREPROC ImgContour {
public:
	ImgContour(Mat& matInputSrcImg) {
		
	}

	~ImgContour() {}

	//* �����ڵ�
	typedef struct _ST_CONTOUR_NODE_ {
		_ST_CONTOUR_NODE_ *pstPrevNode;
		_ST_CONTOUR_NODE_ *pstNextNode;

		INT nIndex;
		vector<Point> *pvecContour;
	} ST_CONTOUR_NODE, *PST_CONTOUR_NODE;


	typedef struct _ST_CONTOUR_GROUP_ {
		PST_CONTOUR_NODE pstContourLink;
		INT nContourNum;
	} ST_CONTOUR_GROUP, *PST_CONTOUR_GROUP;

private:
	Mat matPreProcedImg;
};
 

//* ͼ��Ԥ�����
namespace imgpreproc {
	IMGPREPROC void HistogramEqualization(Mat& matInputGrayImg, Mat& matDestImg);	//* ����ֱ��ͼ�����㷨��ǿͼ��ĶԱȶȺ�������
	IMGPREPROC void ContrastEqualization(Mat& matInputGrayImg, Mat& matDestImg, DOUBLE dblGamma = 0.4,
												DOUBLE dblPowerValue = 0.1, DOUBLE dblNorm = 10);									//* �ԱȶȾ����㷨��ǿͼ��
	IMGPREPROC void ContrastEqualizationWithFilter(Mat& matInputGrayImg, Mat& matDestImg, Size size, FLOAT *pflaFilterKernel, 
														DOUBLE dblGamma = 0.4, DOUBLE dblPowerValue = 0.1, DOUBLE dblNorm = 10);	//* ���˲��ĶԱȶȾ����㷨��ǿͼ��

	//IMGPREPROC_API void FindContourUsingCanny(Mat& matSrcImg, );
};

