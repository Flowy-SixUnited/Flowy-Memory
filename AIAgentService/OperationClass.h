#pragma once
#include <string>
#include <afx.h>
#include "..\dbproxy\ConnProxyIfc.h"
#include "..\dbproxy\DBInfo.h"
#include "include\json\value.h"
#include "include\json\reader.h"
#pragma comment(lib, "..\\x64\\Release\\dbproxy.lib")
#pragma comment(lib,"..\\x64\\Release\\jsoncpp.lib") 
class OperationClass
{
public:
	char* Utf8ToStringT(LPSTR str);
	char* URLDecode(const char* _url);
	std::string GetExecutablePath();
	std::string GetExecutableDirectory();
	//����PostgreSQL���ݿ�
	void loadConfig();
	//static CDeepseekComminuteApp deepseekCominute;
	//��ʼ�����ݿ�
	bool ConnectDB();
	//�ر����ݿ�����
	void closeDB();
	BYTE* convertToByte(std::string content, int& len);
	void httpTasks();
	// ��д
public:
	char dbType;		//���ݿ����� 0:access 1:mssql 2:mysql 3:EXCEL 4:PSQL
	CString serverName;	//���ݿ������IP��������
	CString username;	//���ݿ��û���
	CString password;	//���ݿ�����
	CString dbName;		//���ݿ�ʵ������
	double dbVer;		//���ݿ�汾 access�汾��ͬ�������ִ���ͬ
	char mssqlAuthMode;	//�������� ����mssql��0:windows�����֤ 1:mssql�����֤
	int port;			//���ݿ�˿ں�
	CString portStr;	//�˿ں��ַ��������ڴ洢ʱʹ�ã�����int�Ϳ��ַ���֮���ת������
	//20190630 ��������˳�ʱ���ݿ��Զ�������Ҫ�Ĳ���
	CString days;			//���ݿ�֧�ֱ��ݵ�����
	CString filepath;	//�Զ����ݵ�·��

	int index;//�������������У���¼�����±�
	//int equptID[MAXCOUNT_DOOR];

	CString iniPath;
	CString dataTime;
	CString key;//��̨����Կ

	CConnProxyIfc* pConn;
	//afx_msg void insertRecord(Json::Value jsonDataValue);
	CDBInfo* dBInfo;
	//BYTE* convertToByte(std::string content, int& len);
};

