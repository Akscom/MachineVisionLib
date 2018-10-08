// MachineVisionLib.cpp : ���� DLL Ӧ�ó���ĵ���������
//
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <io.h>
#include <vector>
#include <facedetect-dll.h>
#include "common_lib.h"
#include "MachineVisionLib.h"

//* ִ��Canny��Ե���
MACHINEVISIONLIB void cv2shell::CV2Canny(Mat &matSrc, Mat &matOut)
{
	Mat matGray, matBlur;

	//* �һ�����
	cvtColor(matSrc, matGray, COLOR_BGR2GRAY);

	//* ģ������
	blur(matGray, matBlur, Size(4, 4));

	//* ��Ե
	Canny(matBlur, matOut, 0, 60, 3);
}

//* ִ��Canny��Ե���
MACHINEVISIONLIB void cv2shell::CV2Canny(const CHAR *pszImgName, Mat &matOut)
{
	Mat matImg = imread(pszImgName);
	if (!matImg.empty())
	{
		CV2Canny(matImg, matOut);
	}
}

//* �ȿ��Բ���һ��rtsp��Ƶ����video�ļ������Բ��ű�������ͷ�����ʵʱ��Ƶ�����紫�������ַ�����Ϊ������
//* rtsp://admin:abcd1234@192.168.0.250/mjpeg/ch1��ָ��������������ͷ�����rtspʵʱ��Ƶ��
//* ����blIsNeedToReplay����ָ������������ֹ���������ж�ʱ���Ƿ����������Ӳ��������ţ�������Ƶ�ļ���˵�����Ƿ�ѭ������
template <typename DType>
void cv2shell::CV2ShowVideo(DType dtVideoSrc, BOOL blIsNeedToReplay)
{
	Mat mSrc;
	VideoCapture video;
	BOOL blIsNotOpen = TRUE;
	DOUBLE dblFPS = 40.;

	while (TRUE)
	{
		if (blIsNotOpen)
		{
			if (video.open(dtVideoSrc))
			{
				dblFPS = video.get(CV_CAP_PROP_FPS);

				//video.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
				//video.set(CV_CAP_PROP_FRAME_HEIGHT, 960);

				cout << video.get(CV_CAP_PROP_FRAME_WIDTH) << " x " << video.get(CV_CAP_PROP_FRAME_HEIGHT) << " FPS: " << dblFPS << endl;

				blIsNotOpen = FALSE;
			}
			else
			{
				Sleep(1000);
				continue;
			}
		}

		if (video.read(mSrc))
		{
			String strWinName = String("��Ƶ����") + dtVideoSrc +  String("��");
			imshow(strWinName, mSrc);
			if (waitKey(1000.0 / dblFPS) == 27)
				break;
		}
		else
		{
			if (!blIsNeedToReplay)
			{
				video.release();
				return;
			}

			blIsNotOpen = TRUE;
			video.release();
			continue;
		}
	}
}

//* ͬ�ϣ�ֻ�������˻ص�������
template <typename DType>
void cv2shell::CV2ShowVideo(DType dtVideoSrc, PCB_VIDEOHANDLER pfunNetVideoHandler, DWORD64 dw64InputParam, BOOL blIsNeedToReplay)
{
	Mat mSrc;
	VideoCapture video;
	DOUBLE dblFPS = 40.0;

	bool blIsNotOpen = TRUE;

	while (TRUE)
	{
		if (blIsNotOpen)
		{
			if (video.open(dtVideoSrc))
			{
				dblFPS = video.get(CV_CAP_PROP_FPS);

				cout << video.get(CV_CAP_PROP_FRAME_WIDTH) << " x " << video.get(CV_CAP_PROP_FRAME_HEIGHT) << " FPS: " << dblFPS << endl;

				blIsNotOpen = FALSE;
			}
			else
			{
				Sleep(1000);
				continue;
			}
		}

		if (video.read(mSrc))
		{
			if (NULL != pfunNetVideoHandler)
			{
				pfunNetVideoHandler(mSrc, dw64InputParam);
			}

			if (waitKey(1000.0 / dblFPS) == 27)
				break;
		}
		else
		{
			if (!blIsNeedToReplay)
			{
				video.release();
				return;
			}

			blIsNotOpen = TRUE;
			video.release();
			continue;
		}
	}
}

//* ����ģ��alphaģ��ͼ�񣬸ú������������£�
/*
Mat mat(480, 640, CV_8UC4);	//* ��alphaͨ����mat

CV2CreateAlphaMat(mat);

vector<int> compression_params;
compression_params.push_back(IMWRITE_PNG_COMPRESSION);
compression_params.push_back(0);

try {
imwrite("alpha_test.png", mat, compression_params);
imshow("alpha_test.png", mat);
waitKey(0);
} catch (runtime_error &ex) {
cout << "ͼ��ת����PNG��ʽ��������ԭ��" << ex.what() << endl;
}
*/
MACHINEVISIONLIB void cv2shell::CV2CreateAlphaMat(Mat& mat)
{
	INT i, j;

	for (i = 0; i < mat.rows; i++)
	{
		for (j = 0; j < mat.cols; j++)
		{
			Vec4b &rgba = mat.at<Vec4b>(i, j);
			rgba[0] = UCHAR_MAX;	//* ��ɫ����
			rgba[1] = saturate_cast<uchar>((FLOAT(mat.cols - j)) / ((FLOAT)mat.cols) * UCHAR_MAX);	//* ��ɫ����,saturate_cast�����÷�ֹ������㷨����С��0���ߴ���255����(uchar���ܴ���255)
			rgba[2] = saturate_cast<uchar>((FLOAT(mat.rows - j)) / ((FLOAT)mat.rows) * UCHAR_MAX);	//* ��ɫ����
			rgba[3] = saturate_cast<uchar>(0.5 * (rgba[1] + rgba[2]));	//* alpha������͸����
		}
	}
}

//* ��ֵHash�㷨
MACHINEVISIONLIB string cv2shell::CV2HashValue(Mat& mSrc, PST_IMG_RESIZE pstResize)
{
	string strValues(pstResize->nRows * pstResize->nCols, '\0');

	Mat mImg;
	if (mSrc.channels() == 3)
		cvtColor(mSrc, mImg, CV_BGR2GRAY);
	else
		mImg = mSrc.clone();

	//* ��һ������С�ߴ硣��ͼƬ��С�����ܱ�8�����Ҵ���8�ĳߴ磬��ȥ��ͼƬ��ϸ�ڣ�����������ѹ�����Ŵ򿪵�ϸ�ڣ�
	resize(mImg, mImg, Size(pstResize->nRows, pstResize->nCols));
	//resize(matImg, matImg, Size(pstResize->nCols * 10, pstResize->nRows * 10));
	//imshow("ce", matImg);
	//waitKey(0);

	//* �ڶ�������ɫ��(Color Reduce)������С���ͼƬ��תΪ64���Ҷȡ�
	UCHAR *pubData;
	for (INT i = 0; i<mImg.rows; i++)
	{
		pubData = mImg.ptr<UCHAR>(i);
		for (INT j = 0; j<mImg.cols; j++)
		{
			pubData[j] = pubData[j] / 4;
		}
	}

	//imshow("����Сɫ�ʡ�", matImg);

	//* ������������ƽ��ֵ�������������صĻҶ�ƽ��ֵ��
	INT nAverage = (INT)mean(mImg).val[0];

	//* ���Ĳ����Ƚ����صĻҶȡ���ÿ�����صĻҶȣ���ƽ��ֵ���бȽϡ����ڻ����ƽ��ֵ��Ϊ1,С��ƽ��ֵ��Ϊ0
	//* C++������֮������Ȼ������������
	Mat mMask = (mImg >= (UCHAR)nAverage);

	//* ���岽�������ϣֵ����ʵ���ǽ�16���Ƶ�0��1ת��ASCII�ġ�0���͡�1������0x00->0x30��0x01->0x31������洢��һ�������ַ�����
	INT nIdx = 0;
	for (INT i = 0; i<mMask.rows; i++)
	{
		pubData = mMask.ptr<UCHAR>(i);
		for (INT j = 0; j<mMask.cols; j++)
		{
			if (pubData[j] == 0)
				strValues[nIdx++] = '0';
			else
				strValues[nIdx++] = '1';
		}
	}

	mImg.release();	//* ��ʵ���ͷ�Ҳ���ԣ�OpenCV��������ü��������ͷŵģ�˵������C++���ڴ��������ͷŵ�

	return strValues;
}

//* ��һ��CV2HashValue���������غ���
MACHINEVISIONLIB string cv2shell::CV2HashValue(const CHAR *pszImgName, PST_IMG_RESIZE pstResize)
{
	string strDuff;

	Mat matImg = imread(pszImgName);
	if (matImg.data == NULL)
		return strDuff;

	return CV2HashValue(matImg, pstResize);
}

//* ����ȱ���������Ϊ8����Сͼ��ߴ�������һά��2DͼƬ�ĳ�����ά�����ܱ�8�����Ҵ���8����СͼƬ�ߴ磬���磺
//* 72 x 128 -> 9 x 16
static void __GetResizeValue(PST_IMG_RESIZE pstResize)
{
	if (pstResize->nRows > 64 && pstResize->nCols > 64)
	{
		if (pstResize->nRows % 8 == 0 && pstResize->nCols % 8 == 0)
		{
			pstResize->nRows /= 8;
			pstResize->nCols /= 8;

			__GetResizeValue(pstResize);
		}
	}
}

//* ���ݹ�ϣ�����Ҫ����Ҫ��ͼƬ��С��ȥ��ͼƬϸ�ڣ��ú��������㰴������С�����ܱ�8��������СͼƬ�ߴ�
MACHINEVISIONLIB ST_IMG_RESIZE cv2shell::CV2GetResizeValue(Mat& mImg)
{
	ST_IMG_RESIZE stSize;

	stSize.nRows = EatZeroOfTheNumberTail(mImg.rows);
	stSize.nCols = EatZeroOfTheNumberTail(mImg.cols);

	__GetResizeValue(&stSize);

	return stSize;
}

//* ����ָ���ߴ�����ȱ�������ͼ���С�����ڳ���һ�µ�Ҫ�����Ϊ����һ�µ�ͼƬ��������Ϊ�̱߼��ϵ�һ���صı���ʹ��ȳ�
MACHINEVISIONLIB void cv2shell::ImgEquilateral(Mat& mImg, Mat& mResizeImg, INT nResizeLen, Size& objAddedEdgeSize, const Scalar& border_color)
{
	INT nTop = 0, nBottom = 0, nLeft = 0, nRight = 0, nDifference;
	Mat mBorderImg;

	if (mImg.rows == mImg.cols)
	{
		if (mImg.rows == nResizeLen)
		{
			objAddedEdgeSize.width  = 0;
			objAddedEdgeSize.height = 0;

			mResizeImg = mImg;

			return;
		}
	}

	INT nLongestEdge = max(mImg.rows, mImg.cols);

	if (mImg.rows < nLongestEdge)
	{
		nDifference = nLongestEdge - mImg.rows;
		nTop = nBottom = nDifference / 2;
	}
	else if (mImg.cols < nLongestEdge)
	{
		nDifference = nLongestEdge - mImg.cols;
		nLeft = nRight = nDifference / 2;
	}
	else
		goto __lblSize;

	//* ��ͼ�����ӱ߽磬ʹ��ͼƬ�ȱߣ�cv2.BORDER_CONSTANTָ���߽���ɫ��valueָ��
	copyMakeBorder(mImg, mBorderImg, nTop, nBottom, nLeft, nRight, BORDER_CONSTANT, border_color);

__lblSize:
	Size size(nResizeLen, nResizeLen);

	objAddedEdgeSize.width  = ((DOUBLE)nLeft) * ((DOUBLE)nResizeLen / (DOUBLE)mBorderImg.rows);
	objAddedEdgeSize.height = ((DOUBLE)nTop)  * ((DOUBLE)nResizeLen / (DOUBLE)mBorderImg.cols);

	if (nResizeLen < nLongestEdge)
		resize(mBorderImg, mResizeImg, size, INTER_AREA);
	else
		resize(mBorderImg, mResizeImg, size, INTER_CUBIC);
}

//* ��ͼƬ�ߴ����Ϊ�ȱ�ͼƬ����ʵ���Ƕ̱����ָ����ɫ������ʹ���볤�ߵȳ�
MACHINEVISIONLIB void cv2shell::ImgEquilateral(Mat& mImg, Mat& mResizeImg, Size& objAddedEdgeSize, const Scalar& border_color)
{
	INT nTop = 0, nBottom = 0, nLeft = 0, nRight = 0, nDifference;

	if (mImg.rows == mImg.cols)
	{
		objAddedEdgeSize.width = 0;
		objAddedEdgeSize.height = 0;

		mResizeImg = mImg;
		return;
	}

	INT nLongestEdge = max(mImg.rows, mImg.cols);

	if (mImg.rows < nLongestEdge)
	{
		nDifference = nLongestEdge - mImg.rows;
		nTop = nBottom = nDifference / 2;
	}
	else if (mImg.cols < nLongestEdge)
	{
		nDifference = nLongestEdge - mImg.cols;
		nLeft = nRight = nDifference / 2;
	}
	else;

	objAddedEdgeSize.width = nLeft;
	objAddedEdgeSize.height = nTop;

	//* ��ͼ�����ӱ߽磬��ͼƬ������ȳ���cv2.BORDER_CONSTANTָ���߽���ɫ��valueָ��
	copyMakeBorder(mImg, mResizeImg, nTop, nBottom, nLeft, nRight, BORDER_CONSTANT, border_color);
}

//* ���ݹ�ϣ�����Ҫ����Ҫ��ͼƬ��С��ȥ��ͼƬϸ�ڣ��ú��������㰴������С�����ܱ�8��������СͼƬ�ߴ�
MACHINEVISIONLIB ST_IMG_RESIZE cv2shell::CV2GetResizeValue(const CHAR *pszImgName)
{
	Mat matImg = imread(pszImgName);
	if (matImg.data == NULL)
		return{ 0, 0 };

	return CV2GetResizeValue(matImg);
}

ImgMatcher::ImgMatcher(INT nSetMatchThresholdValue)
{
	nMatchThresholdValue = nSetMatchThresholdValue;
}

//�����������
static int __HanmingDistance(string& str1, string& str2, PST_IMG_RESIZE pstResize)
{
	INT nPixelNum = pstResize->nRows * pstResize->nCols;

	if ((str1.size() != nPixelNum) || (str2.size() != nPixelNum))
		return -1;

	INT nDifference = 0;
	for (INT i = 0; i<nPixelNum; i++)
	{
		if (str1[i] != str2[i])
			nDifference++;
	}

	return nDifference;
}

BOOL ImgMatcher::InitImgMatcher(vector<string *>& vModelImgs)
{
	//* �Ȼ�ȡͼƬ�ߴ�
	string *pstrImgName = vModelImgs.at(0);
	stImgResize = cv2shell::CV2GetResizeValue(pstrImgName->c_str());

	//* ���ȡ������ͼƬ
	vector<string*>::iterator itImgName = vModelImgs.begin();
	for (; itImgName != vModelImgs.end(); itImgName++)
	{
		string strHashValue;

		//* ����ͼƬ�Ĺ�ϣֵ
		strHashValue = cv2shell::CV2HashValue(((string)**itImgName).c_str(), &stImgResize);
		if (strHashValue.empty())
		{
			//* �ͷŵ�֮ǰ�����string
			vector<string *>::iterator itHashValues = vModelImgHashValues.begin();
			for (; itHashValues != vModelImgHashValues.end(); itHashValues++)
				delete *itHashValues;

			return FALSE;
		}

		string *pstrHashValue = new string;
		*pstrHashValue = strHashValue;
		vModelImgHashValues.push_back(pstrHashValue);

		//* ֱ��ȡֵ�������һ��*��ȡ�����Ǳ���أ�Ҳ����pstrImgName��ֻ����һ��*������string
		//cout << **itImgName << endl;

		delete *itImgName;	//* ɾ��ʱ��������ǵ�ַ
	}

	return TRUE;
}

BOOL ImgMatcher::InitImgMatcher(const CHAR *pszModelImg)
{
	vector<string *> vModelImgs;
	string *pstrImgName = new string(pszModelImg);

	vModelImgs.push_back(pstrImgName);

	return InitImgMatcher(vModelImgs);
}

void ImgMatcher::UninitImgMatcher(void)
{
	//* �ͷŵ�֮ǰ�����string
	vector<string *>::iterator itHashValues = vModelImgHashValues.begin();
	for (; itHashValues != vModelImgHashValues.end(); itHashValues++)
		delete *itHashValues;
}

//* ����ģ��ͼƬ����ͼƬ���ƶȣ�ֻҪ������һ��ģ��ͼƬ�ĺ�������С����ֵ����ΪͼƬ���ƣ�����ֵ���£�
//* 0, ͼƬ���ƶȺ�С
//* 1, ͼƬ���ƶȺܴ�
INT ImgMatcher::ImgSimilarity(Mat& mImg)
{
	//* ����ͼƬ�Ĺ�ϣֵ
	string strHashValue = cv2shell::CV2HashValue(mImg, &stImgResize);

	vector<string *>::iterator itHashValues = vModelImgHashValues.begin();
	for (; itHashValues != vModelImgHashValues.end(); itHashValues++)
	{
		INT nDistance = __HanmingDistance(**itHashValues, strHashValue, &stImgResize);
		if (nDistance < nMatchThresholdValue)
			return 1;
	}

	return 0;
}

//* ����ģ��ͼƬ����ͼƬ���ƶȣ�ֻҪ������һ��ģ��ͼƬ�ĺ�������С����ֵ����ΪͼƬ���ƣ�����ֵ���£�
//* 0,  ͼƬ���ƶ�̫С
//* 1,  ͼƬ���ƶȺܴ�
//* -1��Ŀ��ͼƬ�򿪴���
INT ImgMatcher::ImgSimilarity(const CHAR *pszImgName)
{
	Mat matImg = imread(pszImgName);
	if (matImg.data == NULL)
		return -1;

	return ImgSimilarity(matImg);
}

//* ����ģ��ͼƬ����ͼƬ���ƶȣ�ֻҪ������һ��ģ��ͼƬ�ĺ�������С����ֵ����ΪͼƬ���ƣ�����ֵ���£�
//* 0, ͼƬ���ƶȺ�С
//* 1, ͼƬ���ƶȺܴ�
//* ����pnDistance�����ģ��ͼƬ�������������Ǹ�ֵ�������
INT ImgMatcher::ImgSimilarity(Mat& mImg, INT *pnDistance)
{
	INT nMaxDistance = 0, nRtnVal = 0;

	//* ����ͼƬ�Ĺ�ϣֵ
	string strHashValue = cv2shell::CV2HashValue(mImg, &stImgResize);

	vector<string *>::iterator itHashValues = vModelImgHashValues.begin();
	for (; itHashValues != vModelImgHashValues.end(); itHashValues++)
	{
		INT nDistance = __HanmingDistance(**itHashValues, strHashValue, &stImgResize);
		if (nDistance > nMaxDistance)
			nMaxDistance = nDistance;

		if (nDistance < nMatchThresholdValue)
		{
			nMaxDistance = nDistance;
			nRtnVal = 1;
			goto __lblEnd;
		}
	}

__lblEnd:
	*pnDistance = nMaxDistance;

	return nRtnVal;
}

INT ImgMatcher::ImgSimilarity(const CHAR *pszImgName, INT *pnDistance)
{
	Mat matImg = imread(pszImgName);
	if (matImg.data == NULL)
		return -1;

	return ImgSimilarity(matImg, pnDistance);
}

//* ����Shiqi Yu�����Ŀ�Դ��������ʵ�ֵ�������⺯��
//* ������������ڲ���matImg���Ѿ�����ı�����ͼƬ���ݣ����ڲ������ҵ��Ľ����
//* ע����ڲ���ָ��Ļ�������malloc�õ��ģ��ϲ���ú������������ͷŲſɣ��û���
//* ����һ��INT�ֽ�Ϊ��⵽��������������֮����Ǿ����������ͼƬ�е�����λ��
//* flScale:              �������ӣ���ʵ������������㷨���ò�ͬ��С�Ĵ��ڽ���ɨ
//*                       �裬ÿ����ɨ�贰��֮������ű���������ԽС������ٶ�Խ
//*                       ������Ȼ���Ȼ�Խ��
//*
//* nMinNeighbors:        �ж��ٸ�ɨ�贰�ڼ�⵽������������nMinNeighbors������Ϊ
//*                       ��������
//* 
//* nMinPossibleFaceSize: ������С���ܵĳߴ磬����֮������������㷨Ѱ����������С����
MACHINEVISIONLIB INT *cv2shell::FaceDetect(Mat& mImg, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	UCHAR *pubResultBuf = (UCHAR*)malloc(LIBFACEDETECT_BUFFER_SIZE);
	if (!pubResultBuf)
	{
		cout << "cv2shell::FaceDetect()����" << GetLastError() << endl;
		return NULL;
	}

	Mat mGrayImg;
	cvtColor(mImg, mGrayImg, CV_BGR2GRAY);

	INT *pnResult = NULL;
	pnResult = facedetect_multiview_reinforce(pubResultBuf, (unsigned char*)(mGrayImg.ptr(0)),
												mGrayImg.cols, mGrayImg.rows, mGrayImg.step,
		flScale, nMinNeighbors, nMinPossibleFaceSize);
	if (pnResult && *pnResult > 0)
		return pnResult;

	free(pubResultBuf);

	return NULL;
}

MACHINEVISIONLIB INT *cv2shell::FaceDetect(const CHAR *pszImgName, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return NULL;
	}

	return FaceDetect(mImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
}

//* �þ��ο��ǳ�����
MACHINEVISIONLIB void cv2shell::MarkFaceWithRectangle(Mat& mImg, INT *pnFaces)
{
	for (INT i = 0; i < *pnFaces; i++)
	{
		SHORT *psScalar = ((SHORT*)(pnFaces + 1)) + LIBFACEDETECT_RESULT_STEP * i;
		INT x = psScalar[0];
		INT y = psScalar[1];
		INT nWidth = psScalar[2];
		INT nHeight = psScalar[3];
		INT nNeighbors = psScalar[4];

		Point left(x, y);
		Point right(x + nWidth, y + nHeight);
		rectangle(mImg, left, right, Scalar(230, 255, 0), 1);
	}

	free(pnFaces);
}

MACHINEVISIONLIB void cv2shell::MarkFaceWithRectangle(Mat& mImg, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	INT *pnFaces = FaceDetect(mImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
	if (!pnFaces)
		return;

	MarkFaceWithRectangle(mImg, pnFaces);

	imshow("Face Detect Result", mImg);
}

MACHINEVISIONLIB void cv2shell::MarkFaceWithRectangle(const CHAR *pszImgName, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::MarkFaceWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;
		return;
	}

	MarkFaceWithRectangle(mImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
}

//* ��ʼ��������������DNN����
MACHINEVISIONLIB Net cv2shell::InitFaceDetectDNNet(void)
{
	String strModelCfgFile = "C:\\Windows\\System32\\models\\resnet\\deploy.prototxt";
	String strModelFile = "C:\\Windows\\System32\\models\\resnet\\res10_300x300_ssd_iter_140000.caffemodel";

	dnn::Net dnnNet = readNetFromCaffe(strModelCfgFile, strModelFile);
	if (dnnNet.empty())
		cout << "DNN���罨��ʧ�ܣ���ȷ����������������ļ���deploy.prototxt����ģ���ļ���resnet_ssd.caffemodel�������Ҹ�ʽ��ȷ" << endl;

	return dnnNet;
}

//* ʹ��DNN������ѵ���õ�ģ�Ϳ�����������⺯��
static Mat __DNNFaceDetect(Net& dnnNet, Mat& mImg, const Size& size, Size& objAddedEdgeSize, FLOAT flScale, const Scalar& mean)
{
	//* ����ͼƬ�ļ�����һ����size����ָ��ͼƬҪ���ŵ�Ŀ��ߴ磬menaָ��Ҫ��ȥ��ƽ��ֵ��ƽ��������������������ɫͨ����Ҫ����
	//* ����blobFromImage:
	//* image: ����ͼ��
	//* scalefactor�� ���ģ��ѵ��ʱ��һ������0-1֮�䣬��ô���������Ӧ����1.0f/256.0f������Ϊ1
	//* size: Ӧ����ѵ��ʱ������ͼ��ߴ籣��һ�£������Ӧ����300 x 300
	//* mean����ֵ����ģ��ѵ��ʱ��ֵһ�£���������ʹ�õ���Ԥѵ��ģ�ͣ���ֵ�̶�
	//* swapRB: �Ƿ񽻻�ͼ���1��ͨ�������һ��ͨ����˳�����ﲻ��Ҫ
	//* crop: TRUE��������size�ü�ͼ�񣬷���ֱ�ӽ�ͼ�������Size�ߴ�
	Mat mInputBlob = blobFromImage(mImg, flScale, size, mean, FALSE, FALSE);

	//* ������������
	dnnNet.setInput(mInputBlob, "data");

	//* �����������
	Mat mDetection = dnnNet.forward("detection_out");

	Mat mFaces(mDetection.size[2], mDetection.size[3], CV_32F, mDetection.ptr<FLOAT>());

	//* û�����ӱ߿�����Ҫ��ӳ������λ��
	if (!objAddedEdgeSize.width && !objAddedEdgeSize.height)
		return mFaces;

	//* �������ӵı߿���ϵ������������λ��ת��Ϊԭʼͼ���λ��
	FLOAT flSrcImgWidth = (FLOAT)(mImg.cols - 2 * objAddedEdgeSize.width);
	FLOAT flSrcImgHeight = (FLOAT)(mImg.rows - 2 * objAddedEdgeSize.height);
	for (INT i = 0; i < mFaces.rows; i++)
	{
		FLOAT flConfidenceVal = mFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < 0.3)	//* ����0.3�ľͲ������
			continue;

		mFaces.at<FLOAT>(i, 3) = ((FLOAT)(static_cast<INT>(mFaces.at<FLOAT>(i, 3) * mImg.cols) - objAddedEdgeSize.width))  / flSrcImgWidth;
		mFaces.at<FLOAT>(i, 4) = ((FLOAT)(static_cast<INT>(mFaces.at<FLOAT>(i, 4) * mImg.rows) - objAddedEdgeSize.height)) / flSrcImgHeight;
		mFaces.at<FLOAT>(i, 5) = ((FLOAT)(static_cast<INT>(mFaces.at<FLOAT>(i, 5) * mImg.cols) - objAddedEdgeSize.width))  / flSrcImgWidth;
		mFaces.at<FLOAT>(i, 6) = ((FLOAT)(static_cast<INT>(mFaces.at<FLOAT>(i, 6) * mImg.rows) - objAddedEdgeSize.height)) / flSrcImgHeight;
	}

	return mFaces;
}

//* ��ͼƬ�ȱ�����С��ָ������
static Size __ResizeImgToSpecPixel(Mat& mImg, INT nMinPixel)
{
	INT *pnMin, *pnMax, nRows, nCols;

	//* �Ѿ�С��ָ������Сֵ�ˣ��Ǿ�û��Ҫ��������
	if (mImg.rows <= nMinPixel || mImg.cols <= nMinPixel)
		return Size(mImg.cols, mImg.rows);

	//* �����ֱ�ӷ���ָ������Сֵ�Ϳ�����
	if (mImg.rows == mImg.cols)
		return Size(nMinPixel, nMinPixel);

	nRows = mImg.rows;
	nCols = mImg.cols;

	pnMin = &nCols;
	pnMax = &nRows;

	if (mImg.rows < mImg.cols)
	{
		pnMin = &nRows;
		pnMax = &nCols;
	}

	if (nMinPixel < 260)
		nMinPixel = 260;

	FLOAT flScale = ((FLOAT)nMinPixel) / ((FLOAT)(*pnMin));
	*pnMax = (INT)(((FLOAT)*pnMax) * flScale);
	*pnMin = nMinPixel;

	return Size(nCols, nRows);
}

//* �ú��������Զ��ȱ���resizeͼƬ�ߴ絽300�������ң���ȷ�����ʶ��Ч��
MACHINEVISIONLIB Mat cv2shell::FaceDetect(Net& dnnNet, Mat& mImg, ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar& mean)
{
	if (enumMethod == EIRSZM_EQUALRATIO)
	{
		//Size size = __ResizeImgToSpecPixel(matImg, 288); //* ʵ��288Ч�����		
		
		return __DNNFaceDetect(dnnNet, mImg, Size(300, 300), Size(0, 0), flScale, mean);
	}
	else
	{
		Mat matEquilateralImg;
		Size objAddedEdgeSize;

		ImgEquilateral(mImg, matEquilateralImg, objAddedEdgeSize);

		return __DNNFaceDetect(dnnNet, matEquilateralImg, Size(300, 300), objAddedEdgeSize, flScale, mean);
	}
}

MACHINEVISIONLIB Mat cv2shell::FaceDetect(Net& dnnNet, const CHAR *pszImgName, ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar& mean)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		Mat mat;

		return mat;
	}

	return FaceDetect(dnnNet, mImg, enumMethod, flScale, mean);
}

MACHINEVISIONLIB Mat cv2shell::FaceDetect(Net& dnnNet, const CHAR *pszImgName, const Size& size, FLOAT flScale, const Scalar& mean)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		Mat mat;

		return mat;
	}

	return __DNNFaceDetect(dnnNet, mImg, size, Size(0, 0), flScale, mean);
}

//* ����vFaces���ڱ����⵽��������Ϣ����һ���������
static void __DNNFaceDetect(Net& dnnNet, Mat& mImg, vector<Face> &vFaces, const Size& size, Size& objAddedEdgeSize, 
											FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar& mean)
{
	//* ����ͼƬ�ļ�����һ����size����ָ��ͼƬҪ���ŵ�Ŀ��ߴ磬menaָ��Ҫ��ȥ��ƽ��ֵ��ƽ��������������������ɫͨ����Ҫ����
	Mat matInputBlob = blobFromImage(mImg, flScale, size, mean, false, false);

	//* ������������
	dnnNet.setInput(matInputBlob, "data");

	//* �����������
	Mat matDetection = dnnNet.forward("detection_out");

	Mat matFaces(matDetection.size[2], matDetection.size[3], CV_32F, matDetection.ptr<float>());

	for (int i = 0; i < matFaces.rows; i++)
	{
		FLOAT flConfidenceVal = matFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < flConfidenceThreshold)
			continue;

		Face objFace;

		objFace.o_nLeftTopX = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * mImg.cols) - objAddedEdgeSize.width;
		objFace.o_nLeftTopY = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * mImg.rows) - objAddedEdgeSize.height;
		objFace.o_nRightBottomX = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * mImg.cols) - objAddedEdgeSize.width;
		objFace.o_nRightBottomY = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * mImg.rows) - objAddedEdgeSize.height;

		objFace.o_flConfidenceVal = flConfidenceVal;
		vFaces.push_back(objFace);
	}
}

MACHINEVISIONLIB void cv2shell::FaceDetect(Net& dnnNet, Mat& mImg, vector<Face>& vFaces, FLOAT flConfidenceThreshold,
												ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar& mean)
{
	if (enumMethod == EIRSZM_EQUALRATIO)
	{
		//Size size = __ResizeImgToSpecPixel(matImg, 288); //* ʵ��288Ч�����

		__DNNFaceDetect(dnnNet, mImg, vFaces, Size(300, 300), Size(0, 0), flConfidenceThreshold, flScale, mean);
	}
	else
	{
		Mat mEquilateralImg;
		Size objAddedEdgeSize;

		ImgEquilateral(mImg, mEquilateralImg, objAddedEdgeSize);

		__DNNFaceDetect(dnnNet, mEquilateralImg, vFaces, Size(300, 300), objAddedEdgeSize, flConfidenceThreshold, flScale, mean);
	}
}

MACHINEVISIONLIB void cv2shell::FaceDetect(Net& dnnNet, const CHAR *pszImgName, vector<Face>& vFaces,
												FLOAT flConfidenceThreshold, ENUM_IMGRESIZE_METHOD enumMethod, 
												FLOAT flScale, const Scalar& mean)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	FaceDetect(dnnNet, mImg, vFaces, flConfidenceThreshold, enumMethod, flScale, mean);
}

MACHINEVISIONLIB void cv2shell::FaceDetect(Net& dnnNet, const CHAR *pszImgName, vector<Face>& vFaces, const Size& size,
											FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar& mean)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	__DNNFaceDetect(dnnNet, mImg, vFaces, size, Size(0, 0), flConfidenceThreshold, flScale, mean);
}

//* ��������ͼƬ��չʾ�������þ��ο��ǳ����������Ԥ�����
//* ����flConfidenceThresholdָ����С���Ŷ���ֵ��Ҳ����Ԥ������������С����ֵ�����ڴ˸��ʵı����������������
MACHINEVISIONLIB void cv2shell::MarkFaceWithRectangle(Mat& mImg, Mat& mFaces, FLOAT flConfidenceThreshold, BOOL blIsShow)
{
	for (INT i = 0; i < mFaces.rows; i++)
	{
		FLOAT flConfidenceVal = mFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < flConfidenceThreshold)
			continue;

		INT nLeftTopX = static_cast<INT>(mFaces.at<FLOAT>(i, 3) * mImg.cols);
		INT nLeftTopY = static_cast<INT>(mFaces.at<FLOAT>(i, 4) * mImg.rows);
		INT nRightBottomX = static_cast<INT>(mFaces.at<FLOAT>(i, 5) * mImg.cols);
		INT nRightBottomY = static_cast<INT>(mFaces.at<FLOAT>(i, 6) * mImg.rows);

		//cout << nLeftTopX << " " << nLeftTopY << " " << nRightBottomX << " " << nRightBottomY << endl;

		//* ��������
		Rect rectObj(nLeftTopX, nLeftTopY, (nRightBottomX - nLeftTopX), (nRightBottomY - nLeftTopY));
		rectangle(mImg, rectObj, Scalar(0, 255, 0));

		//* �ڱ����ͼƬ��������Ŷȸ���
		//* ======================================================================================
		ostringstream oss;
		oss << flConfidenceVal;
		String strConfidenceVal(oss.str());
		String strLabel = "Face: " + strConfidenceVal;

		INT nBaseLine = 0;
		Size labelSize = getTextSize(strLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
		rectangle(mImg, Rect(Point(nLeftTopX, nLeftTopY - labelSize.height),
			Size(labelSize.width, labelSize.height + nBaseLine)),
			Scalar(255, 255, 255), CV_FILLED);
		putText(mImg, strLabel, Point(nLeftTopX, nLeftTopY), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
		//* ======================================================================================
	}

	if(blIsShow)
		imshow("Face Detect Result", mImg);
}

//* �þ��ο����������������
MACHINEVISIONLIB void cv2shell::MarkFaceWithRectangle(Net& dnnNet, const CHAR *pszImgName, const Size& size, 
															FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar& mean)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::MarkFaceWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	Mat mFaces = __DNNFaceDetect(dnnNet, mImg, size, Size(0, 0), flScale, mean);
	if (mFaces.empty())
		return;

	MarkFaceWithRectangle(mImg, mFaces, flConfidenceThreshold);
}

//* �þ��ο����������������
MACHINEVISIONLIB void cv2shell::MarkFaceWithRectangle(Net& dnnNet, const CHAR *pszImgName, FLOAT flConfidenceThreshold,
														ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar& mean)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "cv2shell::MarkFaceWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	Mat& mFaces = FaceDetect(dnnNet, mImg, enumMethod, flScale, mean);
	if (mFaces.empty())
		return;

	MarkFaceWithRectangle(mImg, mFaces, flConfidenceThreshold);
}

MACHINEVISIONLIB void cv2shell::MarkFaceWithRectangle(Mat& mImg, vector<Face>& vFaces, BOOL blIsShow)
{
	vector<Face>::iterator itFace = vFaces.begin();
	for (; itFace != vFaces.end(); itFace++)
	{
		Face objFace = *itFace;

		//* ��������
		Rect rectObj(objFace.o_nLeftTopX, objFace.o_nLeftTopY, (objFace.o_nRightBottomX - objFace.o_nLeftTopX), (objFace.o_nRightBottomY - objFace.o_nLeftTopY));
		rectangle(mImg, rectObj, Scalar(0, 255, 0));

		//* �ڱ����ͼƬ��������Ŷȸ���
		//* ======================================================================================
		ostringstream oss;
		oss << objFace.o_flConfidenceVal;
		String strConfidenceVal(oss.str());
		String strLabel = "Face: " + strConfidenceVal;

		INT nBaseLine = 0;
		Size labelSize = getTextSize(strLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
		rectangle(mImg, Rect(Point(objFace.o_nLeftTopX, objFace.o_nLeftTopY - labelSize.height),
			Size(labelSize.width, labelSize.height + nBaseLine)),
			Scalar(255, 255, 255), CV_FILLED);
		putText(mImg, strLabel, Point(objFace.o_nLeftTopX, objFace.o_nLeftTopY), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
		//* ======================================================================================
	}

	if (blIsShow)
	{
		namedWindow("Face Detect Result", 0);
		imshow("Face Detect Result", mImg);
	}	
}

//* �ϲ��ص��ľ��Σ�����vSrcRectsΪԭʼ�������ݣ��������þ��μ��Ƿ�����ص��ľ��Σ��ص��ľ��ν����ϲ��󱣴浽����vMergedRectsָ����ڴ���
MACHINEVISIONLIB void cv2shell::MergeOverlappingRect(vector<ST_DIAGONAL_POINTS> vSrcRects, vector<ST_DIAGONAL_POINTS>& vMergedRects)
{
	INT nMergeRectIndex = 0;

__lblLoop:
	if (!vSrcRects.size())
		return;

	vector<ST_DIAGONAL_POINTS>::iterator itSrcRect = vSrcRects.begin();

	vMergedRects.push_back(*itSrcRect);
	vSrcRects.erase(itSrcRect);

	itSrcRect = vSrcRects.begin();
	for (; itSrcRect != vSrcRects.end();)
	{
		//* ���������Ƿ��ཻ
		ST_DIAGONAL_POINTS stRectTarget = *itSrcRect;
		ST_DIAGONAL_POINTS stRectBase = vMergedRects[nMergeRectIndex];

		INT nMinX, nMaxX, nMinY, nMaxY;
		nMinX = max(stRectBase.point_left.x, stRectTarget.point_left.x);
		nMaxX = min(stRectBase.point_right.x, stRectTarget.point_right.x);
		nMinY = max(stRectBase.point_left.y, stRectTarget.point_left.y);
		nMaxY = min(stRectBase.point_right.y, stRectTarget.point_right.y);
		if (nMinX > nMaxX || nMinY > nMaxY)	//* ���ཻ
		{
			itSrcRect++;
			continue;
		}
		else
		{
			vMergedRects[nMergeRectIndex].point_left.x = min(stRectBase.point_left.x, stRectTarget.point_left.x);
			vMergedRects[nMergeRectIndex].point_left.y = min(stRectBase.point_left.y, stRectTarget.point_left.y);
			vMergedRects[nMergeRectIndex].point_right.x = max(stRectBase.point_right.x, stRectTarget.point_right.x);
			vMergedRects[nMergeRectIndex].point_right.y = max(stRectBase.point_right.y, stRectTarget.point_right.y);

			vSrcRects.erase(itSrcRect);
			itSrcRect = vSrcRects.begin();	//* ���¿�ʼ�����ཻ�ľ���
		}
	}

	nMergeRectIndex++;
	goto __lblLoop;
}

//* ��ԭʼͼ���ϱ��ʶ���������
void iOCV2DNNObjectDetector::MarkObject(Mat& mShowImg, vector<RecogCategory>& vObjects)
{
	vector<RecogCategory>::iterator itObject = vObjects.begin();

	for (; itObject != vObjects.end(); itObject++)
	{
		RecogCategory object = *itObject;

		//* ��������
		Rect rectObj(object.nLeftTopX, object.nLeftTopY, (object.nRightBottomX - object.nLeftTopX), (object.nRightBottomY - object.nLeftTopY));
		rectangle(mShowImg, rectObj, Scalar(0, 255, 0), 2);

		//cout << object.nLeftTopX << ", " << object.nLeftTopY << " - " << object.nRightBottomX << ", " << object.nRightBottomY << endl;

		//* �ڱ����ͼƬ��������Ŷȸ���
		//* ======================================================================================
		ostringstream oss;
		oss << object.flConfidenceVal;
		String strConfidenceVal(oss.str());
		String strLabel = object.strCategoryName + ": " + strConfidenceVal;

		INT nBaseLine = 0;
		Size labelSize = getTextSize(strLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
		rectangle(mShowImg, Rect(Point(object.nLeftTopX, object.nLeftTopY - labelSize.height),
					Size(labelSize.width, labelSize.height + nBaseLine)),
					Scalar(255, 255, 255), CV_FILLED);
		putText(mShowImg, strLabel, Point(object.nLeftTopX, object.nLeftTopY), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
		//* ======================================================================================
	}
}

//* ��ȡ��⵽��Ŀ�����������������⵽��Ŀ����������pflConfidenceOfExist������Ŷ�ֵ�������ڵĻ�pflConfidenceOfExist����һ��û�������ֵ
//* ����strObjectName�����InitLightClassifier()���������voc.names�ļ��л�ȡ����Ϊ���������ֻ�ܷ���voc.names�������ЩĿ������
INT iOCV2DNNObjectDetector::GetObjectNum(vector<RecogCategory>& vObjects, string strObjectName, FLOAT *pflConfidenceOfExist, FLOAT *pflConfidenceOfObjectNum)
{
	vector<RecogCategory>::iterator itObject = vObjects.begin();
	FLOAT flConfidenceSum = 0.0f, flMaxConfidence = 0.0f;
	INT nObjectNum = 0;

	for (; itObject != vObjects.end(); itObject++)
	{
		RecogCategory object = *itObject;

		if (object.strCategoryName == strObjectName)
		{
			nObjectNum++;

			if (flMaxConfidence < object.flConfidenceVal)
				flMaxConfidence = object.flConfidenceVal;

			flConfidenceSum += object.flConfidenceVal;
		}
	}

	if (nObjectNum)
	{
		if (pflConfidenceOfObjectNum)
			*pflConfidenceOfObjectNum = flConfidenceSum / ((FLOAT)nObjectNum);

		if (pflConfidenceOfExist)
			*pflConfidenceOfExist = flMaxConfidence;
	}

	return nObjectNum;
}

//* OCV2 DNN�ӿ�֮SSD�����
OCV2DNNObjectDetectorSSD::OCV2DNNObjectDetectorSSD(FLOAT flConfidenceThreshold, ENUM_DETECTOR enumDetector) :
													iOCV2DNNObjectDetector(flConfidenceThreshold), o_enumDetector(enumDetector)
{	
	string strVOCFile, strCfgFile, strModelFile;

	//* ���ݼ���������趨Ҫ����ģ���ļ�
	switch (enumDetector)
	{
	case MOBNETSSD:
		strVOCFile = "C:\\Windows\\System32\\models\\mobile_net_ssd\\voc.names";
		strCfgFile = "C:\\Windows\\System32\\models\\mobile_net_ssd\\MobileNetSSD_deploy.prototxt";
		strModelFile = "C:\\Windows\\System32\\models\\mobile_net_ssd\\MobileNetSSD_deploy.caffemodel";

		o_dblNormalCoef = 0.007843;
		o_objMean = Scalar(127.5, 127.5, 127.5);
		break;

	default:
		strVOCFile = "C:\\Windows\\System32\\models\\vgg_ssd\\voc.names";
		strCfgFile = "C:\\Windows\\System32\\models\\vgg_ssd\\deploy.prototxt";
		strModelFile = "C:\\Windows\\System32\\models\\vgg_ssd\\VGG_VOC0712_SSD_300x300_iter_120000.caffemodel";

		o_dblNormalCoef = 1.F;
		o_objMean = Scalar(104.F, 117.F, 123.F);

		break;
	}

	ifstream ifsClassNamesFile(strVOCFile);
	if (ifsClassNamesFile.is_open())
	{
		string strClassName = "";
		while (getline(ifsClassNamesFile, strClassName))
			o_vClassNames.push_back(strClassName);
	}
	else
	{
		throw runtime_error("Failed to open the voc.names file, please confirm if the file exist(the path is " + strVOCFile + ").");
	}

	o_objDNNNet = readNetFromCaffe(strCfgFile, strModelFile);	
}

//* OCV2 DNN����֮SSDģ��������
static void __SSDObjectDetect(Mat& mImg, Net& objDNNNet, vector<string>& vClassNames, vector<RecogCategory>& vObjects, const Size& objSize,
	Size& objAddedEdgeSize, FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar& objMean)
{
	if (mImg.channels() == 4)
		cvtColor(mImg, mImg, COLOR_BGRA2BGR);

	//* ����ͼƬ�ļ�����һ����size����ָ��ͼƬҪ���ŵ�Ŀ��ߴ磬meanָ��Ҫ��ȥ��ƽ��ֵ��ƽ��������������������ɫͨ����Ҫ����
	Mat mInputBlob = blobFromImage(mImg, flScale, objSize, objMean, FALSE, FALSE);

	//* ������������
	objDNNNet.setInput(mInputBlob, "data");

	//* �����������
	Mat mDetection = objDNNNet.forward("detection_out");

	Mat mIdentifyObjects(mDetection.size[2], mDetection.size[3], CV_32F, mDetection.ptr<FLOAT>());

	for (int i = 0; i < mIdentifyObjects.rows; i++)
	{
		FLOAT flConfidenceVal = mIdentifyObjects.at<FLOAT>(i, 2);

		if (flConfidenceVal < flConfidenceThreshold)
			continue;

		RecogCategory category;

		category.nLeftTopX = static_cast<INT>(mIdentifyObjects.at<FLOAT>(i, 3) * mImg.cols) - objAddedEdgeSize.width;
		category.nLeftTopY = static_cast<INT>(mIdentifyObjects.at<FLOAT>(i, 4) * mImg.rows) - objAddedEdgeSize.height;
		category.nRightBottomX = static_cast<INT>(mIdentifyObjects.at<FLOAT>(i, 5) * mImg.cols) - objAddedEdgeSize.width;
		category.nRightBottomY = static_cast<INT>(mIdentifyObjects.at<FLOAT>(i, 6) * mImg.rows) - objAddedEdgeSize.height;

		category.flConfidenceVal = flConfidenceVal;

		size_t tObjClass = (size_t)mIdentifyObjects.at<FLOAT>(i, 1);
		category.strCategoryName = vClassNames[tObjClass];
		vObjects.push_back(category);
	}
}

//* �������ͼ���ҳ���ʶ������
void OCV2DNNObjectDetectorSSD::detect(Mat& mSrcImg, vector<RecogCategory>& vObjects)
{
	Mat mEquilateralImg;
	Size objAddedEdgeSize;

	cv2shell::ImgEquilateral(mSrcImg, mEquilateralImg, objAddedEdgeSize);

	__SSDObjectDetect(mEquilateralImg, o_objDNNNet, o_vClassNames, vObjects, Size(300, 300), objAddedEdgeSize, o_flConfidenceThreshold, o_dblNormalCoef, o_objMean);
}

void OCV2DNNObjectDetectorSSD::detect(Mat& mSrcImg, vector<RecogCategory>& vObjects, vector<string>& vstrFilter)
{
	Mat mEquilateralImg;
	Size objAddedEdgeSize;

	cv2shell::ImgEquilateral(mSrcImg, mEquilateralImg, objAddedEdgeSize);

	vector<RecogCategory> vOriginalObjects;
	__SSDObjectDetect(mEquilateralImg, o_objDNNNet, o_vClassNames, vOriginalObjects, Size(300, 300), objAddedEdgeSize, o_flConfidenceThreshold, o_dblNormalCoef, o_objMean);

	if (!vstrFilter.empty())
	{
		//* ȡ������vstrFilterָ�����������Ķ���
		for (INT i = 0; i < vstrFilter.size(); i++)
		{
			for (INT k = 0; k < vOriginalObjects.size(); k++)
			{
				if (vOriginalObjects[k].strCategoryName == vstrFilter[i])
				{
					vObjects.push_back(vOriginalObjects[k]);
					break;
				}
			}
		}
	}
	else
	{
		vObjects = vOriginalObjects;
	}	
}

//* ��ʼ��Yolo2������ģ��
static Net __InitYolo2Detector(const CHAR *pszClassNameFile, const CHAR *pszModelCfgFile, const CHAR *pszModelWeightFile, vector<string>& vClassNames)
{
	Net objDNNNet;

	//* �����������
	ifstream ifsClassNamesFile(pszClassNameFile);
	if (ifsClassNamesFile.is_open())
	{
		string strClassName = "";
		while (getline(ifsClassNamesFile, strClassName))
			vClassNames.push_back(strClassName);
	}
	else
	{
		throw runtime_error("Failed to open the voc.names file, please confirm if the file exist(the path is " + string(pszClassNameFile) + ").");
	}

	return readNetFromDarknet(pszModelCfgFile, pszModelWeightFile);
}

//* OCV2 DNN�ӿ�֮YOLO2�����
OCV2DNNObjectDetectorYOLO2::OCV2DNNObjectDetectorYOLO2(FLOAT flConfidenceThreshold, ENUM_DETECTOR enumDetector) : 
														iOCV2DNNObjectDetector(flConfidenceThreshold), o_enumDetector(enumDetector)
{
	switch (enumDetector)
	{
	case YOLO2_TINY_VOC:
		o_objDNNNet = __InitYolo2Detector("C:\\Windows\\System32\\models\\yolov2\\voc.names",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2-tiny-voc.cfg",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2-tiny-voc.weights",
										  o_vClassNames);
		break;

	case YOLO2_VOC:
		o_objDNNNet = __InitYolo2Detector("C:\\Windows\\System32\\models\\yolov2\\voc.names",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2-voc.cfg",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2-voc.weights",
										  o_vClassNames);
		break;

	case YOLO2_TINY:
		o_objDNNNet = __InitYolo2Detector("C:\\Windows\\System32\\models\\yolov2\\coco.names",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2-tiny.cfg",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2-tiny.weights",
										  o_vClassNames);
		break;

	case YOLO2:
	default:
		o_objDNNNet = __InitYolo2Detector("C:\\Windows\\System32\\models\\yolov2\\coco.names",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2.cfg",
										  "C:\\Windows\\System32\\models\\yolov2\\yolov2.weights",
										  o_vClassNames);
		break;
	}
}

void OCV2DNNObjectDetectorYOLO2::detect(Mat& mSrcImg, vector<RecogCategory>& vObjects)
{
	if (mSrcImg.channels() == 4)
		cvtColor(mSrcImg, mSrcImg, COLOR_BGRA2BGR);

#define IMG_EQUILATERAL 0	//* �Ƿ���Ҫ��ͼ��ȱߴ�������ʵ�ⷢ�֣����ģ�Ͳ���Ҫ��ͼƬ�ȱߺ������룬�ȱߺ��Ԥ��Ч���������ã����

#if IMG_EQUILATERAL
	Mat mEquilateralImg;
	Size objAddedEdgeSize;

	if (mSrcImg.cols == mSrcImg.rows)
	{
		mEquilateralImg = mSrcImg;
		objAddedEdgeSize = Size(0, 0);
	}
	else
	{
		ImgEquilateral(mSrcImg, mEquilateralImg, objAddedEdgeSize);
	}
#endif	

#if IMG_EQUILATERAL
	//* ������Ҫ���������ݲ�Ԥ��
	Mat mInputBlob = blobFromImage(mEquilateralImg, 1 / 255.F, Size(416, 416), Scalar(), FALSE, FALSE);
#else
	//* ������Ҫ���������ݲ�Ԥ��
	Mat mInputBlob = blobFromImage(mSrcImg, 1 / 255.F, Size(416, 416), Scalar(), FALSE, FALSE);
#endif

	o_objDNNNet.setInput(mInputBlob, "data");
	Mat mDetection = o_objDNNNet.forward();

	//* ȡ��������
	/*
	* mDetection�����ŷ������ݣ�����Ϊ��λ��һ��һ�����壬�нṹ��ͬ���еĴ洢��λΪFLOAT���нṹ���£�
	* ��0��1�У�Ŀ�������������ĵ������ϵ�������ϵ��������ʵ�ʵ�����λ�ã�����ʵ��λ��[x��y]�ֱ����ͼ���п�cols���п�rows�õ���
	* ��2��3�У�Ŀ��������ռ��������Ŀ��width�͸߶�height
	* ��4   �У�δʹ�ã����岻��֪
	* ��5-N ��: Ԥѵ��ģ��֧�ֶ����ַ��࣬���ж����У�Yolo2ģ��֧��ʶ��80�����壬���������80�У�ÿһ����coco.names�ļ������ķ����б����ϵ��£�һһ��Ӧ����ֵΪ����������coco.names�б�����Ӧ����ĸ���
	*/
	for (INT i = 0; i<mDetection.rows; i++)
	{
		//* ��ȡ���������׵�ַ
		FLOAT *pflProbData = &mDetection.at<FLOAT>(i, YOLO2_PROBABILITY_DATA_INDEX);

		//* �������и������ݣ��ҳ������ʷ����λ��������ע����������������Ǵ�pflProbData��ʼ��
		UINT unMaxProbabilityIndex = max_element(pflProbData, pflProbData + mDetection.cols - YOLO2_PROBABILITY_DATA_INDEX) - pflProbData;

		//* ��ȡ����ֵ������������ֵ�ϵ͵ķ���
		FLOAT flConfidenceVal = mDetection.at<FLOAT>(i, unMaxProbabilityIndex + YOLO2_PROBABILITY_DATA_INDEX);
		if (flConfidenceVal < o_flConfidenceThreshold)
			continue;

		//* ���ĵ�����λ��
		FLOAT flCenterX = mDetection.at<FLOAT>(i, 0);
		FLOAT flCenterY = mDetection.at<FLOAT>(i, 1);
		FLOAT flObjWidth = mDetection.at<FLOAT>(i, 2);
		FLOAT flObjHeight = mDetection.at<FLOAT>(i, 3);

		RecogCategory category;

#if IMG_EQUILATERAL
		category.nLeftTopX = static_cast<INT>((flCenterX - flObjWidth / 2) * mEquilateralImg.cols) - objAddedEdgeSize.width;
		category.nLeftTopY = static_cast<INT>((flCenterY - flObjHeight / 2) * mEquilateralImg.rows) - objAddedEdgeSize.height;
		category.nRightBottomX = static_cast<INT>((flCenterX + flObjWidth / 2) * mEquilateralImg.cols) - objAddedEdgeSize.width;
		category.nRightBottomY = static_cast<INT>((flCenterY + flObjHeight / 2) * mEquilateralImg.rows) - objAddedEdgeSize.height;
#else
		category.nLeftTopX = static_cast<INT>((flCenterX - flObjWidth / 2) * mSrcImg.cols);
		category.nLeftTopY = static_cast<INT>((flCenterY - flObjHeight / 2) * mSrcImg.rows);
		category.nRightBottomX = static_cast<INT>((flCenterX + flObjWidth / 2) * mSrcImg.cols);
		category.nRightBottomY = static_cast<INT>((flCenterY + flObjHeight / 2) * mSrcImg.rows);
#endif

		category.flConfidenceVal = flConfidenceVal;
		category.strCategoryName = o_vClassNames[unMaxProbabilityIndex];
		vObjects.push_back(category);
	}

#undef IMG_EQUILATERAL
}

void OCV2DNNObjectDetectorYOLO2::detect(Mat& mSrcImg, vector<RecogCategory>& vObjects, vector<string>& vstrFilter)
{
	vector<RecogCategory> vOriginalObjects;
	detect(mSrcImg, vOriginalObjects);

	if (!vstrFilter.empty())
	{
		//* ȡ������vstrFilterָ�����������Ķ���
		for (INT i = 0; i < vstrFilter.size(); i++)
		{
			for (INT k = 0; k < vOriginalObjects.size(); k++)
			{
				if (vOriginalObjects[k].strCategoryName == vstrFilter[i])
				{
					vObjects.push_back(vOriginalObjects[k]);
					break;
				}
			}
		}
	}
	else
		vObjects = vOriginalObjects;
}

//* ������DNN�����ϻ��ѵ�ʱ�䣬��λ����
MACHINEVISIONLIB DOUBLE cv2shell::GetTimeSpentInNetDetection(Net& objDNNNet)
{
	vector<DOUBLE> vdblLayersTimings;
	DOUBLE dblFreq = getTickFrequency() / 1000;
	DOUBLE dblTime = objDNNNet.getPerfProfile(vdblLayersTimings) / dblFreq;

	return dblTime;
}

//* ��ʾ��������OpenCV�Ĵ��ڣ�����blIsShowing��TRUE����ʾ��FALSE������
MACHINEVISIONLIB void cv2shell::ShowImageWindow(const CHAR *pszWindowTitle, BOOL blIsShowing)
{
	HWND hWndOCVImgShow = (HWND)cvGetWindowHandle(pszWindowTitle);
	HWND hWndParentOCVImgShow = GetParent(hWndOCVImgShow);

	if(blIsShowing)
		ShowWindow(hWndParentOCVImgShow, SW_SHOW);
	else
		ShowWindow(hWndParentOCVImgShow, SW_HIDE);
}

//* ��֤��ͼƬԤ�����������ʵ����ȥ��ͼƬ�ı�������ɫ����
MACHINEVISIONLIB void cv2shell::CAPTCHAImgPreProcess(Mat& mSrcImg, Mat& mDstImg)
{
	if (mSrcImg.empty())
	{
		cout << "��⵽����mSrcImgΪ�գ�CAPTCHAImgPreProcess()�����޷���ͼƬ����Ԥ���������" << endl;
		return;
	}

	Mat mGrayImg;
	if (mSrcImg.channels() == 3)
		cvtColor(mSrcImg, mGrayImg, COLOR_BGR2GRAY);
	else
		mSrcImg.copyTo(mGrayImg);

	//* ���Ҷ�ͼ��ɫ��ת��Ҳ����ƫ������ɫ�䰵��ƫ���ı��������������ڶ�ֵ��ʱ����֤�벿�ֱ�ɰ�ɫ
	mGrayImg = 255 - mGrayImg;

	//* ����ͼ��ĻҶȾ�ֵ������ֵ�������ھ�ֵ�ĺ�ɫ�����ھ�ֵ�İ�ɫ
	DOUBLE dblMean = mean(mGrayImg)[0];
	threshold(mGrayImg, mDstImg, dblMean, 255, THRESH_BINARY);
}

//* ����size����ָ�����п�����������С�����ʱʹ�õĺ˴�С���Ƽ�ֵΪSize(3, 3)
MACHINEVISIONLIB void cv2shell::CAPTCHAImgPreProcess(Mat& mSrcImg, Mat& mDstImg, const Size& size)
{
	Mat mBinaryImg;
	CAPTCHAImgPreProcess(mSrcImg, mBinaryImg);

	Mat element = getStructuringElement(MORPH_RECT, size);
	morphologyEx(mBinaryImg, mDstImg, MORPH_OPEN, element);
}

CMachineVisionLib::CMachineVisionLib()
{
    return;
}

template <typename DType>
caffe::Net<DType>* caffe2shell::LoadNet(std::string strParamFile, std::string strModelFile, caffe::Phase phase)
{
	//* _access()�����ĵ�2������0������ļ��ķ���Ȩ�ޣ�0������ļ��Ƿ���ڣ�2��д��4������6����д
	if (_access(strParamFile.c_str(), 0) == -1)
	{
		cout << "file " << strParamFile << " not exist!" << endl;
		return NULL;
	}

	if (_access(strModelFile.c_str(), 0) == -1)
	{
		cout << "file " << strModelFile << " not exist!" << endl;
		return NULL;
	}

	caffe::Net<DType>* caNet(new caffe::Net<DType>(strParamFile, phase));
	caNet->CopyTrainedLayersFrom(strModelFile);

	return caNet;
}

//* ��ȡͼ������
template <typename DType> 
void caffe2shell::ExtractFeature(caffe::Net<DType> *pNet, caffe::MemoryDataLayer<DType> *pMemDataLayer, 
									Mat& matImgROI, DType *pdtaImgFeature, INT nFeatureDimension, const CHAR *pszBlobName)
{
	//* �����ݺͱ�ǩ��������
	vector<Mat> vmatImgROI;
	vector<INT> vnLabel;
	vmatImgROI.push_back(matImgROI);
	vnLabel.push_back(0);
	pMemDataLayer->AddMatVector(vmatImgROI, vnLabel);

	//* ǰ�򴫲�����ȡ��������
	PerformanceTimer objPerformTimer;
	objPerformTimer.start();
	pNet->Forward();	
	cout << "Execution time of caffe net Forward(): " << objPerformTimer.end() / 1000 << "ms." << endl;

	//* ����Blob��������ϸ��Blog��ַ���£�
	//* https://blog.csdn.net/junmuzi/article/details/52761379
	//* ����boost����ָ������ϸ�ĵ�ַ���£�
	//* https://www.cnblogs.com/helloamigo/p/3575098.html
	boost::shared_ptr<caffe::Blob<DType>> blobImgFeature = pNet->blob_by_name(pszBlobName);

	//* һ������cpu_data()��caffe���Զ�ͬ��GPU->CPU����Ȼû��ʹ��GPU����ô����һֱ��CPU��ߣ�caffe�Ͳ����Զ�ͬ���ˣ�
	const DType *pFeatureData = blobImgFeature->cpu_data();
	memcpy(pdtaImgFeature, pFeatureData, nFeatureDimension * sizeof(DType));
}

//* ��ȡͼ������
template <typename DType>
void caffe2shell::ExtractFeature(caffe::Net<DType> *pNet, caffe::MemoryDataLayer<DType> *pMemDataLayer,
									Mat& matImgROI, Mat& matImgFeature, INT nFeatureDimension, const CHAR *pszBlobName)
{
	//* �����ݺͱ�ǩ��������
	vector<Mat> vmatImgROI;
	vector<INT> vnLabel;
	vmatImgROI.push_back(matImgROI);
	vnLabel.push_back(0);
	pMemDataLayer->AddMatVector(vmatImgROI, vnLabel);

	//* ǰ�򴫲�����ȡ��������
	pNet->Forward();

	//* ����Blob��������ϸ��Blog��ַ���£�
	//* https://blog.csdn.net/junmuzi/article/details/52761379
	//* ����boost����ָ������ϸ�ĵ�ַ���£�
	//* https://www.cnblogs.com/helloamigo/p/3575098.html
	boost::shared_ptr<caffe::Blob<DType>> blobImgFeature = pNet->blob_by_name(pszBlobName);

	//* һ������cpu_data()��caffe���Զ�ͬ��GPU->CPU����Ȼû��ʹ��GPU����ô����һֱ��CPU��ߣ�caffe�Ͳ����Զ�ͬ���ˣ�
	const DType *pFeatureData = blobImgFeature->cpu_data();	
	memcpy(matImgFeature.data, pFeatureData, nFeatureDimension * sizeof(DType));
}
