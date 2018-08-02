#pragma once

#include "resource.h"

class Image {
public:
	Image():o_blIsEdited(FALSE){}
	~Image() {}

	//* ��ͼƬ
	BOOL open(CHAR *pszImgFileName) 
	{
		//* ����ͼ��		
		o_mOpenedImg = imread(pszImgFileName);
		if (o_mOpenedImg.empty())
			return FALSE;		
		
		o_mOpenedImg.copyTo(o_mResultImg);

		return TRUE;
	}

	//* ��ʾͼƬ
	void show(CHAR *pszWindowTitle)
	{
		if (o_mResultImg.empty())
			return;
		
		if (fabs(o_dblScaleFactor - 1.0) < 1e-6)
		{
			o_mResultImg.copyTo(o_mShowImg);
		}
		else
		{
			resize(o_mResultImg, o_mShowImg, Size(o_mResultImg.cols * o_dblScaleFactor, o_mResultImg.rows * o_dblScaleFactor), 0, 0, INTER_AREA);
			//cout << "o_mResultImg.cols * o_dblScaleFactor: " << o_mResultImg.cols * o_dblScaleFactor << " - o_mResultImg.rows * o_dblScaleFactor: " << o_mResultImg.rows * o_dblScaleFactor << endl;
			//pyrDown(o_mResultImg, mShowImg, Size(o_mResultImg.cols / 2, o_mResultImg.rows / 2));
		}			

		imshow(pszWindowTitle, o_mShowImg);
	}

	Mat& GetSrcImg(void)
	{
		return o_mOpenedImg;
	}

	Mat& GetResultImg(void)
	{
		return o_mResultImg;
	}

	Mat& GetShowImg(void)
	{
		return o_mShowImg;
	}

	INT GetImgWidth(void)
	{
		if (o_mResultImg.empty())
			return 0;

		return o_mResultImg.cols;
	}

	INT GetImgHeight(void)
	{
		if (o_mResultImg.empty())
			return 0;

		return o_mResultImg.rows;
	}

	//* ������������
	void SetScaleFactor(DOUBLE dblScaleFactor)
	{
		o_dblScaleFactor = dblScaleFactor;
	}

	DOUBLE GetScaleFactor(void) const
	{
		return o_dblScaleFactor;
	}

	void SetEditFlag(BOOL blFlag)
	{
		o_blIsEdited = blFlag;
	}

	BOOL GetEditFlag(void)
	{
		return o_blIsEdited;
	}

private:
	Mat o_mOpenedImg;			//* �����ԭʼͼƬ����
	Mat o_mResultImg;			//* �����Ľ������
	Mat o_mShowImg;				//* ���ڽ�����ʾ��Դͼ��

	DOUBLE o_dblScaleFactor;	//* ���ڳ���ֱ��ʵ�ͼ����Ҫ������С��PC��Ļ֧�ֵ�����Χ�ڲ�����ʾ���������������
								//* �ܲ����㣬�ò�����¼�����������

	BOOL o_blIsEdited;			//* �Ƿ�༭��
};
