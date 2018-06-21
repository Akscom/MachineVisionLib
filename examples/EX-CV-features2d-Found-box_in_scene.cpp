#include "stdafx.h"
#include <iostream>  
#include <fstream> 
#include <list>  
#include <vector>  
#include <map>  
#include <stack>  
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/objdetect.hpp>

using namespace std;
using namespace cv;

//* ������ƥ����ֱ�۵�˵��
//* https://blog.csdn.net/civiliziation/article/details/38370167
//* cv2��cv3�����ı�ģ�
//* https://www.cnblogs.com/anqiang1995/p/7398218.html


//* ��Ҫ���ܼ���˶�Ŀ��
//* https://www.jpjodoin.com/urbantracker/index.htm
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		return -1;
	}

	Mat img_object = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	//pyrDown(img_object, img_object, Size(img_object.cols / 2, img_object.rows / 2));
	Mat img_scene = imread(argv[2]);
	Mat img_dst = img_scene;
	cvtColor(img_scene, img_scene, COLOR_BGR2GRAY);
	//pyrDown(img_scene, img_scene, Size(img_scene.cols / 2, img_scene.rows / 2));

	if (!img_object.data || !img_scene.data)
	{
		std::cout << " --(!) Error reading images " << std::endl; return -1;
	}

	//-- Step 1: Detect the keypoints using SURF Detector and Calculate descriptors (feature vectors)
	int minHessian = 400;
	Ptr<FeatureDetector> detector = xfeatures2d::SURF::create(minHessian);

	vector<KeyPoint> keypoints_object, keypoints_scene;
	Mat descriptors_object, descriptors_scene;
	detector->detectAndCompute(img_object, noArray(), keypoints_object, descriptors_object);
	detector->detectAndCompute(img_scene, noArray(), keypoints_scene, descriptors_scene);

	//-- Step 2: Matching descriptor vectors using FLANN matcher
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
	vector< DMatch > matches;
	matcher->match(descriptors_object, descriptors_scene, matches);

	double max_dist = 0; double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_object.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	std::vector< DMatch > good_matches;

	for (int i = 0; i < descriptors_object.rows; i++)
	{
		if (matches[i].distance < 3 * min_dist)
		{
			good_matches.push_back(matches[i]);
		}
	}

	//Mat img_matches;
	//drawMatches(img_object, keypoints_object, img_scene, keypoints_scene,
	//	good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
	//	vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	//-- Localize the object
	vector<Point2f> obj;
	vector<Point2f> scene;

	for (int i = 0; i < good_matches.size(); i++)
	{
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
	}

	Mat H = findHomography(obj, scene, CV_RANSAC);

	//-- Get the corners from the image_1 ( the object to be "detected" )
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = cvPoint(0, 0); 
	obj_corners[1] = cvPoint(img_object.cols, 0);
	obj_corners[2] = cvPoint(img_object.cols, img_object.rows); 
	obj_corners[3] = cvPoint(0, img_object.rows);
	std::vector<Point2f> scene_corners(4);
	
	if (!H.empty())
	{
		perspectiveTransform(obj_corners, scene_corners, H);

		//-- Draw lines between the corners (the mapped object in the scene - image_2 )
		//line(img_matches, scene_corners[0] + Point2f(img_object.cols, 0), scene_corners[1] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
		//line(img_matches, scene_corners[1] + Point2f(img_object.cols, 0), scene_corners[2] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
		//line(img_matches, scene_corners[2] + Point2f(img_object.cols, 0), scene_corners[3] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);
		//line(img_matches, scene_corners[3] + Point2f(img_object.cols, 0), scene_corners[0] + Point2f(img_object.cols, 0), Scalar(0, 255, 0), 4);

		line(img_dst, scene_corners[0], scene_corners[1], Scalar(0, 255, 0), 4);
		line(img_dst, scene_corners[1], scene_corners[2], Scalar(0, 255, 0), 4);
		line(img_dst, scene_corners[2], scene_corners[3], Scalar(0, 255, 0), 4);
		line(img_dst, scene_corners[3], scene_corners[0], Scalar(0, 255, 0), 4);
	}

	//-- Show detected matches
	imshow("Object Image", img_object);
	imshow("Good Matches & Object detection", img_dst);

	cv::waitKey(0);
	destroyAllWindows();

	return 0;
}