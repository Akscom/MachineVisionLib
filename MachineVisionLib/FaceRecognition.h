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

#define FACE_FEATURE_DIMENSION					2622						//* Ҫ��ȡ������ά��
#define FACE_FEATURE_LAYER_NAME					"fc8"						//* �����������
#define FACE_DB_PATH							"./PERSONS"					//* ���ݿⱣ��·��
#define FACE_DB_STATIS_FILE						"./FACEDB_STATISTIC.xml"	//* ������ͳ���ļ�
#define FDBSTATIS_LABEL_PERSON_NUM				"PERSON_NUM"				//* ������ͳ���ļ���ǩ֮����
#define FDBSTATIS_LABEL_PERSONNAME_TOTAL_LEN	"PERSONNAME_TOTAL_LENGTH"	//* ������ͳ���ļ���ǩ֮�����ܳ��ȣ�������\x00�౻ͳ������


#define FACE_DATA_FILE_NAME		"NEO-FACEFEATURE-DATA.DAT"	//* ���������������ݵ��ڴ�ʱ���ڴ��ļ�������
#define PERSON_NAME_FILE_NAME	"NEO-PERSON-NAME.TXT"		//* �����������ݵ��ڴ�ʱ���ڴ��ļ�������

//* �������ݿ�ͳ����Ϣ
typedef struct _ST_FACE_DB_STATIS_INFO_ {
	INT nPersonNum;				//* ����
	INT nTotalLenOfPersonName;	//* �����������ܳ���
} ST_FACE_DB_STATIS_INFO, *PST_FACE_DB_STATIS_INFO;

//* �������ݿ���
class FACERECOGNITION_API FaceDatabase {
public:
	FaceDatabase() {
#if NEED_GPU
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
#else
		caffe::Caffe::set_mode(caffe::Caffe::CPU);
#endif		

		stMemFileFaceData.pvMem = NULL;
		stMemFileFaceData.hMem = INVALID_HANDLE_VALUE;

		stMemFilePersonName.pvMem = NULL;
		stMemFilePersonName.hMem = INVALID_HANDLE_VALUE;		
	}

	~FaceDatabase() {	
		common_lib::DeletMemFile(&stMemFileFaceData);
		common_lib::DeletMemFile(&stMemFilePersonName);
	}

	BOOL LoadCaffeVGGNet(string strCaffePrototxtFile, string strCaffeModelFile);

	BOOL IsPersonAdded(const string& strPersonName);
	BOOL AddFace(const CHAR *pszImgName, const string& strPersonName);
	BOOL AddFace(Mat& matImg, const string& strPersonName);

	void GetFaceDBStatisInfo(PST_FACE_DB_STATIS_INFO pstInfo);
	BOOL LoadFaceData(void);

	DOUBLE Predict(Mat& matImg, string& strPersonName, FLOAT flConfidenceThreshold = 0.3, FLOAT flStopPredictThreshold = 0.95);
	DOUBLE Predict(const CHAR *pszImgName, string& strPersonName, FLOAT flConfidenceThreshold = 0.3, FLOAT flStopPredictThreshold = 0.95);

	caffe::Net<FLOAT> *pcaflNet;
	caffe::MemoryDataLayer<FLOAT> *pflMemDataLayer;

private:
	//* ��ȡ����ͼ��ע��ֻ����ȡһ������ͼ��
	Mat ExtractFaceChips(Mat matImg, FLOAT flScale = 1.05f, INT nMinNeighbors = 5, INT nMinPossibleFaceSize = 16);
	Mat ExtractFaceChips(const CHAR *pszImgName, FLOAT flScale = 1.05f, INT nMinNeighbors = 5, INT nMinPossibleFaceSize = 16);
	Mat FaceChipsHandle(Mat& matFaceChips, DOUBLE dblPowerValue = 0.1, DOUBLE dblGamma = 0.2, DOUBLE dblNorm = 10);	
	void UpdateFaceDBStatisticFile(const string& strPersonName);
	void PutFaceToMemFile(void);

	ST_MEM_FILE stMemFileFaceData;
	ST_MEM_FILE stMemFilePersonName;
	INT nActualNumOfPerson;
};
