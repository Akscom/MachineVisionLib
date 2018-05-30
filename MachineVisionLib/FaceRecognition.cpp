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

Mat FaceDatabase::ExtractFaceChips(Mat matImg, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
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

	//* ����dlib�����matFace�����ǻҶ�ͼ����Ҫת�ɻҶ�ͼ��ſ�
	cvtColor(matFace, matFace, CV_BGR2GRAY);

	//* ��������Ҫ�����Ϊ224,224��С
	resize(matFace, matFace, Size(224, 224));

	free(pubResultBuf);

	return matFace;
}

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

//* ���ۼ���ʽ���μ���
//* https://blog.csdn.net/wuzuyu365/article/details/51898714
//* ����٤��У�����������ϣ�
//* https://blog.csdn.net/w450468524/article/details/51649651
//* �����˲���ģ�壺
//* http://blog.sina.com.cn/s/blog_6ac784290101e47s.html
Mat FaceDatabase::FaceChipsHandle(Mat& matFaceChips, DOUBLE dblPowerValue, DOUBLE dblGamma, DOUBLE dblNorm)
{
	FLOAT *pflData;

	//* ��������ת�ɸ�������
	Mat matFloat;
	matFaceChips.convertTo(matFloat, CV_32F);
		
	Mat matTransit(Size(matFloat.cols, matFloat.rows), CV_32F, Scalar(0));	//* �����õĹ��ɾ���
	
	//* ������gammaֵΪ0��Ҳ����ǿ��������Ӱ����ĶԱȶ�
	if (fabs(dblGamma) < 1e-6)
		dblGamma = 0.2;
	for (INT i = 0; i < matFloat.rows; i++)
	{
		for (INT j = 0; j < matFloat.cols; j++)
			matFloat.ptr<FLOAT>(i)[j] = pow(matFloat.ptr<FLOAT>(i)[j], dblGamma);		
	}
	
	//* ��ͨ�˲��ˣ��ԱȲ��Խ����Ŀǰ�������ã�
	//* 0.85-0.86 0.88-0.87 0.88-0.85 0.78-0.75
	FLOAT flaHighFilterKernel[9] = {-1, -1, -1, -1, 9, -1, -1, -1, -1}; 	
	Mat matFilterKernel = Mat(3, 3, CV_32FC1, flaHighFilterKernel);	
	filter2D(matFloat, matFloat, matFloat.depth(), matFilterKernel);

	//* ���Ϊ0.0��ֱ���������һ��
	DOUBLE dblTrim = fabs(dblNorm);
	if (dblTrim < 1e-6)
		goto __lblEnd;
	
	//* matFloat����ָ�����㲢�����ָ
	DOUBLE dblMeanVal = 0.0;
	matTransit = cv::abs(matFloat);
	for (INT i = 0; i<matFloat.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)
		{
			pflData[j] = pow(matTransit.ptr<FLOAT>(i)[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matFloat.rows * matFloat.cols);

	//* ��õľ�ֵָ���������������
	DOUBLE dblDivisor = cv::pow(dblMeanVal, 1/ dblPowerValue);
	for (INT i = 0; i < matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j<matFloat.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* ��ȥ�������ֵ��Ȼ����һ�����ֵ
	dblMeanVal = 0.0;
	matTransit = cv::abs(matFloat);
	for (INT i = 0; i<matFloat.rows; i++)
	{
		pflData = matTransit.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)
		{
			if (pflData[j] > dblTrim)
				pflData[j] = dblTrim;

			pflData[j] = pow(pflData[j], dblPowerValue);
			dblMeanVal += pflData[j];
		}
	}
	dblMeanVal /= (matFloat.rows * matFloat.cols);

	//* ��һ�ν���ֵ����ָ���������������
	dblDivisor = cv::pow(dblMeanVal, 1 / dblPowerValue);
	for (INT i = 0; i < matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j<matFloat.cols; j++)
			pflData[j] /= dblDivisor;
	}

	//* �������0.0
	if(dblNorm > 1e-6)
	{ 
		for (INT i = 0; i<matFloat.rows; i++)
		{
			pflData = matFloat.ptr<FLOAT>(i);
			for (INT j = 0; j<matFloat.cols; j++)
				pflData[j] = dblTrim * tanh(pflData[j] / dblTrim);
		}
	}

__lblEnd:
	//* �ҳ��������Сֵ
	FLOAT flMin = matFloat.ptr<FLOAT>(0)[0];
	for (INT i = 0; i <matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)
		{
			if (pflData[j] < flMin)
				flMin = pflData[j];
		}
	}

	//* �������Сֵ
	for (INT i = 0; i <matFloat.rows; i++)
	{
		pflData = matFloat.ptr<FLOAT>(i);
		for (INT j = 0; j < matFloat.cols; j++)	
				pflData[j] += flMin;
	}

	//* ��һ��
	normalize(matFloat, matFloat, 0, 255, NORM_MINMAX);
	matFloat.convertTo(matFloat, CV_8UC1);

	//* ����caffe��ȡ����֮ǰ������ΪRGB��ʽ�ſɣ�����caffe�ᱨ��
	cvtColor(matFloat, matFloat, CV_GRAY2RGB);

	imshow("FaceChipsHandle", matFloat);
	waitKey(600);

	//* ��֤���룬�����߼�����Ҫ
	//FileStorage fs("mat.xml", FileStorage::WRITE);
	//fs << "MAT-DATA" << matFloat;	
	//fs.release();

	return matFloat;
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

	//* ROI(region of interest)������һ���㷨����������ת��Ϊcaffe����������ȡ��Ҫ����������
	Mat matFaceROI = FaceChipsHandle(matFaceChips);

	//* ͨ��caffe������ȡͼ������
	FLOAT flaFaceFeature[FACE_FEATURE_DIMENSION];
	caffe2shell::ExtractFeature(pcaflNet, pflMemDataLayer, matFaceROI, flaFaceFeature, FACE_FEATURE_DIMENSION, FACE_FEATURE_LAYER_NAME);

	//* ����ƥ�����������
	const CHAR *pszPerson = (const CHAR *)stMemFilePersonName.pvMem;
	const FLOAT *pflaData = (FLOAT*)stMemFileFaceData.pvMem;
	DOUBLE dblMaxSimilarity = flConfidenceThreshold, dblSimilarity;
	const CHAR *pszMatchPersonName = NULL;
	for (INT i = 0; i < nActualNumOfPerson; i++)
	{		
		dblSimilarity = cv2shell::CosineSimilarity(pflaData + i * FACE_FEATURE_DIMENSION, flaFaceFeature, FACE_FEATURE_DIMENSION);
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

		cout << pszPerson << ":" << dblSimilarity << endl;
		
		//* ��һ��
		pszPerson += strlen(pszPerson) + 1;
	}

	//* �������û����̵���С���ƶ�ֵ���������Ϊ�ҵ���
	if (dblMaxSimilarity > flConfidenceThreshold)	
		strPersonName = pszMatchPersonName;

	return dblMaxSimilarity;
}

DOUBLE FaceDatabase::Predict(const CHAR *pszImgName, string& strPersonName, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	Mat matImg = imread(pszImgName);
	if (matImg.empty())
	{
		cout << "Predict() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return FALSE;
	}

	return Predict(matImg, strPersonName, flConfidenceThreshold);
}

