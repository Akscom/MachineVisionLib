#pragma once
#ifdef MACHINEVISIONLIB_EXPORTS
#define FACERECOGNITION_API __declspec(dllexport)
#else
#define FACERECOGNITION_API __declspec(dllimport)
#endif

#include "caffe/caffe.hpp"

namespace caffe
{
	extern INSTANTIATE_CLASS(InputLayer);
	extern INSTANTIATE_CLASS(InnerProductLayer);
	extern INSTANTIATE_CLASS(DropoutLayer);
	extern INSTANTIATE_CLASS(ConvolutionLayer);
	extern INSTANTIATE_CLASS(ReLULayer);
	extern INSTANTIATE_CLASS(PoolingLayer);
	extern INSTANTIATE_CLASS(LRNLayer);
	extern INSTANTIATE_CLASS(SoftmaxLayer);
	extern INSTANTIATE_CLASS(MemoryDataLayer);
}

#define FACE_FEATURE_DIMENSION	2622		//* Ҫ��ȡ������ά��
#define FACE_FEATURE_LAYER_NAME	"fc8"		//* �����������
#define FACE_DB_PATH			"./PERSONS"	//* ���ݿⱣ��·��

//* �������ݿ���
class FACERECOGNITION_API FaceDatabase {
public:
	FaceDatabase() {
#if NEED_GPU
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
#else
		caffe::Caffe::set_mode(caffe::Caffe::CPU);
#endif
		unFaceNum = 0;
	}
	~FaceDatabase() {		
	}

	BOOL LoadCaffeVGGNet(string strCaffePrototxtFile, string strCaffeModelFile);

	BOOL AddFace(const CHAR *pszImgName, const string& strPersonName);
	BOOL AddFace(Mat& matImg, const string& strPersonName);

	caffe::Net<FLOAT> *pcaflNet;
	caffe::MemoryDataLayer<FLOAT> *pflMemDataLayer;

private:
	//* ��ȡ����ͼ��ע��ֻ����ȡһ������ͼ��
	Mat ExtractFaceChips(Mat matImg, FLOAT flScale = 1.05f, INT nMinNeighbors = 5, INT nMinPossibleFaceSize = 16);
	Mat ExtractFaceChips(const CHAR *pszImgName, FLOAT flScale = 1.05f, INT nMinNeighbors = 5, INT nMinPossibleFaceSize = 16);
	Mat FaceChipsHandle(Mat& matFaceChips, DOUBLE dblPowerValue = 0.1, DOUBLE dblGamma = 0.2, DOUBLE dblNorm = 10);	
	BOOL IsPersonAdded(const string& strPersonName);

	UINT unFaceNum;	
};
