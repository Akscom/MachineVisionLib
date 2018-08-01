#pragma once

class ImagePerspectiveTransformation {
public:
	ImagePerspectiveTransformation():nDragingCornerPointIdx(INVALID_INDEX){}
	~ImagePerspectiveTransformation(){}
	void process(Mat& mSrcImg, Mat& mResultImg, Mat& mShowImg, DOUBLE dblScaleFactor);

	vector<Point2f>& GetROI(void)
	{		
		return o_vptROI;
	}

private:
	vector<Point2f> o_vptROI;
	INT nDragingCornerPointIdx;	//* ����ק�Ľǵ����������������������o_vptROI�е�λ��
};
