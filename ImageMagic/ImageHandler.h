#pragma once

#define MOUSE_SHOT_OFFSET	10	//* ȷ��������ĳ������ʱ��������ƫ�ƣ���λ���أ�ֻҪС�����ֵ������Ϊ����Ѿ������������

class ImagePerspectiveTransformation {
public:
	ImagePerspectiveTransformation():o_nDragingCornerPointIdx(INVALID_INDEX), o_blIsNeedPaint(FALSE) {}
	~ImagePerspectiveTransformation(){}
	void process(Mat& mSrcImg, Mat& mResultImg, Mat& mSrcShowImg, DOUBLE dblScaleFactor);

	vector<Point2f>& GetROI(void)
	{		
		return o_vptROI;
	}

	void SetDragingCornerPointIndex(INT nDragingCornerPointIdx)
	{
		o_nDragingCornerPointIdx = nDragingCornerPointIdx;
	}

	INT GetDragingCornerPointIndex(void) const
	{
		return o_nDragingCornerPointIdx;
	}

	//* ���û��Ʊ�ǣ�����process()�������µĵ�����룬��Ҫ�ٴλ��ƽǵ�����
	void NeedToDrawCornerPoint(void)
	{
		o_blIsNeedPaint = TRUE;
	}

	DOUBLE GetScaleFactor(void)
	{
		return o_dblScaleFactor;
	}

private:
	vector<Point2f> o_vptROI;
	INT o_nDragingCornerPointIdx;	//* ����ק�Ľǵ����������������������o_vptROI�е�λ��
	BOOL o_blIsNeedPaint;
	DOUBLE o_dblScaleFactor;
};

//* �˵�������
typedef void(*PFUN_IMGHANDLER)(void);
typedef struct _ST_IMGHANDLER_ {
	INT nMenuID;
	PFUN_IMGHANDLER pfunHadler;
} ST_IMGHANDLER, *PST_IMGHANDLER;
