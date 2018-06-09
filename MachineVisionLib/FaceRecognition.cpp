// FaceRecognition.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <facedetect-dll.h>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MathAlgorithmLib.h"
#include "ImagePreprocess.h"
#include "FaceRecognition.h"

BOOL FaceDatabase::LoadCaffeVGGNet(string strCaffePrototxtFile, string strCaffeModelFile)
{
	pcaflNet = caffe2shell::LoadNet<FLOAT>(strCaffePrototxtFile, strCaffeModelFile, caffe::TEST);

	if (pcaflNet)
	{
		pflMemDataLayer = (caffe::MemoryDataLayer<FLOAT> *)pcaflNet->layers()[0].get();
		return TRUE;
	}
	else
		return FALSE;
}

//* //* ��һ������ͼƬ����ȡ��������ͼ���ú���ֻ����ȡһ������
static Mat __ExtractFaceFeatureChips(Mat& matGray, SHORT *psScalar, Mat& matFace)
{
	//* ��ȡ68�����������㣬����������:0-35, �۾�:36-47,���Σ�48-60�����ߣ�61-67
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
	matFace = dlib::toMat(FaceChips[0]);

	//* ����dlib�����matFace�����ǻҶ�ͼ����Ҫת�ɻҶ�ͼ��ſ�
	cvtColor(matFace, matFace, CV_BGR2GRAY);

	//* ��������Ҫ�����Ϊ224,224��С
	resize(matFace, matFace, Size(224, 224));

	return matFace;
}

//* ��һ������ͼƬ����ȡ��������ͼ
Mat FaceDatabase::ExtractFaceChips(Mat& matImg, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
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

	Mat matFace;
	__ExtractFaceFeatureChips(matGray, psScalar, matFace);
	
	free(pubResultBuf);

	return matFace;
}

//* ͬ��
Mat FaceDatabase::ExtractFaceChips(const CHAR *pszImgName, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "ExtractFaceChips() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return matImg;
	}

	return ExtractFaceChips(matImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
}

//* ��Թ��ա��ɼ��豸�豸�仯�����⣬�����������ʹ���˽��gammaУ������ָ�˹�˲�(DoG)���ԱȶȾ��⻯���ּ����Ĺ���Ԥ�����㷨�����ۼ���ʽ�μ���
//* https://blog.csdn.net/wuzuyu365/article/details/51898714
Mat FaceDatabase::FaceChipsHandle(Mat& matFaceChips, DOUBLE dblPowerValue, DOUBLE dblGamma, DOUBLE dblNorm)
{
	Mat matDstChips;

	//* ��ͨ�˲��ˣ��ԱȲ��Խ����Ŀǰ�������ã�
	//* 0.85-0.86 0.88-0.87 0.88-0.85 0.78-0.75
	FLOAT flaHighFilterKernel[9] = { -1, -1, -1, -1, 9, -1, -1, -1, -1 };
	imgpreproc::ContrastEqualizationWithFilter(matFaceChips, matDstChips, Size(3, 3), flaHighFilterKernel, dblGamma, dblPowerValue, dblNorm);

	//* ����caffe��ȡ����֮ǰ������ΪRGB��ʽ�ſɣ�����caffe�ᱨ��
	cvtColor(matDstChips, matDstChips, CV_GRAY2RGB);

	//imshow("FaceChipsHandle", matFloat);
	//waitKey(600);

	//* ��֤���룬�����߼�����Ҫ
	//FileStorage fs("mat.xml", FileStorage::WRITE);
	//fs << "MAT-DATA" << matFloat;	
	//fs.release();

	return matDstChips;
}

//* ���������Ƿ��Ѿ���ӵ����ݿ��У������ظ����
BOOL FaceDatabase::IsPersonAdded(const string& strPersonName)
{
	HANDLE hDir = CLIBOpenDirectory(FACE_DB_PATH);
	if (hDir == INVALID_HANDLE_VALUE)
	{
		cout << "Unabled to open the folder " << FACE_DB_PATH << ", the process will be exit." << endl;
		exit(-1);
	}

	UINT unNameLen;
	string strFileName, strPersonFileName = strPersonName + ".xml";
	while ((unNameLen = CLIBReadDir(hDir, strFileName)) > 0)
	{		
		if (strFileName == strPersonFileName)
		{
			CLIBCloseDirectory(hDir);
			return TRUE;
		}
	}

	CLIBCloseDirectory(hDir);

	return FALSE;
}

//* ����������ͳ���ļ�
void FaceDatabase::UpdateFaceDBStatisticFile(const string& strPersonName)
{
	ST_FACE_DB_STATIS_INFO stInfo;

	//* �ȶ���
	GetFaceDBStatisInfo(&stInfo);

	//* ������ֵ������stInfo.dwTotalLenOfPersonNameԤ����"\x00"�����Է����ȡ
	stInfo.nPersonNum += 1;
	stInfo.nTotalLenOfPersonName += strPersonName.size() + 1;

	//* д��
	FileStorage fs(FACE_DB_STATIS_FILE, FileStorage::WRITE);
	fs << FDBSTATIS_LABEL_PERSON_NUM << stInfo.nPersonNum;
	fs << FDBSTATIS_LABEL_PERSONNAME_TOTAL_LEN << stInfo.nTotalLenOfPersonName;

	fs.release();
}

void FaceDatabase::GetFaceDBStatisInfo(PST_FACE_DB_STATIS_INFO pstInfo)
{
	FileStorage fs;
	if (fs.open(FACE_DB_STATIS_FILE, FileStorage::READ))
	{
		fs[FDBSTATIS_LABEL_PERSON_NUM] >> pstInfo->nPersonNum;
		fs[FDBSTATIS_LABEL_PERSONNAME_TOTAL_LEN] >> pstInfo->nTotalLenOfPersonName;

		fs.release();
	}
	else
	{		
		cout << "The statistic file ��" << FACE_DB_STATIS_FILE << "�� of the face database does not exist, and has not added face data yet?" << endl;

		pstInfo->nPersonNum = 0;
		pstInfo->nTotalLenOfPersonName = 0;
	}
}

BOOL FaceDatabase::AddFace(Mat& matImg, const string& strPersonName)
{
	if (matImg.empty())
	{
		cout << "error para in " << __FUNCTION__ << "(), in file " << __FILE__ << ", line " << __LINE__ - 3 << ", the parameter matImg is empty." << GetLastError() << endl;
		return FALSE;
	}

	if (IsPersonAdded(strPersonName))
	{
		cout << strPersonName  << " has been added to the face database." << endl;
		return TRUE;
	}

	//* ��ͼƬ����ȡ��������
	Mat matFaceChips = ExtractFaceChips(matImg);
	if (matFaceChips.empty())
	{
		cout << "No face was detected." << endl;
		return FALSE;
	}

	//imshow("pic", matFaceChips);
	//cv::waitKey(60);

	//* ROI(region of interest)������һ���㷨����������ת��Ϊcaffe����������ȡ��Ҫ����������
	Mat matFaceROI = FaceChipsHandle(matFaceChips);

	//* ͨ��caffe������ȡͼ������
	Mat matImgFeature(1, FACE_FEATURE_DIMENSION, CV_32F);
	caffe2shell::ExtractFeature(pcaflNet, pflMemDataLayer, matFaceROI, matImgFeature, FACE_FEATURE_DIMENSION, FACE_FEATURE_LAYER_NAME);
	
	//* ���������ݴ����ļ�
	string strXMLFile = string(FACE_DB_PATH) + "/" + strPersonName + ".xml";
	FileStorage fs(strXMLFile, FileStorage::WRITE);
	fs << "VGG-FACEFEATURE" << matImgFeature;
	fs.release();

	//* ���������������ֽ����������������ݿ����
	UpdateFaceDBStatisticFile(strPersonName);

	return TRUE;
}

BOOL FaceDatabase::AddFace(const CHAR *pszImgName, const string& strPersonName)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "AddFace() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return FALSE;
	}

	return AddFace(matImg, strPersonName);
}

//* ���������ݴ��ļ���ȡ��������ָ���ڴ�
static BOOL __PutFaceDataFromFile(const string strFaceDataFile, FLOAT *pflFaceData)
{
	Mat matFaceFeatureData;

	FileStorage fs;
	if (fs.open(strFaceDataFile, FileStorage::READ))
	{
		fs["VGG-FACEFEATURE"] >> matFaceFeatureData;

		fs.release();

		for (INT i=0; i < FACE_FEATURE_DIMENSION; i++)
			pflFaceData[i] = matFaceFeatureData.at<FLOAT>(0, i);

		return TRUE;
	}
	else
	{
		cout << "File ��" << strFaceDataFile << "�� not exist." << endl;

		return FALSE;
	}	
}

//* ����������д���ڴ��ļ�
void FaceDatabase::PutFaceToMemFile(void)
{
	HANDLE hDir = CLIBOpenDirectory(FACE_DB_PATH);
	if (hDir == INVALID_HANDLE_VALUE)
	{
		cout << "Unabled to open the folder " << FACE_DB_PATH << ", the process will be exit." << endl;
		exit(-1);
	}

	UINT unNameLen, unWriteBytes;
	string strFileName;
	unWriteBytes = 0;
	nActualNumOfPerson = 0;
	while ((unNameLen = CLIBReadDir(hDir, strFileName)) > 0)
	{
		//* д����������
		string strFaceDataFile = string(FACE_DB_PATH) + "/" + strFileName;
		if (!__PutFaceDataFromFile(strFaceDataFile, ((FLOAT *)stMemFileFaceData.pvMem) + nActualNumOfPerson * FACE_FEATURE_DIMENSION))
			continue;		
		nActualNumOfPerson++;

		//* д���ļ���
		size_t sztEndPos = strFileName.rfind(".xml");		
		string strPersonName = strFileName.substr(0, sztEndPos);
		sprintf(((CHAR*)stMemFilePersonName.pvMem) + unWriteBytes, "%s", strPersonName.c_str());
		unWriteBytes += strPersonName.size() + 1;		
	}

	CLIBCloseDirectory(hDir);
}

//* �����ݿ��м����������ݵ��ڴ�
BOOL FaceDatabase::LoadFaceData(void)
{
	ST_FACE_DB_STATIS_INFO stInfo;

	//* ����ͳ����Ϣ
	GetFaceDBStatisInfo(&stInfo);
	if (!stInfo.nPersonNum)
	{
		cout << "No face data has been added, the process will be exit." << endl;
		exit(-1);
	}

	//* Ϊ�������ݽ����ڴ��ļ�
	if(!common_lib::CreateMemFile(&stMemFileFaceData, stInfo.nPersonNum * FACE_FEATURE_DIMENSION * sizeof(FLOAT)))
	{ 
		return FALSE;
	}

	//* Ϊ���������ڴ��ļ�
	if (!common_lib::CreateMemFile(&stMemFilePersonName, stInfo.nTotalLenOfPersonName))
	{
		common_lib::DeletMemFile(&stMemFileFaceData);

		return FALSE;
	}
	memset((CHAR*)stMemFilePersonName.pvMem, 0, stInfo.nTotalLenOfPersonName);

	//* ���������ݷ����ڴ��ļ�
	PutFaceToMemFile();

	//* ��֤���룬�����߼�����Ҫ
	//const CHAR *pszPerson = (const CHAR *)stMemFilePersonName.pvMem;
	//const FLOAT *pflaData = (FLOAT*)stMemFileFaceData.pvMem;
	//for (INT i = 0; i < nActualNumOfPerson; i++)
	//{
	//	cout << "��Name�� " << pszPerson << " ";
	//	pszPerson += strlen(pszPerson) + 1;

	//	cout << pflaData[i * FACE_FEATURE_DIMENSION] << " " << pflaData[i * FACE_FEATURE_DIMENSION + 1]
	//		<< " " << pflaData[i * FACE_FEATURE_DIMENSION + 2] << " " << pflaData[i * FACE_FEATURE_DIMENSION + 3] << endl;
	//}
	
	return TRUE;
}

//* ���ʵ�ʵ�Ԥ��
static DOUBLE __Predict(FaceDatabase *pface_db, Mat& matFaceChips, string& strPersonName, 
							FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	//* ROI(region of interest)������һ���㷨(����gammaУ�����˲�����һ���ȴ���)����������ת��Ϊcaffe����������ȡ��Ҫ����������
	Mat matFaceROI = pface_db->FaceChipsHandle(matFaceChips);

	//* ͨ��caffe������ȡͼ������
	FLOAT flaFaceFeature[FACE_FEATURE_DIMENSION];
	caffe2shell::ExtractFeature(pface_db->pcaflNet, pface_db->pflMemDataLayer, matFaceROI, flaFaceFeature, FACE_FEATURE_DIMENSION, FACE_FEATURE_LAYER_NAME);

	//* ����ƥ�����������
	const CHAR *pszPerson = (const CHAR *)pface_db->stMemFilePersonName.pvMem;
	const FLOAT *pflaData = (FLOAT*)pface_db->stMemFileFaceData.pvMem;
	DOUBLE dblMaxSimilarity = flConfidenceThreshold, dblSimilarity;
	const CHAR *pszMatchPersonName = NULL;
	for (INT i = 0; i < pface_db->nActualNumOfPerson; i++)
	{
		dblSimilarity = malib::CosineSimilarity(pflaData + i * FACE_FEATURE_DIMENSION, flaFaceFeature, FACE_FEATURE_DIMENSION);
		if (dblSimilarity > dblMaxSimilarity)
		{
			//* ����ֹͣ���ҵ���ֵ�������Ϳ���ȷ����������ˣ����Բ��ٲ�����
			if (dblSimilarity > flStopPredictThreshold)
			{
				strPersonName = pszPerson;

				return dblSimilarity;
			}

			dblMaxSimilarity = dblSimilarity;
			pszMatchPersonName = pszPerson;
		}

		//cout << pszPerson << ":" << dblSimilarity << endl;

		//* ��һ��
		pszPerson += strlen(pszPerson) + 1;
	}

	//* �������û����̵���С���ƶ�ֵ���������Ϊ�ҵ���
	if (dblMaxSimilarity > flConfidenceThreshold)
		strPersonName = pszMatchPersonName;

	return dblMaxSimilarity;
}

//* �������ƶȣ�����strPersonName�����ҵ���������flConfidenceThresholdָ����Ϳ��Ŷ�ֵ���������ֵ����Ϊ���������
//* flStopPredictThresholdָ��ֹͣ���ҵ���ֵ����Ϊ����������������ݿ����ҵ�������ƶȵ��������������ֵ����ֻҪ�ҵ��������
//* ֵ��������ֹͣ�����ˣ���������Ч����Ч��
DOUBLE FaceDatabase::Predict(Mat& matImg, string& strPersonName, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	if (matImg.empty())
	{
		cout << "error para in " << __FUNCTION__ << "(), in file " << __FILE__ << ", line " << __LINE__ - 3 << ", the parameter matImg is empty." << GetLastError() << endl;
		return 0;
	}

	//* ��ͼƬ����ȡ��������
	Mat matFaceChips = ExtractFaceChips(matImg);
	if (matFaceChips.empty())
	{
		cout << "No face was detected." << endl;
		return 0;
	}

	//imshow("pic", matFaceChips);
	//cv::waitKey(60);

	return __Predict(this, matFaceChips, strPersonName, flConfidenceThreshold, flStopPredictThreshold);
}

DOUBLE FaceDatabase::Predict(const CHAR *pszImgName, string& strPersonName, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "Predict() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return 0;
	}

	return Predict(matImg, strPersonName, flConfidenceThreshold);
}

//* ��ƵԤ��ģ��
vector<ST_PERSON> FaceDatabase::VideoPredict::Predict(Mat& matVideoImg, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	vector<ST_PERSON> vPersons;

	Mat matGray;
	cvtColor(matVideoImg, matGray, CV_BGR2GRAY);

	//* ���������⣬�������Ҫ��DLib���������⺯����΢��һЩ
	INT nLandmark = 1;
	INT *pnFaces = facedetect_multiview_reinforce(pubFeaceDetectResultBuf, (UCHAR*)(matGray.ptr(0)), matGray.cols, matGray.rows, (INT)matGray.step,
													flScale, nMinNeighbors, nMinPossibleFaceSize, 0, nLandmark);
	if (!pnFaces)
		return vPersons;

	for (INT i = 0; i < *pnFaces; i++)
	{
		SHORT *psScalar = ((SHORT*)(pnFaces + 1)) + LIBFACEDETECT_RESULT_STEP * i;		

		//* ����λ��
		INT x = psScalar[0];
		INT y = psScalar[1];
		INT nWidth = psScalar[2];
		INT nHeight = psScalar[3];		

		//* ���ο������
		Point left(x, y);
		Point right(x + nWidth, y + nHeight);
		rectangle(matVideoImg, left, right, Scalar(230, 255, 0), 1);

		Mat matFaceChips;
		__ExtractFaceFeatureChips(matGray, psScalar, matFaceChips);

		//* Ԥ�Ⲣ���
		INT nBaseLine = 0;
		String strPersonLabel;
		string strConfidenceLabel;
		Rect rect;		
		ST_PERSON stPerson;
		stPerson.dblConfidence = __Predict(pface_db, matFaceChips, stPerson.strPersonName, flConfidenceThreshold, flStopPredictThreshold);
		if (stPerson.dblConfidence > flConfidenceThreshold)
		{
			vPersons.push_back(stPerson);

			//* ��������Ϳ��Ŷ�
			strPersonLabel = "Name: " + stPerson.strPersonName;
			strConfidenceLabel = "Confidence: " + static_cast<ostringstream*>(&(ostringstream() << stPerson.dblConfidence))->str();
			
			Size personLabelSize = getTextSize(strPersonLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
			Size confidenceLabelSize = getTextSize(strConfidenceLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
			rect = Rect(Point(x, y - (personLabelSize.height + confidenceLabelSize.height + 3)), 
							Size(personLabelSize.width > confidenceLabelSize.width ? personLabelSize.width : confidenceLabelSize.width, 
								personLabelSize.height + confidenceLabelSize.height + nBaseLine + 3));
			rectangle(matVideoImg, rect, Scalar(255, 255, 255), CV_FILLED);
			putText(matVideoImg, strPersonLabel, Point(x, y - confidenceLabelSize.height - 3), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
			putText(matVideoImg, strConfidenceLabel, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
		}
		else
		{
			strPersonLabel = "No matching face was found";

			Size personLabelSize = getTextSize(strPersonLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
			rect = Rect(Point(x, y - personLabelSize.height), Size(personLabelSize.width, personLabelSize.height + nBaseLine));

			rectangle(matVideoImg, rect, Scalar(255, 255, 255), CV_FILLED);
			putText(matVideoImg, strPersonLabel, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
		}
	}

	return vPersons;
}

