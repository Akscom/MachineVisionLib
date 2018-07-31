#pragma once

class ImagePerspectiveTransformation {
public:
	ImagePerspectiveTransformation():nDragingCornerPointIdx(INVALID_INDEX){}
	~ImagePerspectiveTransformation(){}
	void process(Mat& mSrcImg, Mat& mResutImg);

private:
	vector<Point2f> o_vptROI;
	INT nDragingCornerPointIdx;	//* ����ק�Ľǵ����������������������o_vptROI�е�λ��
};
