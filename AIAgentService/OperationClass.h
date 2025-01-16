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
	//新增PostgreSQL数据库
	void loadConfig();
	//static CDeepseekComminuteApp deepseekCominute;
	//初始化数据库
	bool ConnectDB();
	//关闭数据库连接
	void closeDB();
	BYTE* convertToByte(std::string content, int& len);
	void httpTasks();
	// 重写
public:
	char dbType;		//数据库类型 0:access 1:mssql 2:mysql 3:EXCEL 4:PSQL
	CString serverName;	//数据库服务器IP或主机名
	CString username;	//数据库用户名
	CString password;	//数据库密码
	CString dbName;		//数据库实例名称
	double dbVer;		//数据库版本 access版本不同，连接字串不同
	char mssqlAuthMode;	//连接类型 对于mssql，0:windows身份认证 1:mssql身份认证
	int port;			//数据库端口号
	CString portStr;	//端口号字符串，用于存储时使用，减少int和宽字符串之间的转换次数
	//20190630 增加软件退出时数据库自动备份需要的参数
	CString days;			//数据库支持备份的天数
	CString filepath;	//自动备份的路径

	int index;//如果存放在数组中，记录数组下标
	//int equptID[MAXCOUNT_DOOR];

	CString iniPath;
	CString dataTime;
	CString key;//后台的秘钥

	CConnProxyIfc* pConn;
	//afx_msg void insertRecord(Json::Value jsonDataValue);
	CDBInfo* dBInfo;
	//BYTE* convertToByte(std::string content, int& len);
};

