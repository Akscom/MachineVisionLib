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

//* �������Ҽ������ƶȣ����ڴˣ�������������ʽ�μ���
//* https://blog.csdn.net/u012160689/article/details/15341303
//* C++�㷨ʵ�ֲμ���
//* https://blog.csdn.net/akadiao/article/details/79767113
MATHLGORITHM_API DOUBLE malib::CosineSimilarity(const FLOAT *pflaBaseData, const FLOAT *pflaTargetData, UINT unDimension)
{
	DOUBLE dblDotProduct = 0.0f;
	DOUBLE dblQuadraticSumOfBase = 0.0f;
	DOUBLE dblQuadraticSumOfTarget = 0.0f;

	for (UINT i = 0; i < unDimension; i++)
	{
		//* ���
		dblDotProduct += pflaBaseData[i] * pflaTargetData[i];

		dblQuadraticSumOfBase += (pflaBaseData[i] * pflaBaseData[i]);
		dblQuadraticSumOfTarget += (pflaTargetData[i] * pflaTargetData[i]);
	}

	return dblDotProduct / (sqrt(dblQuadraticSumOfBase) * sqrt(dblQuadraticSumOfTarget));
}