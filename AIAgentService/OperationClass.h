#pragma once
#include <afx.h>
#include <string>
#include "..\dbproxy\ConnProxyIfc.h"
#include "..\dbproxy\DBInfo.h"

class OperationClass
{
public:
    void loadConfig();
	bool ConnectDB();
	void httpTasks();
	void closeDB();
    std::string GetExecutablePath();
    std::string GetExecutableDirectory();
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
	BYTE* convertToByte(std::string content, int& len);
};

