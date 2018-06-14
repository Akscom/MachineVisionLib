// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� COMMON_LIB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// COMMON_LIB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef COMMON_LIB_EXPORTS
#define COMMON_LIB_API __declspec(dllexport)
#else
#define COMMON_LIB_API __declspec(dllimport)
#endif

#include <winsock2.h>
#include <mswsock.h>
#include <iostream>
#include <vector>

using namespace std;

//* �ڴ��ļ�
typedef struct _ST_MEM_FILE_ {	
	HANDLE hMem;
	void *pvMem;
} ST_MEM_FILE, *PST_MEM_FILE;

class COMMON_LIB_API PerformanceTimer {
public:
	PerformanceTimer() {
		QueryPerformanceFrequency(&uniOSFreq);
	}

	void start(void) {
		QueryPerformanceCounter(&uniStartCount);
	}

	//* ���ص�λΪ΢��
	DOUBLE end(void) {
		LARGE_INTEGER uniStopCount;

		QueryPerformanceCounter(&uniStopCount);

		return (1e6 / ((DOUBLE)uniOSFreq.QuadPart)) * (DOUBLE)(uniStopCount.QuadPart - uniStartCount.QuadPart);
	}

private:
	LARGE_INTEGER uniOSFreq;
	LARGE_INTEGER uniStartCount;
};

//* ����������
namespace common_lib {
	COMMON_LIB_API HANDLE CLIBOpenDirectory(const CHAR *pszDirectName);
	COMMON_LIB_API void CLIBCloseDirectory(HANDLE hDir);
	COMMON_LIB_API UINT CLIBReadDir(HANDLE hDir, string &strFileName);
	COMMON_LIB_API UINT GetFileNumber(const CHAR *pszDirectName);

	COMMON_LIB_API HANDLE IPCCreateSHM(CHAR *pszSHMemName, UINT unSize);
	COMMON_LIB_API CHAR *IPCCreateSHM(CHAR *pszSHMemName, UINT unSize, HANDLE *phSHM);
	COMMON_LIB_API CHAR *IPCOpenSHM(CHAR *pszSHMemName, HANDLE *phSHM);
	COMMON_LIB_API void IPCCloseSHM(CHAR *pszSHM);
	COMMON_LIB_API void IPCCloseSHM(CHAR *pszSHM, HANDLE hSHM);
	COMMON_LIB_API void IPCDelSHM(HANDLE hSHM);

	COMMON_LIB_API BOOL CreateMemFile(PST_MEM_FILE pstMemFile, DWORD dwFileSize);
	COMMON_LIB_API void DeletMemFile(PST_MEM_FILE pstMemFile);

	COMMON_LIB_API UINT IsProcExist(CHAR *pszProcName, INT nProcNameLen);
	COMMON_LIB_API BOOL IsProcExist(CHAR *pszProcName, ...);		

	COMMON_LIB_API UINT GetWorkPath(CHAR *pszPath, UINT unPathBytes);

	COMMON_LIB_API INT EatZeroOfTheNumberTail(INT nNum);
};