// FaceRecognition.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include <facedetect-dll.h>
#include "common_lib.h"
#include "MachineVisionLib.h"
#include "MathAlgorithmLib.h"
#include "ImagePreprocess.h"
#include "FaceRecognition.h"

BOOL FaceDatabase::LoadCaffeVGGNet(string strCaffePrototxtFile, string strCaffeModelFile)
{
	o_pcaflNet = caffe2shell::LoadNet<FLOAT>(strCaffePrototxtFile, strCaffeModelFile, caffe::TEST);

	if (o_pcaflNet)
	{
		o_pflMemDataLayer = (caffe::MemoryDataLayer<FLOAT> *)o_pcaflNet->layers()[0].get();
		return TRUE;
	}
	else
		return FALSE;
}

//* ��һ������ͼƬ����ȡ��������ͼ���ú���ֻ����ȡһ�����ģ�����mFaceFeatureChipsΪ���ڲ�����������ȡ����������
static void __ExtractFaceFeatureChips(Mat& mGray, SHORT *psScalar, Mat& mFaceFeatureChips)
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

	//dlib::array<dlib::array2d<dlib::rgb_pixel>> FaceChips;	//* rgb_pixelָ������RGB��ͨ�������ڴ�
	dlib::array<dlib::array2d<uchar>> FaceChips;
	extract_image_chips(dlib::cv_image<uchar>(mGray), get_face_chip_details(shapes), FaceChips);
	mFaceFeatureChips = dlib::toMat(FaceChips[0]);		

	//* ��������Ҫ�����Ϊ224,224��С
	resize(mFaceFeatureChips, mFaceFeatureChips, Size(224, 224), INTER_AREA);
	//imshow("dlib", mFaceFeatureChips);
	//waitKey(0);
}

//* ͬ�ϣ���һ������ͼƬ����ȡ��������ͼ���ú���ֻ����ȡһ�����ģ�����mFaceFeatureChipsΪ���ڲ�����������ȡ����������
static void __ExtractFaceFeatureChips(dlib::shape_predictor& objShapePredictor, Mat& mFaceGray, Face& objFace, Mat& mFaceFeatureChips)
{
	vector<dlib::full_object_detection> vobjShapes;	

	//* ��ȡ68��������
	dlib::rectangle objDLIBRect(objFace.o_nLeftTopX, objFace.o_nLeftTopY, objFace.o_nRightBottomX, objFace.o_nRightBottomY);
	vobjShapes.push_back(objShapePredictor(dlib::cv_image<uchar>(mFaceGray), objDLIBRect));
	if (vobjShapes.empty())
		return;	

	dlib::array<dlib::array2d<uchar>> objFaceChips;
	extract_image_chips(dlib::cv_image<uchar>(mFaceGray), get_face_chip_details(vobjShapes), objFaceChips);
	mFaceFeatureChips = dlib::toMat(objFaceChips[0]);

	
	//* ��������Ҫ�����Ϊ224,224��С
	resize(mFaceFeatureChips, mFaceFeatureChips, Size(224, 224), INTER_AREA);
}

//* ��һ������ͼƬ����ȡ��������ͼ
Mat FaceDatabase::ExtractFaceChips(Mat& mImg, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat mDummy;

	INT *pnFaces = NULL;
	UCHAR *pubResultBuf = (UCHAR*)malloc(LIBFACEDETECT_BUFFER_SIZE);
	if (!pubResultBuf)
	{
		cout << "error para in " << __FUNCTION__ << "(), in file " << __FILE__ << ", line " << __LINE__ - 3 << ", malloc error code:" << GetLastError() << endl;
		return mDummy;
	}

	Mat mGray;
	cvtColor(mImg, mGray, CV_BGR2GRAY);

	//* ���������⣬�������Ҫ��DLib���������⺯����΢��һЩ
	INT nLandmark = 1;
	pnFaces = facedetect_multiview_reinforce(pubResultBuf, (UCHAR*)(mGray.ptr(0)), mGray.cols, mGray.rows, (INT)mGray.step,
												flScale, nMinNeighbors, nMinPossibleFaceSize, 0, nLandmark);
	if (!pnFaces)
	{
		cout << "Error: No face was detected." << endl;
		return mDummy;
	}	

	//* ����ͼƬֻ�������һ������
	if (*pnFaces != 1)
	{
		cout << "Error: Multiple faces were detected, and only one face was allowed." << endl;
		return mDummy;
	}

	//* ��ȡ68�����������㣬����������:0-35, �۾�:36-47,���Σ�48-60�����ߣ�61-67
	SHORT *psScalar = ((SHORT*)(pnFaces + 1));

	Mat mFace;
	__ExtractFaceFeatureChips(mGray, psScalar, mFace);
	
	free(pubResultBuf);

	return mFace;
}

//* ͬ��
Mat FaceDatabase::ExtractFaceChips(const CHAR *pszImgName, FLOAT flScale, INT nMinNeighbors, INT nMinPossibleFaceSize)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "ExtractFaceChips() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return mImg;
	}

	return ExtractFaceChips(mImg, flScale, nMinNeighbors, nMinPossibleFaceSize);
}

//* ��Թ��ա��ɼ��豸�豸�仯�����⣬�����������ʹ���˽��gammaУ������ָ�˹�˲�(DoG)���ԱȶȾ��⻯���ּ����Ĺ���Ԥ�����㷨�����ۼ���ʽ�μ���
//* https://blog.csdn.net/wuzuyu365/article/details/51898714
Mat FaceDatabase::FaceChipsHandle(Mat& mFaceChips, DOUBLE dblPowerValue, DOUBLE dblGamma, DOUBLE dblNorm)
{
	Mat mDstChips;

	//* ��ͨ�˲��ˣ��ԱȲ��Խ����Ŀǰ�������ã�
	//* 0.85-0.86 0.88-0.87 0.88-0.85 0.78-0.75
	FLOAT flaHighFilterKernel[9] = { -1, -1, -1, -1, 9, -1, -1, -1, -1 };
	imgpreproc::ContrastEqualizationWithFilter(mFaceChips, mDstChips, Size(3, 3), flaHighFilterKernel, dblGamma, dblPowerValue, dblNorm);

	//* ����caffe��ȡ����֮ǰ������ΪRGB��ʽ�ſɣ�����caffe�ᱨ��
	cvtColor(mDstChips, mDstChips, CV_GRAY2RGB);

	//imshow("FaceChipsHandle", mDstChips);
	//waitKey(0);

	//* ��֤���룬�����߼�����Ҫ
	//FileStorage fs("mat.xml", FileStorage::WRITE);
	//fs << "MAT-DATA" << matFloat;	
	//fs.release();

	return mDstChips;
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

BOOL FaceDatabase::AddFace(Mat& mImg, const string& strPersonName)
{
	if (mImg.empty())
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
	Mat mFaceChips = ExtractFaceChips(mImg);
	if (mFaceChips.empty())
	{
		cout << "No face was detected." << endl;
		return FALSE;
	}

	//* ROI(region of interest)������һ���㷨����������ת��Ϊcaffe����������ȡ��Ҫ����������
	Mat mFaceROI = FaceChipsHandle(mFaceChips);

	//* ͨ��caffe������ȡͼ������
	Mat mImgFeature(1, FACE_FEATURE_DIMENSION, CV_32F);
	caffe2shell::ExtractFeature(o_pcaflNet, o_pflMemDataLayer, mFaceROI, mImgFeature, FACE_FEATURE_DIMENSION, FACE_FEATURE_LAYER_NAME);	
	
	//* ���������ݴ����ļ�
	string strXMLFile = string(FACE_DB_PATH) + "/" + strPersonName + ".xml";
	FileStorage fs(strXMLFile, FileStorage::WRITE);
	fs << "VGG-FACEFEATURE" << mImgFeature;
	fs.release();

	//* ���������������ֽ����������������ݿ����
	UpdateFaceDBStatisticFile(strPersonName);

	return TRUE;
}

BOOL FaceDatabase::AddFace(const CHAR *pszImgName, const string& strPersonName)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "AddFace() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return FALSE;
	}

	return AddFace(mImg, strPersonName);
}

//* �����������ͬ������������������ʹ��DLIB�ṩ��68��������ģ����ȡ��������
BOOL FaceDatabase::AddFace(Mat& mImgGray, Face& objFace, const string& strPersonName)
{
	if (mImgGray.empty())
	{
		cout << "error para in " << __FUNCTION__ << "(), in file " << __FILE__ << ", line " << __LINE__ - 3 << ", the parameter matImg is empty." << GetLastError() << endl;
		return FALSE;
	}

	if (IsPersonAdded(strPersonName))
	{
		cout << strPersonName << " has been added to the face database." << endl;
		return TRUE;
	}

	Mat mFaceChips;
	__ExtractFaceFeatureChips(o_objShapePredictor, mImgGray, objFace, mFaceChips);
	if (mFaceChips.empty())
	{
		cout << "No face was detected." << endl;
		return FALSE;
	}

	//* ROI(region of interest)������һ���㷨����������ת��Ϊcaffe����������ȡ��Ҫ����������
	Mat mFaceROI = FaceChipsHandle(mFaceChips);

	//* ͨ��caffe������ȡͼ������
	Mat mImgFeature(1, FACE_FEATURE_DIMENSION, CV_32F);
	caffe2shell::ExtractFeature(o_pcaflNet, o_pflMemDataLayer, mFaceROI, mImgFeature, FACE_FEATURE_DIMENSION, FACE_FEATURE_LAYER_NAME);

	//* ���������ݴ����ļ�
	string strXMLFile = string(FACE_DB_PATH) + "/" + strPersonName + ".xml";
	FileStorage fs(strXMLFile, FileStorage::WRITE);
	fs << "VGG-FACEFEATURE" << mImgFeature;
	fs.release();

	//* ���������������ֽ����������������ݿ����
	UpdateFaceDBStatisticFile(strPersonName);

	return TRUE;
}

//* ���������ݴ��ļ���ȡ��������ָ���ڴ�
static BOOL __PutFaceDataFromFile(const string strFaceDataFile, FLOAT *pflFaceData)
{
	Mat mFaceFeatureData;

	FileStorage fs;
	if (fs.open(strFaceDataFile, FileStorage::READ))
	{
		fs["VGG-FACEFEATURE"] >> mFaceFeatureData;

		fs.release();

		for (INT i=0; i < FACE_FEATURE_DIMENSION; i++)
			pflFaceData[i] = mFaceFeatureData.at<FLOAT>(0, i);

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
	o_nActualNumOfPerson = 0;
	while ((unNameLen = CLIBReadDir(hDir, strFileName)) > 0)
	{
		//* д����������
		string strFaceDataFile = string(FACE_DB_PATH) + "/" + strFileName;
		if (!__PutFaceDataFromFile(strFaceDataFile, ((FLOAT *)o_stMemFileFaceData.pvMem) + o_nActualNumOfPerson * FACE_FEATURE_DIMENSION))
			continue;		
		o_nActualNumOfPerson++;

		//* д���ļ���
		size_t sztEndPos = strFileName.rfind(".xml");		
		string strPersonName = strFileName.substr(0, sztEndPos);
		sprintf(((CHAR*)o_stMemFilePersonName.pvMem) + unWriteBytes, "%s", strPersonName.c_str());
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
	if(!common_lib::CreateMemFile(&o_stMemFileFaceData, stInfo.nPersonNum * FACE_FEATURE_DIMENSION * sizeof(FLOAT)))
	{ 
		return FALSE;
	}

	//* Ϊ���������ڴ��ļ�
	if (!common_lib::CreateMemFile(&o_stMemFilePersonName, stInfo.nTotalLenOfPersonName))
	{
		common_lib::DeletMemFile(&o_stMemFileFaceData);

		return FALSE;
	}
	memset((CHAR*)o_stMemFilePersonName.pvMem, 0, stInfo.nTotalLenOfPersonName);

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

BOOL FaceDatabase::LoadDLIB68FaceLandmarksModel(const CHAR *pszModelFileName)
{
	try {
		dlib::deserialize(pszModelFileName) >> o_objShapePredictor;
		o_blIsLoadDLIB68FaceLandmarksModel = TRUE;
		return TRUE;
	}
	catch (runtime_error& e) {		
		cerr << e.what() << endl;
		return FALSE;
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		return FALSE;
	}
}

//* ���ʵ�ʵ�Ԥ��
static DOUBLE __Predict(FaceDatabase *pobjFaceDB, Mat& matFaceChips, string& strPersonName, 
							FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{	
	//* ROI(region of interest)������һ���㷨(����gammaУ�����˲�����һ���ȴ���)����������ת��Ϊcaffe����������ȡ��Ҫ����������
	Mat matFaceROI = pobjFaceDB->FaceChipsHandle(matFaceChips);	
	
	//* ͨ��caffe������ȡͼ������
	FLOAT flaFaceFeature[FACE_FEATURE_DIMENSION];	
	caffe2shell::ExtractFeature(pobjFaceDB->o_pcaflNet, pobjFaceDB->o_pflMemDataLayer, matFaceROI, flaFaceFeature, FACE_FEATURE_DIMENSION, FACE_FEATURE_LAYER_NAME);		

	//* ����ƥ�����������	
	const CHAR *pszPerson = (const CHAR *)pobjFaceDB->o_stMemFilePersonName.pvMem;
	const FLOAT *pflaData = (FLOAT*)pobjFaceDB->o_stMemFileFaceData.pvMem;
	DOUBLE dblMaxSimilarity = flConfidenceThreshold, dblSimilarity;
	const CHAR *pszMatchPersonName = NULL;
	for (INT i = 0; i < pobjFaceDB->o_nActualNumOfPerson; i++)
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
DOUBLE FaceDatabase::Predict(Mat& mImg, string& strPersonName, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	if (mImg.empty())
	{
		cout << "error para in " << __FUNCTION__ << "(), in file " << __FILE__ << ", line " << __LINE__ - 3 << ", the parameter matImg is empty." << GetLastError() << endl;
		return 0;
	}

	//* ��ͼƬ����ȡ��������
	Mat mFaceChips = ExtractFaceChips(mImg);	
	if (mFaceChips.empty())
	{
		//cout << "No face was detected." << endl;
		return 0;
	}

	//imshow("pic", matFaceChips);
	//cv::waitKey(60);

	return __Predict(this, mFaceChips, strPersonName, flConfidenceThreshold, flStopPredictThreshold);
}

DOUBLE FaceDatabase::Predict(const CHAR *pszImgName, string& strPersonName, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	Mat mImg = imread(pszImgName);
	if (mImg.empty())
	{
		cout << "Predict() error��unable to read the picture, please confirm that the picture��" << pszImgName << "�� exists and the format is corrrect." << endl;

		return 0;
	}

	return Predict(mImg, strPersonName, flConfidenceThreshold);
}

//* ʹ��DLIB�ṩ68��������ģ����ȡ��������
DOUBLE FaceDatabase::Predict(Mat& mImgGray, Face& objFace, string& strPersonName, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	if (mImgGray.empty())
	{
		cout << "error para in " << __FUNCTION__ << "(), in file " << __FILE__ << ", line " << __LINE__ - 3 << ", the parameter matImgGray is empty." << GetLastError() << endl;
		return 0;
	}

	Mat mFaceChips;
	__ExtractFaceFeatureChips(o_objShapePredictor, mImgGray, objFace, mFaceChips);

	DOUBLE dblConfidence = 0;
	if (!mFaceChips.empty())
		dblConfidence = __Predict(this, mFaceChips, strPersonName, flConfidenceThreshold, flStopPredictThreshold);

	return dblConfidence;
}

//* ��ƵԤ��ģ��
vector<ST_PERSON> FaceDatabase::VideoPredict::Predict(Mat& mVideoImg, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	vector<ST_PERSON> vPersons;

	Mat mGray;
	cvtColor(mVideoImg, mGray, CV_BGR2GRAY);

	//* ���������⣬�������Ҫ��DLib���������⺯����΢��һЩ
	INT nLandmark = 1;
	INT *pnFaces = facedetect_multiview_reinforce(o_pubFeaceDetectResultBuf, (UCHAR*)(mGray.ptr(0)), mGray.cols, mGray.rows, (INT)mGray.step,
													o_flScale, o_nMinNeighbors, o_nMinPossibleFaceSize, 0, nLandmark);
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
		rectangle(mVideoImg, left, right, Scalar(0, 255, 0), 1);	
		
		Mat mFaceChips;
		__ExtractFaceFeatureChips(mGray, psScalar, mFaceChips);			

		//* Ԥ�Ⲣ���
		INT nBaseLine = 0;
		String strPersonLabel;
		string strConfidenceLabel;
		Rect rect;		
		ST_PERSON stPerson;		
		stPerson.dblConfidence = __Predict(o_pobjFaceDB, mFaceChips, stPerson.strPersonName, flConfidenceThreshold, flStopPredictThreshold);
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
			rectangle(mVideoImg, rect, Scalar(255, 255, 255), CV_FILLED);
			putText(mVideoImg, strPersonLabel, Point(x, y - confidenceLabelSize.height - 3), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
			putText(mVideoImg, strConfidenceLabel, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
		}
		else
		{
			strPersonLabel = "No matching face was found";

			Size personLabelSize = getTextSize(strPersonLabel, FONT_HERSHEY_SIMPLEX, 0.5, 1, &nBaseLine);
			rect = Rect(Point(x, y - personLabelSize.height), Size(personLabelSize.width, personLabelSize.height + nBaseLine));

			rectangle(mVideoImg, rect, Scalar(255, 255, 255), CV_FILLED);
			putText(mVideoImg, strPersonLabel, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(107, 194, 53));
		}
	}

	return vPersons;
}

//* ��ƵԤ��ģ�飬�ú�����Ҫ�ϲ���ú�������⵽���������ݴ��ݸ�������Ƹú�����Ŀ�������û�ѡ����ٵ�������⺯��������ͬ������ʹ�õļ�⺯���ܺ�ʱ
DOUBLE FaceDatabase::VideoPredict::Predict(Mat& mVideoImgGray, Face& objFace, dlib::shape_predictor& objShapePredictor,
											string& strPersonName, FLOAT flConfidenceThreshold, FLOAT flStopPredictThreshold)
{
	Mat mFaceChips;	
	__ExtractFaceFeatureChips(objShapePredictor, mVideoImgGray, objFace, mFaceChips);

	DOUBLE dblConfidence = 0;
	if (!mFaceChips.empty())
		dblConfidence = __Predict(o_pobjFaceDB, mFaceChips, strPersonName, flConfidenceThreshold, flStopPredictThreshold);

	return dblConfidence;
}

