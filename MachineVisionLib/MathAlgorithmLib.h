#pragma once
#ifdef MACHINEVISIONLIB_EXPORTS
#define MATHLGORITHM_API __declspec(dllexport)
#else
#define MATHLGORITHM_API __declspec(dllimport)
#endif

//* ��ѧ�㷨��
namespace malib {
	MATHLGORITHM_API DOUBLE CosineSimilarity(const FLOAT *pflaBaseData, const FLOAT *pflaTargetData, UINT unDimension);
};
