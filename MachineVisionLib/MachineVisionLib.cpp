// MachineVisionLib.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <io.h>
#include <vector>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/opencv/cv_image.h>
#include <dlib/opencv.h>

#include <facedetect-dll.h>
#include "common_lib.h"
#include "MachineVisionLib.h"

#if NEED_GPU
#pragma comment(lib,"cublas.lib")
#pragma comment(lib,"cuda.lib")
#pragma comment(lib,"cudart.lib")
#pragma comment(lib,"curand.lib")
#pragma comment(lib,"cudnn.lib")
#endif

//* ִ��Canny��Ե���
MACHINEVISIONLIB_API void cv2shell::CV2Canny(Mat &matSrc, Mat &matOut)
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
MACHINEVISIONLIB_API void cv2shell::CV2Canny(const CHAR *pszImgName, Mat &matOut)
{
	Mat matImg = imread(pszImgName);
	if (!matImg.empty())
	{
		CV2Canny(matImg, matOut);
	}
}

//* ����һ����·����ͷʵʱ��Ƶ������pszNetURL����ָ��ʵʱ��Ƶ���ŵ�ַ�����磺
//* rtsp://admin:abcd1234@192.168.0.250/mjpeg/ch1
MACHINEVISIONLIB_API void cv2shell::CV2ShowVideoFromNetCamera(const CHAR *pszNetURL)
{
	Mat mSrc;
	VideoCapture video;
	BOOL blIsNotOpen = TRUE;

	while (true)
	{
		if (blIsNotOpen)
		{
			if (video.open(pszNetURL))
				blIsNotOpen = FALSE;
			else
			{
				Sleep(1000);
				continue;
			}
		}

		if (video.read(mSrc))
		{
			imshow(pszNetURL, mSrc);
			if (waitKey(40) >= 0)
				break;
		}
		else
		{
			blIsNotOpen = TRUE;
			video.release();
			continue;
		}
	}
}

//* ��ȡ��������ͷ��ʵʱ��Ƶ���ݣ��������û�ͨ���ص�������ʽ�����յ���ʵʱ��Ƶ���ݣ������������£�
//* CV2ShowVideoFromNetCamera("rtsp://admin:abcd1234@192.168.0.250/mjpeg/ch1", CLSCV2Shell::CV2Canny);
MACHINEVISIONLIB_API void cv2shell::CV2ShowVideoFromNetCamera(const CHAR *pszNetURL, PCB_NETVIDEOHANDLER pfunNetVideoHandler)
{
	Mat mSrc, mHandleData;
	VideoCapture video;

	bool blIsNotOpen = TRUE;

	while (TRUE)
	{
		if (blIsNotOpen)
		{
			if (video.open(pszNetURL))
				blIsNotOpen = FALSE;
			else
			{
				Sleep(1000);
				continue;
			}
		}

		if (video.read(mSrc))
		{
			mHandleData = mSrc;
			if (NULL != pfunNetVideoHandler)
			{
				mHandleData = pfunNetVideoHandler(mSrc);
			}

			imshow(pszNetURL, mHandleData);

			if (waitKey(40) >= 0)
				break;
		}
		else
		{
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
MACHINEVISIONLIB_API void cv2shell::CV2CreateAlphaMat(Mat &mat)
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
MACHINEVISIONLIB_API string cv2shell::CV2HashValue(Mat &matSrc, PST_IMG_RESIZE pstResize)
{
	string strValues(pstResize->nRows * pstResize->nCols, '\0');

	Mat matImg;
	if (matSrc.channels() == 3)
		cvtColor(matSrc, matImg, CV_BGR2GRAY);
	else
		matImg = matSrc.clone();

	//* ��һ������С�ߴ硣��ͼƬ��С�����ܱ�8�����Ҵ���8�ĳߴ磬��ȥ��ͼƬ��ϸ�ڣ�����������ѹ�����Ŵ򿪵�ϸ�ڣ�
	resize(matImg, matImg, Size(pstResize->nRows, pstResize->nCols));
	//resize(matImg, matImg, Size(pstResize->nCols * 10, pstResize->nRows * 10));
	//imshow("ce", matImg);
	//waitKey(0);

	//* �ڶ�������ɫ��(Color Reduce)������С���ͼƬ��תΪ64���Ҷȡ�
	UCHAR *pubData;
	for (INT i = 0; i<matImg.rows; i++)
	{
		pubData = matImg.ptr<UCHAR>(i);
		for (INT j = 0; j<matImg.cols; j++)
		{
			pubData[j] = pubData[j] / 4;
		}
	}

	//imshow("����Сɫ�ʡ�", matImg);

	//* ������������ƽ��ֵ�������������صĻҶ�ƽ��ֵ��
	INT nAverage = (INT)mean(matImg).val[0];

	//* ���Ĳ����Ƚ����صĻҶȡ���ÿ�����صĻҶȣ���ƽ��ֵ���бȽϡ����ڻ����ƽ��ֵ��Ϊ1,С��ƽ��ֵ��Ϊ0
	//* C++������֮������Ȼ������������
	Mat matMask = (matImg >= (UCHAR)nAverage);

	//* ���岽�������ϣֵ����ʵ���ǽ�16���Ƶ�0��1ת��ASCII�ġ�0���͡�1������0x00->0x30��0x01->0x31������洢��һ�������ַ�����
	INT nIdx = 0;
	for (INT i = 0; i<matMask.rows; i++)
	{
		pubData = matMask.ptr<UCHAR>(i);
		for (INT j = 0; j<matMask.cols; j++)
		{
			if (pubData[j] == 0)
				strValues[nIdx++] = '0';
			else
				strValues[nIdx++] = '1';
		}
	}

	matImg.release();	//* ��ʵ���ͷ�Ҳ���ԣ�OpenCV��������ü��������ͷŵģ�˵������C++���ڴ��������ͷŵ�

	return strValues;
}

//* ��һ��CV2HashValue���������غ���
MACHINEVISIONLIB_API string cv2shell::CV2HashValue(const CHAR *pszImgName, PST_IMG_RESIZE pstResize)
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
MACHINEVISIONLIB_API ST_IMG_RESIZE cv2shell::CV2GetResizeValue(Mat &matImg)
{
	ST_IMG_RESIZE stSize;

	stSize.nRows = EatZeroOfTheNumberTail(matImg.rows);
	stSize.nCols = EatZeroOfTheNumberTail(matImg.cols);

	__GetResizeValue(&stSize);

	return stSize;
}

//* ����ָ���ߴ�����ȱ�������ͼ���С�����ڳ���һ�µ�Ҫ�����Ϊ����һ�µ�ͼƬ��������Ϊ�̱߲��ϵ�һ���صı���ʹ��ȳ�
MACHINEVISIONLIB_API void cv2shell::ImgEquilateral(Mat &matImg, Mat &matResizeImg, INT nResizeLen, const Scalar &border_color)
{
	INT nTop = 0, nBottom = 0, nLeft = 0, nRight = 0, nDifference;
	Mat matBorderImg;

	INT nLongestEdge = max(matImg.rows, matImg.cols);

	if (matImg.rows < nLongestEdge)
	{
		nDifference = nLongestEdge - matImg.rows;
		nTop = nDifference / 2;
		nBottom = nDifference - nTop;
	}
	else if (matImg.cols < nLongestEdge)
	{
		nDifference = nLongestEdge - matImg.cols;
		nLeft = nDifference / 2;
		nRight = nDifference - nLeft;
	}
	else;

	//* ��ͼ�����ӱ߽磬��ͼƬ������ȳ���cv2.BORDER_CONSTANTָ���߽���ɫ��valueָ��
	copyMakeBorder(matImg, matBorderImg, nTop, nBottom, nLeft, nRight, BORDER_CONSTANT, border_color);

	Size size(nResizeLen, nResizeLen);

	if (nResizeLen < nLongestEdge)
		resize(matBorderImg, matResizeImg, size, INTER_AREA);
	else
		resize(matBorderImg, matResizeImg, size, INTER_CUBIC);
}

//* ��ͼƬ�ߴ����Ϊ�ȱ�ͼƬ����ʵ���Ƕ̱����ָ����ɫ������ʹ���볤�ߵȳ�
MACHINEVISIONLIB_API void cv2shell::ImgEquilateral(Mat &matImg, Mat &matResizeImg, const Scalar &border_color)
{
	INT nTop = 0, nBottom = 0, nLeft = 0, nRight = 0, nDifference;

	INT nLongestEdge = max(matImg.rows, matImg.cols);

	if (matImg.rows < nLongestEdge)
	{
		nDifference = nLongestEdge - matImg.rows;
		nTop = nDifference / 2;
		nBottom = nDifference - nTop;
	}
	else if (matImg.cols < nLongestEdge)
	{
		nDifference = nLongestEdge - matImg.cols;
		nLeft = nDifference / 2;
		nRight = nDifference - nLeft;
	}
	else;

	//* ��ͼ�����ӱ߽磬��ͼƬ������ȳ���cv2.BORDER_CONSTANTָ���߽���ɫ��valueָ��
	copyMakeBorder(matImg, matResizeImg, nTop, nBottom, nLeft, nRight, BORDER_CONSTANT, border_color);
}

//* ���ݹ�ϣ�����Ҫ����Ҫ��ͼƬ��С��ȥ��ͼƬϸ�ڣ��ú��������㰴������С�����ܱ�8��������СͼƬ�ߴ�
MACHINEVISIONLIB_API ST_IMG_RESIZE cv2shell::CV2GetResizeValue(const CHAR *pszImgName)
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
static int __HanmingDistance(string &str1, string &str2, PST_IMG_RESIZE pstResize)
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

BOOL ImgMatcher::InitImgMatcher(vector<string *> &vModelImgs)
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
INT ImgMatcher::ImgSimilarity(Mat &matImg)
{
	//* ����ͼƬ�Ĺ�ϣֵ
	string strHashValue = cv2shell::CV2HashValue(matImg, &stImgResize);

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
INT ImgMatcher::ImgSimilarity(Mat &matImg, INT *pnDistance)
{
	INT nMaxDistance = 0, nRtnVal = 0;

	//* ����ͼƬ�Ĺ�ϣֵ
	string strHashValue = cv2shell::CV2HashValue(matImg, &stImgResize);

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
MACHINEVISIONLIB_API INT *cv2shell::FaceDetect(Mat &matImg, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	UCHAR *pubResultBuf = (UCHAR*)malloc(LIBFACEDETECT_BUFFER_SIZE);
	if (!pubResultBuf)
	{
		cout << "cv2shell::FaceDetect()����" << GetLastError() << endl;
		return NULL;
	}

	Mat matGrayImg;
	cvtColor(matImg, matGrayImg, CV_BGR2GRAY);

	INT *pnResult = NULL;
	pnResult = facedetect_multiview_reinforce(pubResultBuf, (unsigned char*)(matGrayImg.ptr(0)),
		matGrayImg.cols, matGrayImg.rows, matGrayImg.step,
		flScale, nMinNeighbors, nMinPossibleFaceSize);
	if (pnResult && *pnResult > 0)
		return pnResult;

	free(pubResultBuf);

	return NULL;
}

MACHINEVISIONLIB_API INT *cv2shell::FaceDetect(const CHAR *pszImgName, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return NULL;
	}

	return FaceDetect(matImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
}

//* �þ��ο��ǳ�����
MACHINEVISIONLIB_API void cv2shell::MarkFaceWithRectangle(Mat &matImg, INT *pnFaces)
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
		rectangle(matImg, left, right, Scalar(230, 255, 0), 1);
	}

	free(pnFaces);
}

MACHINEVISIONLIB_API void cv2shell::MarkFaceWithRectangle(const CHAR *pszImgName, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::MarkFaceWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;
		return;
	}

	INT *pnFaces = FaceDetect(matImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
	if (!pnFaces)
		return;

	MarkFaceWithRectangle(matImg, pnFaces);

	imshow("Face Detect Result", matImg);
}

//* ��ʼ��������������DNN����
MACHINEVISIONLIB_API Net cv2shell::InitFaceDetectDNNet(void)
{
	String strModelCfgFile = "C:\\Windows\\System32\\models\\resnet\\deploy.prototxt";
	String strModelFile = "C:\\Windows\\System32\\models\\resnet\\res10_300x300_ssd_iter_140000.caffemodel";

	dnn::Net dnnNet = readNetFromCaffe(strModelCfgFile, strModelFile);
	if (dnnNet.empty())
		cout << "DNN���罨��ʧ�ܣ���ȷ����������������ļ���deploy.prototxt����ģ���ļ���resnet_ssd.caffemodel�������Ҹ�ʽ��ȷ" << endl;

	return dnnNet;
}

//* ʹ��DNN������ѵ���õ�ģ�Ϳ�����������⺯��
MACHINEVISIONLIB_API Mat cv2shell::FaceDetect(Net &dnnNet, Mat &matImg, const Size &size, FLOAT flScale, const Scalar &mean)
{
	//* ����ͼƬ�ļ�����һ����size����ָ��ͼƬҪ���ŵ�Ŀ��ߴ磬menaָ��Ҫ��ȥ��ƽ��ֵ��ƽ��������������������ɫͨ����Ҫ����
	Mat matInputBlob = blobFromImage(matImg, flScale, size, mean, false, false);

	//* ������������
	dnnNet.setInput(matInputBlob, "data");

	//* �����������
	Mat matDetection = dnnNet.forward("detection_out");

	Mat matFaces(matDetection.size[2], matDetection.size[3], CV_32F, matDetection.ptr<float>());

	return matFaces;
}

//* ��ͼƬ�ȱ�����С��ָ������
static Size __ResizeImgToSpecPixel(Mat &matImg, INT nMinPixel)
{
	INT *pnMin, *pnMax, nRows, nCols;

	//* �Ѿ�С��ָ������Сֵ�ˣ��Ǿ�û��Ҫ��������
	if (matImg.rows <= nMinPixel || matImg.cols <= nMinPixel)
		return Size(matImg.cols, matImg.rows);

	//* �����ֱ�ӷ���ָ������Сֵ�Ϳ�����
	if (matImg.rows == matImg.cols)
		return Size(nMinPixel, nMinPixel);

	nRows = matImg.rows;
	nCols = matImg.cols;

	pnMin = &nCols;
	pnMax = &nRows;

	if (matImg.rows < matImg.cols)
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
MACHINEVISIONLIB_API Mat cv2shell::FaceDetect(Net &dnnNet, Mat &matImg, ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar &mean)
{
	if (enumMethod == EIRSZM_EQUALRATIO)
	{
		Size size = __ResizeImgToSpecPixel(matImg, 288); //* ʵ��288Ч�����

		return FaceDetect(dnnNet, matImg, size, flScale, mean);
	}
	else
	{
		Mat matEquilateralImg;

		ImgEquilateral(matImg, matEquilateralImg);
		//namedWindow("Equilateral", 0);
		//imshow("Equilateral", matEquilateralImg);	
		//imwrite("D:\\work\\OpenCV\\resize_test.jpg", matEquilateralImg);

		return FaceDetect(dnnNet, matEquilateralImg, Size(300, 300), flScale, mean);
	}
}

MACHINEVISIONLIB_API Mat cv2shell::FaceDetect(Net &dnnNet, const CHAR *pszImgName, ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		Mat mat;

		return mat;
	}

	return FaceDetect(dnnNet, matImg, enumMethod, flScale, mean);
}

MACHINEVISIONLIB_API Mat cv2shell::FaceDetect(Net &dnnNet, const CHAR *pszImgName, const Size &size, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		Mat mat;

		return mat;
	}

	return FaceDetect(dnnNet, matImg, size, flScale, mean);
}

//* ����vFaces���ڱ����⵽��������Ϣ����һ���������
MACHINEVISIONLIB_API void cv2shell::FaceDetect(Net &dnnNet, Mat &matImg, vector<Face> &vFaces, const Size &size,
	FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar &mean)
{
	//* ����ͼƬ�ļ�����һ����size����ָ��ͼƬҪ���ŵ�Ŀ��ߴ磬menaָ��Ҫ��ȥ��ƽ��ֵ��ƽ��������������������ɫͨ����Ҫ����
	Mat matInputBlob = blobFromImage(matImg, flScale, size, mean, false, false);

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

		Face face;

		face.xLeftBottom = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * matImg.cols);
		face.yLeftBottom = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * matImg.rows);
		face.xRightTop = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * matImg.cols);
		face.yRightTop = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * matImg.rows);

		face.flConfidenceVal = flConfidenceVal;
		vFaces.push_back(face);
	}
}

MACHINEVISIONLIB_API void cv2shell::FaceDetect(Net &dnnNet, Mat &matImg, vector<Face> &vFaces, FLOAT flConfidenceThreshold,
												ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar &mean)
{
	if (enumMethod == EIRSZM_EQUALRATIO)
	{
		Size size = __ResizeImgToSpecPixel(matImg, 288); //* ʵ��288Ч�����

		FaceDetect(dnnNet, matImg, vFaces, size, flConfidenceThreshold, flScale, mean);
	}
	else
	{
		Mat matEquilateralImg;

		ImgEquilateral(matImg, matEquilateralImg);
		//namedWindow("Equilateral", 0);
		//imshow("Equilateral", matEquilateralImg);	
		//imwrite("D:\\work\\OpenCV\\resize_test.jpg", matEquilateralImg);

		FaceDetect(dnnNet, matEquilateralImg, vFaces, Size(300, 300), flConfidenceThreshold, flScale, mean);
	}
}

MACHINEVISIONLIB_API void cv2shell::FaceDetect(Net &dnnNet, const CHAR *pszImgName, vector<Face> &vFaces,
												FLOAT flConfidenceThreshold, ENUM_IMGRESIZE_METHOD enumMethod, 
												FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	FaceDetect(dnnNet, matImg, vFaces, flConfidenceThreshold, enumMethod, flScale, mean);
}

MACHINEVISIONLIB_API void cv2shell::FaceDetect(Net &dnnNet, const CHAR *pszImgName, vector<Face> &vFaces, const Size &size,
	FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::FaceDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	FaceDetect(dnnNet, matImg, vFaces, size, flConfidenceThreshold, flScale, mean);
}

//* ��������ͼƬ��չʾ�������þ��ο��ǳ����������Ԥ�����
//* ����flConfidenceThresholdָ����С���Ŷ���ֵ��Ҳ����Ԥ������������С����ֵ�����ڴ˸��ʵı����������������
MACHINEVISIONLIB_API void cv2shell::MarkFaceWithRectangle(Mat &matImg, Mat &matFaces, FLOAT flConfidenceThreshold)
{
	for (int i = 0; i < matFaces.rows; i++)
	{
		FLOAT flConfidenceVal = matFaces.at<FLOAT>(i, 2);
		if (flConfidenceVal < flConfidenceThreshold)
			continue;

		INT xLeftBottom = static_cast<INT>(matFaces.at<FLOAT>(i, 3) * matImg.cols);
		INT yLeftBottom = static_cast<INT>(matFaces.at<FLOAT>(i, 4) * matImg.rows);
		INT xRightTop = static_cast<INT>(matFaces.at<FLOAT>(i, 5) * matImg.cols);
		INT yRightTop = static_cast<INT>(matFaces.at<FLOAT>(i, 6) * matImg.rows);

		//* ��������
		Rect rectObj(xLeftBottom, yLeftBottom, (xRightTop - xLeftBottom), (yRightTop - yLeftBottom));
		rectangle(matImg, rectObj, Scalar(0, 255, 0));

		//* �ڱ����ͼƬ��������Ŷȸ���
		//* ======================================================================================
		ostringstream oss;
		oss << flConfidenceVal;
		String strConfidenceVal(oss.str());
		String strLabel = "Face: " + strConfidenceVal;

		INT nBaseLine = 0;
		Size labelSize = getTextSize(strLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
		rectangle(matImg, Rect(Point(xLeftBottom, yLeftBottom - labelSize.height),
			Size(labelSize.width, labelSize.height + nBaseLine)),
			Scalar(255, 255, 255), CV_FILLED);
		putText(matImg, strLabel, Point(xLeftBottom, yLeftBottom), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
		//* ======================================================================================
	}

	imshow("Face Detect Result", matImg);
}

//* �þ��ο����������������
MACHINEVISIONLIB_API void cv2shell::MarkFaceWithRectangle(Net &dnnNet, const CHAR *pszImgName, const Size &size, 
															FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::MarkFaceWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	Mat matFaces = FaceDetect(dnnNet, matImg, size, flScale, mean);
	if (matFaces.empty())
		return;

	MarkFaceWithRectangle(matImg, matFaces, flConfidenceThreshold);
}

//* �þ��ο����������������
MACHINEVISIONLIB_API void cv2shell::MarkFaceWithRectangle(Net &dnnNet, const CHAR *pszImgName, FLOAT flConfidenceThreshold,
	ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::MarkFaceWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	Mat &matFaces = FaceDetect(dnnNet, matImg, enumMethod, flScale, mean);
	if (matFaces.empty())
		return;

	MarkFaceWithRectangle(matImg, matFaces, flConfidenceThreshold);
}

MACHINEVISIONLIB_API void cv2shell::MarkFaceWithRectangle(Mat &matImg, vector<Face> &vFaces)
{
	vector<Face>::iterator itFace = vFaces.begin();
	for (; itFace != vFaces.end(); itFace++)
	{
		Face face = *itFace;

		//* ��������
		Rect rectObj(face.xLeftBottom, face.yLeftBottom, (face.xRightTop - face.xLeftBottom), (face.yRightTop - face.yLeftBottom));
		rectangle(matImg, rectObj, Scalar(0, 255, 0));

		//* �ڱ����ͼƬ��������Ŷȸ���
		//* ======================================================================================
		ostringstream oss;
		oss << face.flConfidenceVal;
		String strConfidenceVal(oss.str());
		String strLabel = "Face: " + strConfidenceVal;

		INT nBaseLine = 0;
		Size labelSize = getTextSize(strLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
		rectangle(matImg, Rect(Point(face.xLeftBottom, face.yLeftBottom - labelSize.height),
			Size(labelSize.width, labelSize.height + nBaseLine)),
			Scalar(255, 255, 255), CV_FILLED);
		putText(matImg, strLabel, Point(face.xLeftBottom, face.yLeftBottom), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
		//* ======================================================================================
	}

	namedWindow("Face Detect Result", 0);
	imshow("Face Detect Result", matImg);
}

MACHINEVISIONLIB_API Mat cv2shell::ExtractFaceChips(Mat matImg, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat matDummy;

	INT *pnFaces = NULL;
	UCHAR *pubResultBuf = (UCHAR*)malloc(LIBFACEDETECT_BUFFER_SIZE);
	if (!pubResultBuf)
	{
		cout << "error para in " << __FUNCTION__ << "(), in file " << __FILE__ << ", line " << __LINE__ - 3 << ", malloc error code:" << GetLastError() << endl;		
		return matDummy;
	}

	Mat matGray;
	cvtColor(matImg, matGray, CV_BGR2GRAY);

	//* ���������⣬�������Ҫ��DLib���������⺯����΢��һЩ
	INT nLandmark = 1;
	pnFaces = facedetect_multiview_reinforce(pubResultBuf, (UCHAR*)(matGray.ptr(0)), matGray.cols, matGray.rows, (INT)matGray.step,
												flScale, nMinNeighbors, nMinPossibleFaceSize, 0, nLandmark);	
	if (!pnFaces)
	{ 
		cout << "Error: No face was detected." << endl;
		return matDummy;
	}

	//* ����ͼƬֻ�������һ������
	if (*pnFaces != 1)
	{
		cout << "Error: Multiple faces were detected, and only one face was allowed." << endl;
		return matDummy;
	}

	//* ��ȡ68�����������㣬����������:0-35, �۾�:36-47,���Σ�48-60�����ߣ�61-67
	SHORT *psScalar = ((SHORT*)(pnFaces + 1));
	vector<dlib::point> vFaceFeature;
	for (INT j = 0; j < 68; j++)
		vFaceFeature.push_back(dlib::point((INT)psScalar[6 + 2 * j], (INT)psScalar[6 + 2 * j + 1]));

	//* Ϊ���ٶȣ������˲��ֿɶ��ԣ�
	//* psScalar��0��1��2��3�ֱ��������x��y��width��height����ˣ�
	//* rect()���ĸ������ֱ������Ͻ���������½�����
	dlib::rectangle rect(psScalar[0], psScalar[1], psScalar[0] + psScalar[2] - 1, psScalar[1] + psScalar[3] - 1);

	//* ��68��������תΪdlib����
	dlib::full_object_detection shape(rect, vFaceFeature);
	vector<dlib::full_object_detection> shapes;
	shapes.push_back(shape);

	dlib::array<dlib::array2d<dlib::rgb_pixel>> FaceChips;
	extract_image_chips(dlib::cv_image<uchar>(matGray), get_face_chip_details(shapes), FaceChips);
	Mat matFace = dlib::toMat(FaceChips[0]);

	//* ��������Ҫ�����Ϊ224,224��С
	resize(matFace, matFace, Size(224, 224));

	free(pubResultBuf);

	return matFace;
}

MACHINEVISIONLIB_API Mat cv2shell::ExtractFaceChips(const CHAR *pszImgName, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "ExtractFaceChips() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return matImg;
	}

	return ExtractFaceChips(matImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
}

//* ��ʼ�����ͷ���������ʵ���ǰ�DNN���������ļ���ѵ���õ�ģ�ͼ��ص��ڴ沢�ݴ˽���DNN����
MACHINEVISIONLIB_API Net cv2shell::InitLightClassifier(vector<string> &vClassNames)
{
	dnn::Net dnnNet;

	ifstream ifsClassNamesFile("C:\\Windows\\System32\\models\\vgg_ssd\\voc.names");
	if (ifsClassNamesFile.is_open())
	{
		string strClassName = "";
		while (getline(ifsClassNamesFile, strClassName))
			vClassNames.push_back(strClassName);
	}
	else
		return dnnNet;

	String strModelCfgFile("C:\\Windows\\System32\\models\\vgg_ssd\\deploy.prototxt"),
		strModelFile("C:\\Windows\\System32\\models\\vgg_ssd\\VGG_VOC0712_SSD_300x300_iter_120000.caffemodel");

	dnnNet = readNetFromCaffe(strModelCfgFile, strModelFile);
	if (dnnNet.empty())
	{
		cout << "DNN���罨��ʧ�ܣ���ȷ����������������ļ���deploy.prototxt����ģ���ļ���VGG_SSD.caffemodel�������Ҹ�ʽ��ȷ" << endl;
	}

	return dnnNet;
}

//* ʹ��DNN����ʶ��Ŀ��������������
MACHINEVISIONLIB_API void cv2shell::ObjectDetect(Mat &matImg, Net &dnnNet, vector<string> &vClassNames, vector<RecogCategory> &vObjects, 
													const Size &size, FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar &mean)
{
	if (matImg.channels() == 4)
		cvtColor(matImg, matImg, COLOR_BGRA2BGR);

	//* ����ͼƬ�ļ�����һ����size����ָ��ͼƬҪ���ŵ�Ŀ��ߴ磬menaָ��Ҫ��ȥ��ƽ��ֵ��ƽ��������������������ɫͨ����Ҫ����
	Mat matInputBlob = blobFromImage(matImg, flScale, size, mean, false, false);

	//* ������������
	dnnNet.setInput(matInputBlob, "data");

	//* �����������
	Mat matDetection = dnnNet.forward("detection_out");

	Mat matIdentifyObjects(matDetection.size[2], matDetection.size[3], CV_32F, matDetection.ptr<float>());

	for (int i = 0; i < matIdentifyObjects.rows; i++)
	{
		FLOAT flConfidenceVal = matIdentifyObjects.at<FLOAT>(i, 2);

		if (flConfidenceVal < flConfidenceThreshold)
			continue;

		RecogCategory category;

		category.xLeftBottom = static_cast<int>(matIdentifyObjects.at<float>(i, 3) * matImg.cols);
		category.yLeftBottom = static_cast<int>(matIdentifyObjects.at<float>(i, 4) * matImg.rows);
		category.xRightTop = static_cast<int>(matIdentifyObjects.at<float>(i, 5) * matImg.cols);
		category.yRightTop = static_cast<int>(matIdentifyObjects.at<float>(i, 6) * matImg.rows);

		category.flConfidenceVal = flConfidenceVal;

		size_t tObjClass = (size_t)matIdentifyObjects.at<float>(i, 1);
		category.strCategoryName = vClassNames[tObjClass];
		vObjects.push_back(category);
	}
}

MACHINEVISIONLIB_API void cv2shell::ObjectDetect(Mat &matImg, Net &dnnNet, vector<string> &vClassNames, vector<RecogCategory> &vObjects, 
													FLOAT flConfidenceThreshold, ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar &mean)
{
	if (enumMethod == EIRSZM_EQUALRATIO)
	{
		Size size = __ResizeImgToSpecPixel(matImg, 260);

		ObjectDetect(matImg, dnnNet, vClassNames, vObjects, size, flConfidenceThreshold, flScale, mean);
	}
	else
	{
		Mat matEquilateralImg;

		ImgEquilateral(matImg, matEquilateralImg);

		//namedWindow("Equilateral", 0);
		//imshow("Equilateral", matEquilateralImg);	
		//imwrite("D:\\work\\OpenCV\\resize_test.jpg", matEquilateralImg);

		ObjectDetect(matEquilateralImg, dnnNet, vClassNames, vObjects, Size(300, 300), flConfidenceThreshold, flScale, mean);
	}
}

MACHINEVISIONLIB_API void cv2shell::ObjectDetect(const CHAR *pszImgName, Net &dnnNet, vector<string> &vClassNames, vector<RecogCategory> &vObjects,
													const Size &size, FLOAT flConfidenceThreshold, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::ObjectDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	ObjectDetect(matImg, dnnNet, vClassNames, vObjects, size, flConfidenceThreshold, flScale, mean);
}

MACHINEVISIONLIB_API void cv2shell::ObjectDetect(const CHAR *pszImgName, Net &dnnNet, vector<string> &vClassNames, vector<RecogCategory> &vObjects,
													FLOAT flConfidenceThreshold, ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::ObjectDetect()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	ObjectDetect(matImg, dnnNet, vClassNames, vObjects, flConfidenceThreshold, enumMethod, flScale, mean);
}

//* ��������Ŀ�������ԭͼ���þ��ο��ǳ���
MACHINEVISIONLIB_API void cv2shell::MarkObjectWithRectangle(Mat &matImg, vector<RecogCategory> &vObjects)
{
	vector<RecogCategory>::iterator itObject = vObjects.begin();

	for (; itObject != vObjects.end(); itObject++)
	{
		RecogCategory object = *itObject;

		//* ��������
		Rect rectObj(object.xLeftBottom, object.yLeftBottom, (object.xRightTop - object.xLeftBottom), (object.yRightTop - object.yLeftBottom));
		rectangle(matImg, rectObj, Scalar(0, 255, 0));

		//* �ڱ����ͼƬ��������Ŷȸ���
		//* ======================================================================================
		ostringstream oss;
		oss << object.flConfidenceVal;
		String strConfidenceVal(oss.str());
		String strLabel = object.strCategoryName + ": " + strConfidenceVal;

		INT nBaseLine = 0;
		Size labelSize = getTextSize(strLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
		rectangle(matImg, Rect(Point(object.xLeftBottom, object.yLeftBottom - labelSize.height),
			Size(labelSize.width, labelSize.height + nBaseLine)),
			Scalar(255, 255, 255), CV_FILLED);
		putText(matImg, strLabel, Point(object.xLeftBottom, object.yLeftBottom), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0));
		//* ======================================================================================
	}

	//namedWindow("Object Detect Result", 0);	
	if (matImg.rows > 768 && matImg.rows == matImg.cols)
	{
		Mat matResizeImg;
		resize(matImg, matResizeImg, Size(720, 720), 0, 0, INTER_AREA);
		imshow("Object Detect Result", matResizeImg);
	}
	else
		imshow("Object Detect Result", matImg);
}

MACHINEVISIONLIB_API void cv2shell::MarkObjectWithRectangle(const CHAR *pszImgName, Net &dnnNet, vector<string> &vClassNames, 
															const Size &size, FLOAT flConfidenceThreshold, FLOAT flScale, 
															const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::MarkObjectWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	vector<RecogCategory> vObjects;
	ObjectDetect(matImg, dnnNet, vClassNames, vObjects, size, flConfidenceThreshold, flScale, mean);
	MarkObjectWithRectangle(matImg, vObjects);
}

MACHINEVISIONLIB_API void cv2shell::MarkObjectWithRectangle(const CHAR *pszImgName, Net &dnnNet, vector<string> &vClassNames, FLOAT flConfidenceThreshold,
															ENUM_IMGRESIZE_METHOD enumMethod, FLOAT flScale, const Scalar &mean)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "cv2shell::MarkObjectWithRectangle()�����޷�����ͼƬ����ȷ��ͼƬ��" << pszImgName << "�����ڻ��߸�ʽ��ȷ" << endl;

		return;
	}

	vector<RecogCategory> vObjects;
	ObjectDetect(matImg, dnnNet, vClassNames, vObjects, flConfidenceThreshold, enumMethod, flScale, mean);
	MarkObjectWithRectangle(matImg, vObjects);
}

//* ��ȡ��⵽��Ŀ�����������������⵽��Ŀ����������pflConfidenceOfExist������Ŷ�ֵ�������ڵĻ�pflConfidenceOfExist����һ��û�������ֵ
//* ����strObjectName�����InitLightClassifier()���������voc.names�ļ��л�ȡ����Ϊ���������ֻ�ܷ���voc.names�������ЩĿ������
MACHINEVISIONLIB_API INT cv2shell::GetObjectNum(vector<RecogCategory> &vObjects, string strObjectName, FLOAT *pflConfidenceOfExist, 
												FLOAT *pflConfidenceOfObjectNum)
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
									Mat& matImgROI, vector<DType>& vImgFeature, INT nFeatureDimension, const CHAR *pszBlobName)
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
#if NEED_GPU
	//* ʹ��GPUʱֱ�Ӵ�GPU��ȡ���ݿ�������GPU->CPU��ͬ�������������ܿ�һ��
	const DType *pFeatureData = blobImgFeature->gpu_data();
#else
	//* ʹ��GPUʱҲ����ʹ��cpu_data()����Ϊ����cpu_data()ʱ��caffe���Զ�ͬ��GPU->CPU������������Ҫ�ķ�һЩͬ��ʱ�䣬����ֻ�е���CPUʱ�ŵ���cpu_data()
	const DType *pFeatureData = blobImgFeature->gpu_data();

#endif

	//* ���������ݴ�����ڲ��������ϲ���ú���ʹ��
	for (INT i = 0; i < nFeatureDimension; i++)
		vImgFeature.push_back(pFeatureData[i]);
}
