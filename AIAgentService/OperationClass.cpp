#include "OperationClass.h"
#include <thread>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/locale.hpp>
#include <iostream>
#include <string>
#include <iomanip>
#include <afx.h>
#include <fstream>
#include <codecvt>
#include "include/json/json.h"
#include "include/json/reader.h"
#include <locale>
#include <csignal>
#include <iostream>
#include <stdexcept>
#define TAB_SPACE "    "
#define NEW_LINE  "\r\n"
#pragma comment(lib,"..\\x64\\Debug\\jsoncpp.lib") 

namespace beast = boost::beast;     // Boost.Beast �����ռ�
namespace http = beast::http;      // HTTP Э��
namespace net = boost::asio;       // Boost.Asio �����ռ�
namespace ssl = net::ssl;          // SSL

using tcp = boost::asio::ip::tcp;  // ������


#define IsHexNum(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))

//�źŴ�����
void signalHandler(int signal) {
	if (signal == SIGSEGV) {
		throw std::runtime_error("Memory access violation (SIGSEGV) occurred!");
	}
}
std::string getLevelStr(int level)
{
	std::string levelStr = "";
	for (int i = 0; i < level; i++)
	{
		levelStr += TAB_SPACE;
	}
	return levelStr;
}
std::string formatJson(char* input) {
	std::string result = "";
	std::string inputData = input;
	int level = 0;

	//inputData = CT2A(input.GetString());

	for (std::string::size_type index = 0; index < inputData.size(); index++)
	{
		char c = inputData[index];

		if (level > 0 && '\r\n' == inputData[inputData.size() - 1])
		{
			result += getLevelStr(level);
		}

		switch (c)
		{
		case '{':
		case '[':
			result = result + c + NEW_LINE;
			level++;
			result += getLevelStr(level);
			break;
		case ',':
			result = result + c;

			// �ж��Ƿ���inputData�Ķ���
			if ((inputData[index - 1] == '"') || (inputData[index + 1] == '"')) {
				result = result + NEW_LINE;
				result += getLevelStr(level);
			}

			break;
		case '}':
		case ']':
			result += NEW_LINE;
			level--;
			result += getLevelStr(level);
			result += c;
			break;
		default:
			// �س���
			if (c == '\r') {
			}
			else {
				result += c;
			}
			break;
		}

	}

	// ���

	return result;
}
char* OperationClass::Utf8ToStringT(LPSTR str)
{
	_ASSERT(str);
	USES_CONVERSION;
	WCHAR* buf;
	int length = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	buf = new WCHAR[length];
	ZeroMemory(buf, (length) * sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, length);

	char* data = NULL;
	std::string recData = formatJson(W2A(buf));
	delete[]buf;
	buf = NULL;
	char* new_cstr = new char[recData.length() + 1]; // Ϊ�ǳ����ַ��������ռ�
	std::strcpy(new_cstr, recData.c_str()); // ��������
	data = new_cstr;
	delete[]str;
	str = NULL;
	return data;
}
char* OperationClass::URLDecode(const char* _url)
{
	_ASSERT(_url);
	USES_CONVERSION;
	// ��ȡ UTF-8 �ַ����ĳ���
	int wideSize = MultiByteToWideChar(CP_UTF8, 0, _url, -1, nullptr, 0);
	if (wideSize > 0) {
		// ������ַ�������
		wchar_t* wideStr = new wchar_t[wideSize];
		// ת��Ϊ���ַ�
		MultiByteToWideChar(CP_UTF8, 0, _url, -1, wideStr, wideSize);
		delete[] wideStr;
		wideStr = NULL;
	}

	int i = 0;
	int length = (int)strlen(_url) + 1;
	CHAR* buf = new CHAR[length];
	ZeroMemory(buf, length);
	LPSTR p = buf;
	char tmp[4];
	while (i < length)
	{
		if (i <= length - 3 && _url[i] == '%' && IsHexNum(_url[i + 1]) && IsHexNum(_url[i + 2]))
		{
			memset(tmp, 0, sizeof(tmp));
			memcpy(tmp, _url + i + 1, 2);
			sscanf(tmp, "%x", p++);
			i += 3;
		}
		else
		{
			*(p++) = _url[i];
			i++;
		}
	}

	//return CString(buf);
	return Utf8ToStringT(buf);
}
//��ȡ��ǰ�ĳ���ִ��·��
std::string OperationClass::GetExecutablePath()
{
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, path, MAX_PATH);
	return std::string(path);
}
// ��ȡ��ǰ��ִ���ļ����ڵ�Ŀ¼
std::string OperationClass::GetExecutableDirectory()
{
	std::string exePath = GetExecutablePath();
	return exePath.substr(0, exePath.find_last_of("\\/"));
}
//�����˻���Ϣ
void OperationClass::loadConfig() {
	CString configUrl;
	configUrl = GetExecutableDirectory().c_str();
	iniPath = configUrl + _T("\\config.ini");
	TCHAR server[32] = { 0 }, db[256] = { 0 }, userContent[32] = { 0 }, password[32] = { 0 }
	, vers[16] = { 0 }, portBuff[16] = { 0 }, keyValue[256] = { 0 };

	this->dbType = GetPrivateProfileInt(_T("database"), _T("dbType"), 0, iniPath);
	this->mssqlAuthMode = GetPrivateProfileInt(_T("database"), _T("authMode"), 0, iniPath);

	GetPrivateProfileString(_T("database"), _T("server"), _T(""), server, 32, iniPath);
	if (::wcscmp(server, L".") == 0)
		::wcscpy(server, L"127.0.0.1");
	GetPrivateProfileString(_T("database"), _T("db"), _T(""), db, 256, iniPath);
	GetPrivateProfileString(_T("database"), _T("user"), _T(""), userContent, 32, iniPath);
	GetPrivateProfileString(_T("database"), _T("password"), _T(""), password, 32, iniPath);
	GetPrivateProfileString(_T("database"), _T("vers"), _T(""), vers, 32, iniPath);
	GetPrivateProfileString(_T("database"), _T("port"), _T(""), portBuff, 32, iniPath);
	GetPrivateProfileString(_T("API"), _T("key"), _T(""), keyValue, 256, iniPath);

	this->serverName = server;
	this->dbName = db;
	this->username = userContent;
	this->password = password;
	this->dbVer = _tstof(vers);
	this->portStr = portBuff;
	this->key = keyValue;
	char portChars[16] = { 0 };
	size_t st;
	wcstombs_s(&st, portChars, portBuff, 16);
	port = atof(portChars);
}
bool OperationClass::ConnectDB()
{
	loadConfig();//��������
	dBInfo = new CDBInfo();
	dBInfo->dbName = this->dbName;
	dBInfo->dbType = 4;
	dBInfo->username = this->username;
	dBInfo->password = this->password;
	dBInfo->port = port;
	dBInfo->serverName = this->serverName;
	pConn = CConnProxyIfc::getConnProxy();
	pConn->isShortConnect = true;//������
	bool res = pConn->connect(dBInfo);
	if (res) {
		//�������ݿ�
		dBInfo->dbName = L"aiagent_memory";
		//pConn->close();
		pConn->create(dBInfo);
		pConn->connect(dBInfo);
		//�����û������б����ڱ���������ݺ�ʱ��
		CString sql = L"CREATE TABLE IF NOT EXISTS userList (\
ID SERIAL PRIMARY KEY,\
USERID_VECTOR VARCHAR[],\
CONTENT_VECTOR BYTEA[],\
addCode INT DEFAULT 0,\
datatime TIMESTAMP  DEFAULT NULL); ";
		pConn->execUpdate(sql.GetBuffer(0));
	}
	//�����ݽ�������˿�
	std::thread worker(std::bind(&OperationClass::httpTasks, this));
	std::this_thread::sleep_for(std::chrono::seconds(5));
	// �ȴ����߳����
	worker.detach(); // �����̣߳�ʹ���ں�̨����
	return res;
}
// ���ַ���ת��Ϊָ��������ֽ�����
std::string stringToHex(const std::string& str, const std::string& encoding = "UTF-8") {
	std::string result;
	if (encoding == "UTF-8") {
		for (unsigned char ch : str) {
			std::ostringstream oss;
			oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
			result += oss.str();
		}
	}
	else if (encoding == "GBK") {
		// UTF-8 -> GBK ����ת��
		int gbkSize = WideCharToMultiByte(CP_ACP, 0, std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str).c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string gbk(gbkSize, 0);
		WideCharToMultiByte(CP_ACP, 0, std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str).c_str(), -1, &gbk[0], gbkSize, nullptr, nullptr);

		// �� GBK �ֽ�����ת��Ϊ 16 ����
		for (unsigned char ch : gbk) {
			std::ostringstream oss;
			oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
			result += oss.str();
		}
	}
	return result;
}
std::string CStringToString(const CString& cstr) {
	std::string result;

	if (cstr.GetLength() == 0) {
		return result;  // ���ַ��������⴦��
	}

	int bufferSize = WideCharToMultiByte(CP_ACP, 0, cstr.GetString(), -1, nullptr, 0, nullptr, nullptr);
	if (bufferSize > 0) {
		char* buffer = new char[bufferSize];
		WideCharToMultiByte(CP_ACP, 0, cstr.GetString(), -1, buffer, bufferSize, nullptr, nullptr);
		result.assign(buffer);
		delete[] buffer;
	}

	return result;
}
// �� CString ת��Ϊ 16 �����ַ���
std::string CStringToHex(const CString& input) {
	std::string recData = CStringToString(input);
	std::wstring wide_string = boost::locale::conv::to_utf<wchar_t>(recData, "UTF-8");
	return stringToHex(recData);
	//std::ostringstream hexStream;

	//// ���� CString ��ÿ���ַ�
	//for (int i = 0; i < input.GetLength(); ++i) {
	//	unsigned char c = static_cast<unsigned char>(input[i]);
	//	// ��ʽ��Ϊ 2 λʮ������
	//	hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
	//}
	//return hexStream.str();
}

void  overlayTextFile(const std::string& filePath, const std::string& newData)
{
	// ���ļ�����ȡ��������
	std::ifstream inputFile(filePath, std::ios::in);
	std::string existingData;

	if (inputFile) {
		std::string line;
		while (std::getline(inputFile, line)) {
			existingData += line + "\n"; // ����ԭ����
		}
		inputFile.close();
	}

	// ��ԭ���ݺ������ݽ��е���
	std::string combinedData = existingData + newData;

	// ����������д���ļ�
	std::ofstream outputFile(filePath, std::ios::out | std::ios::trunc); // ����д��
	if (outputFile) {
		outputFile << combinedData;
		outputFile.close();
		std::cout << "�ļ��Ѹ��³ɹ���" << std::endl;
	}
	else {
		std::cerr << "�޷����ļ�����д�룡" << std::endl;
	}
}
// �� std::vector<std::string> ת��Ϊ SQL �����ʽ
std::string vectorToSqlArray(const std::vector<std::string>& vec) {

	std::string result = "ARRAY[";
	for (size_t i = 0; i < vec.size(); ++i) {
		result += "'" + vec[i] + "'::bytea";
		if (i != vec.size() - 1) {
			result += ", ";
		}
	}
	result += "]";
	return result;
}
// CString ת std::string �Ĺ��ߺ���
std::string ConvertCStringToString(const CString& cstr) {
#ifdef _UNICODE
	// ��� CString �� Unicode ����
	std::wstring wstr = std::wstring(cstr);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
#else
	// ��� CString �� ANSI ����
	return std::string(CT2A(cstr));
#endif
}
std::string createJsonWithArray(bool res, std::vector<std::string> content) {
	std::ostringstream oss;
	std::string key1 = "\"data";

	oss << "{";
	if (res == true) {
		oss << "\"success\": true,";
		oss << key1 << "\": [";
		for (size_t i = 0; i < content.size(); ++i) {
			oss << content[i];
			if (i < content.size() - 1) {
				oss << ", ";
			}
		}
		oss << "]";
		oss << "}";
	}
	else {
		oss << "\"success\": false,";
		oss << "}";
	}

	return oss.str();
}
// ��16�����ַ���ת��Ϊ�ֽ�����
std::vector<unsigned char> hexStringToBytes(const std::string& hex) {
	std::vector<unsigned char> bytes;
	for (size_t i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
		bytes.push_back(byte);
	}
	return bytes;
}

// ���ֽ�����תΪ�ַ�����֧�� GBK ���룩
std::string bytesToString(const std::vector<unsigned char>& bytes, const std::string& encoding = "GBK") {
	std::string result;
	if (encoding == "GBK") {
		// GBK -> UTF-8 ת��
		int wcharSize = MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<const char*>(bytes.data()), bytes.size(), nullptr, 0);
		std::wstring wideString(wcharSize, 0);
		MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<const char*>(bytes.data()), bytes.size(), &wideString[0], wcharSize);

		int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), nullptr, 0, nullptr, nullptr);
		result.resize(utf8Size);
		WideCharToMultiByte(CP_UTF8, 0, wideString.data(), wideString.size(), &result[0], utf8Size, nullptr, nullptr);
	}
	else if (encoding == "UTF-8") {
		result.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	}
	return result;
}
//ʹ�ñ���������Ƿ���json
bool isValidJson(const std::string& str) {
	// ����Ƿ������ֵĺ���
	auto isNumber = [](const std::string& s) {
		size_t i = 0;
		if (s[i] == '-' || s[i] == '+') i++; // �����ź�����
		bool dotSeen = false;
		for (; i < s.size(); i++) {
			if (std::isdigit(s[i])) continue;
			if (s[i] == '.' && !dotSeen) {
				dotSeen = true;
				continue;
			}
			return false;
		}
		return true;
		};

	std::stack<char> brackets;  // ��ջ����֤����ƥ��
	bool inString = false;  // ����Ƿ����ַ�����
	bool escape = false;  // ����Ƿ����ڴ���ת���ַ�
	bool lastWasComma = false;  // ���ǰһ���ַ��Ƿ��Ƕ���
	bool valid = true;
	size_t i = 0;

	// �����հ��ַ�
	auto skipWhitespace = [&]() {
		while (i < str.size() && std::isspace(str[i])) {
			i++;
		}
		};

	while (i < str.size()) {
		char c = str[i];

		skipWhitespace();  // �����հ��ַ�

		if (inString) {
			// ����ַ����е�ת���ַ�
			if (escape) {
				escape = false;  // ����ת���ַ�
			}
			else if (c == '\\') {
				escape = true;  // ��ʼת���ַ�
			}
			else if (c == '"') {
				inString = false;  // �ַ�������
			}
		}
		else {
			// ���� JSON �ķ��ַ�������
			if (c == '{' || c == '[') {
				brackets.push(c);  // �����ţ���ջ
			}
			else if (c == '}' || c == ']') {
				if (brackets.empty()) {
					valid = false;  // ������ʱջΪ�գ���ʾ��ƥ��
					break;
				}
				char top = brackets.top();
				if ((c == '}' && top != '{') || (c == ']' && top != '[')) {
					valid = false;  // ���Ų�ƥ��
					break;
				}
				brackets.pop();  // �����ţ���ջ
			}
			else if (c == '"') {
				inString = true;  // �����ַ���
			}
			else if (c == ',' && lastWasComma) {
				valid = false;  // �����Ķ����ǷǷ���
				break;
			}
			else if (c == ':') {
				// ���ð�ź������
				if (i == 0 || i == str.size() - 1 || str[i - 1] == '{' || str[i - 1] == '[') {
					valid = false;  // ð��ǰ����ֵ���Ƿ�
					break;
				}
			}
			else if (std::isdigit(c) || c == '-' || c == '.') {
				// ��������Ƿ�Ϸ�
				size_t start = i;
				while (i < str.size() && (std::isdigit(str[i]) || str[i] == '.' || str[i] == '-')) {
					i++;
				}
				if (!isNumber(str.substr(start, i - start))) {
					valid = false;  // ���ָ�ʽ����ȷ
					break;
				}
				i--;  // ����λ�ã���ֹ��������
			}
			else if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
				// ������Ч�ַ�
				valid = false;
				break;
			}

			if (c == ',') {
				lastWasComma = true;
			}
			else {
				lastWasComma = false;
			}
		}
		i++;
	}

	// ȷ���������Ŷ���ƥ��
	if (!brackets.empty()) {
		valid = false;
	}


	return valid;
}
//ȥ���ַ����Ŀո�ͻس�
void removeSpacesAndNewlines(std::string& str) {
	str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) {
		return std::isspace(c) || c == '\r' || c == '\n';  // ȥ���ո�ͻس����з�
		}), str.end());
}
//����json���ݣ���ִ�ж�Ӧ��ָ��
void getJsonInfo(CConnProxyIfc* pConn, std::string jsonData, std::string& recData) {
	std::signal(SIGSEGV, signalHandler);
	try {

		Json::Value jsonDataValue;
		Json::Value searchjsonData;
		Json::CharReaderBuilder readerBuilder;
		std::string errs;
		// �� JSON ���ݷ���������
		std::istringstream iss(jsonData);
		/*if (!iss.good()) {
			throw std::runtime_error("Input stream is invalid.");
		}*/
		//removeSpacesAndNewlines(jsonData);
		// ʹ�� JsonCpp �� parseFromStream �������� JSON
		if (Json::parseFromStream(readerBuilder, iss, &jsonDataValue, &errs)) {
			// �������ʧ�ܣ���ӡ������Ϣ������
		//��jsonData���ݽ��н������
			if (!jsonDataValue.isMember("function")) {
				recData = "{\"success\":false,\"errMsg\":\"function not exist\"}";
				return;
			}
			if (!jsonDataValue.isMember("data")) {
				recData = "{\"success\":false,\"errMsg\":\"data not exist\"}";
				return;
			}
			bool res = true;
			//����jsonData���ݿ�ʼ��һ���Ĳ���
			CString command = jsonDataValue["function"].asCString();
			if (command == L"addMemory") {//���������£���¼
				//����ÿһ����¼�������뵽���ݿ���
				int recCount = jsonDataValue["data"].size();
				for (int i = 0; i < recCount; i++)
				{
					if (!jsonDataValue["data"][i].isMember("userContent")) {
						recData = "{\"success\":false,\"errMsg\":\"userContent not exist\"}";
						return;
					}
					if (!jsonDataValue["data"][i].isMember("agentContent")) {
						recData = "{\"success\":false,\"errMsg\":\"agentContent not exist\"}";
						return;
					}
					if (!jsonDataValue["data"][i].isMember("time")) {
						recData = "{\"success\":false,\"errMsg\":\"time not exist\"}";
						return;
					}
					if (!jsonDataValue["data"][i].isMember("addCode")) {
						recData = "{\"success\":false,\"errMsg\":\"addCode not exist\"}";
						return;
					}
					CString userContent = jsonDataValue["data"][i]["userContent"].asCString();
					CString agentContent = jsonDataValue["data"][i]["agentContent"].asCString();

					std::string userContentTemp = CStringToHex(userContent);
					std::string agentContentTemp = CStringToHex(agentContent);
					userContent = userContentTemp.c_str();
					agentContent = agentContentTemp.c_str();
					CString time = jsonDataValue["data"][i]["time"].asCString();
					int addCode = jsonDataValue["data"][i]["addCode"].asInt();
					//��������
					CString sql;
					//�Ȳ��ID
					sql.Format(_T("SELECT ID FROM userList WHERE CONTENT_VECTOR @> ARRAY[decode('%s', 'hex')::bytea, decode('%s', 'hex')::bytea]::bytea[]"), userContent
						, agentContent);
					std::string stdStr;
					stdStr = CT2A(sql);  // ANSI ����
					//overlayTextFile("D:\\errLog.txt", stdStr);
					int row, col;
					CString** info = pConn->query(sql.GetBuffer(0), row, col);
					if (row > 0)
					{//����
						sql.Format(_T("UPDATE userList SET CONTENT_VECTOR=ARRAY[decode('%s', 'hex')::bytea, decode('%s', 'hex')::bytea]::bytea[], datatime = '%s',addCode='%d';"), userContent
							, agentContent, time, addCode);
						int errorCode = pConn->execUpdate(sql.GetBuffer(0));
						if (!errorCode) {
							res = false;
						}
					}
					else {//����
						//overlayTextFile("D:\\errLog.txt", "����");
						sql.Format(_T("INSERT INTO userList (CONTENT_VECTOR,datatime,addCode) VALUES(\
					ARRAY[decode('%s', 'hex')::bytea, decode('%s', 'hex')::bytea]::bytea[], '%s','%d'); "), userContent
							, agentContent, time, addCode);
						//stdStr = CT2A(sql);  // ANSI ����
						//overlayTextFile("D:\\errLog.txt", stdStr);
						int errorCode = pConn->execInsert(sql.GetBuffer(0));
						if (!errorCode) {
							res = false;
						}
					}
				}
			}
			else if (command == L"searchMemory") {
				if (!jsonDataValue["data"][0].isMember("startTime")) {
					recData = "{\"success\":false,\"errMsg\":\"startTime not exist\"}";
					return;
				}
				if (!jsonDataValue["data"][0].isMember("endTime")) {
					recData = "{\"success\":false,\"errMsg\":\"endTime not exist\"}";
					return;
				}
				CString startTime = jsonDataValue["data"][0]["startTime"].asCString();
				CString endTime = jsonDataValue["data"][0]["endTime"].asCString();
				int addCode = jsonDataValue["data"][0]["addCode"].asInt();
				//��ѯ��¼
				//SELECT  ARRAY(SELECT encode(element, 'hex') FROM unnest(CONTENT_VECTOR) AS element) AS content_vector_hex,\
				datatime, addCode FROM userList 
				CString sql;
				sql.Format(L"SELECT  ARRAY(SELECT encode(element, 'hex') FROM unnest(CONTENT_VECTOR) AS element) AS content_vector_hex,\
		datatime, addCode FROM userList where (datatime between '%s' and '%s') and addCode='%d'", startTime, endTime, addCode);
				int row, col;
				CString** info = pConn->query(sql.GetBuffer(0), row, col);
				//searchjsonData["success"] = true;
				std::vector<std::string> content;
				for (int i = 0; i < row; i++) {
					std::string input = std::string(CT2A(info[i][0]));
					if (input.front() == '{' && input.back() == '}') {
						input = input.substr(1, input.size() - 2);
					}
					std::vector<std::string> resultInfo;
					// �ָ��ַ���
					std::stringstream ss(input);
					std::string item;
					while (std::getline(ss, item, ',')) {
						std::string hexString = item;
						std::vector<unsigned char> bytes = hexStringToBytes(hexString); // תΪ�ֽ�����
						std::string result = bytesToString(bytes, "GBK");               // תΪ�ַ���
						resultInfo.push_back(result);
					}


					std::string userContent = resultInfo.at(0);
					std::string agentContent = resultInfo.at(1);

					std::string contentInfo;
					std::ostringstream oss;
					oss << "{";
					oss << "\"userContent\": \"" << userContent << "\", ";
					oss << "\"agentContent\": \"" << agentContent << "\"";
					oss << "}";

					contentInfo = oss.str();
					content.push_back(contentInfo);
					//searchjsonData["data"][i]["userContent"] = resultInfo.at(0);
					//searchjsonData["data"][i]["agentContent"] = resultInfo.at(1);
					//searchjsonData["data"][i]["time"] = std::string(CT2A(info[i][1]));
				}
				std::string output = createJsonWithArray(true, content);
				pConn->clearHeap();
				//Json::StreamWriterBuilder writer;
				//writer["indentation"] = "  "; // ��������Ϊ�����ո�
				//std::string output = Json::writeString(writer, jsonData);
				recData = output;
				return;
				//int recCount = jsonDataValue["data"].size();
			}
			else if (command == L"delMemory") {
				//ɾ����¼
				int recCount = jsonDataValue["data"].size();
				for (int i = 0; i < recCount; i++) {
					if (!jsonDataValue["data"][i].isMember("userContent")) {
						recData = "{\"success\":false,\"errMsg\":\"userContent not exist\"}";
						return;
					}
					if (!jsonDataValue["data"][i].isMember("agentContent")) {
						recData = "{\"success\":false,\"errMsg\":\"agentContent not exist\"}";
						return;
					}
					CString userContent = jsonDataValue["data"][i]["userContent"].asCString();
					CString agentContent = jsonDataValue["data"][i]["agentContent"].asCString();
					std::string userContentTemp = CStringToHex(userContent);
					std::string agentContentTemp = CStringToHex(agentContent);
					userContent = userContentTemp.c_str();
					agentContent = agentContentTemp.c_str();
					CString sql;
					sql.Format(L"DELETE FROM userList WHERE CONTENT_VECTOR=ARRAY[decode('%s', 'hex')::bytea, decode('%s', 'hex')::bytea]::bytea[]", userContent
						, agentContent);
					int errorCode = pConn->execUpdate(sql.GetBuffer(0));
					if (!errorCode) {
						res = false;
					}
				}
			}
			else {
				recData = "{\"success\":false,\"errMsg\":\"Method does not exist\"}";
				return;
			}
			if (res) {//�����ɹ�
				recData = "{\"success\":true,\"errMsg\":\"\"}";
			}
			else {
				recData = "{\"success\":false,\"errMsg\":\"data error\"}";
			}
		}
		else {
			throw Json::Exception("{\"success\":false,\"errMsg\":\"Error parsing JSON\"}");
		}
	}
	catch (const beast::system_error& e) {
		recData = "{\"success\":false,\"errMsg\":\"HTTP Error" + std::string(e.what()) + std::string(":\"}");
		std::cerr << "HTTP Error: " << e.what() << "\n";
	}
	catch (Json::Exception e) {
		recData = "{\"success\":false,\"errMsg\":\"Error parsing JSON\"}";
	}

	catch (const std::exception& e) {
		recData = e.what();
	}
	catch (...) {
		recData = "{\"success\":false,\"errMsg\":\"Error parsing JSON\"}";
	}
}
std::wstring utf8_to_utf16(const std::string& utf8_str) {
	// ��ȡ��Ҫ�Ŀ��ַ���������С
	int wstr_size = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
	if (wstr_size == 0) {
		throw std::runtime_error("Error converting UTF-8 to UTF-16");
	}

	// ���仺����������ת��
	std::wstring utf16_str(wstr_size, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &utf16_str[0], wstr_size);

	// �Ƴ�ĩβ�Ŀ��ַ�
	utf16_str.pop_back();
	return utf16_str;
}
//std:wstringתstd::string
std::string wstringToString(const std::wstring& wstr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

// ������ HTTP ����
void handle_request(http::request<http::string_body>& req, http::response<http::string_body>& res, CConnProxyIfc* pConn) {
	if (req.method() == http::verb::get) {
		res.result(http::status::ok);
		res.body() = "Hello, GET request received!";
		//����json���ݲ�ִ�й���
		std::string recJson;
		std::wstring recJson2 = utf8_to_utf16(req.body());
		std::string json = wstringToString(recJson2);
		getJsonInfo(pConn, wstringToString(recJson2), recJson);
		res.body() = recJson;
	}
	else if (req.method() == http::verb::post) {
		res.result(http::status::ok);
		res.body() = "Hello, POST data received: " + req.body();

		//����json���ݲ�ִ�й���
		std::string recJson;
		std::wstring recJson2 = utf8_to_utf16(req.body());
		CString recData = recJson2.c_str();
		getJsonInfo(pConn, std::string((CW2A)recJson2.c_str()), recJson);
		res.body() = recJson;
	}
	else {
		res.result(http::status::bad_request);
		res.body() = "Unsupported HTTP method";
	}

	res.prepare_payload();
}

// ���� HTTP �Ự
void do_session_http(tcp::socket socket, CConnProxyIfc* pConn) {
	beast::flat_buffer buffer;

	try {
		// ��ȡ����
		http::request<http::string_body> req;
		http::read(socket, buffer, req);

		// ������Ӧ
		http::response<http::string_body> res;
		res.version(req.version());
		res.set(http::field::server, "Boost.Beast HTTP Server");
		handle_request(req, res, pConn);

		// д����Ӧ
		http::write(socket, res);

		// ���Źر�����
		beast::error_code ec;
		socket.shutdown(tcp::socket::shutdown_both, ec);
	}
	catch (const beast::system_error& e) {
		std::cerr << "HTTP Error: " << e.what() << "\n";
	}
}

// ���� HTTPS �Ự
void do_session_https(beast::ssl_stream<beast::tcp_stream> stream, CConnProxyIfc* pConn) {
	beast::flat_buffer buffer;

	try {
		// SSL ����
		stream.handshake(ssl::stream_base::server);
		// ��ȡ����
		http::request<http::string_body> req;
		http::read(stream, buffer, req);

		// ������Ӧ
		http::response<http::string_body> res;
		res.version(req.version());
		res.set(http::field::server, "Boost.Beast HTTPS Server");
		handle_request(req, res, pConn);


		// д����Ӧ
		http::write(stream, res);
		// �ر� SSL ��
		stream.shutdown();
	}
	catch (const std::error_code& e) {
		std::cerr << "SSL Error: " << e.message() << "\n";
	}
	catch (const beast::system_error& e) {
		std::cerr << "HTTPS Error: " << e.what() << "\n";
	}
}

// ���� HTTP �˿�
void run_http_server(net::io_context& ioc, unsigned short port, CConnProxyIfc* pConn) {
	tcp::acceptor acceptor(ioc, { tcp::v4(), port });

	while (true) {
		try {
			tcp::socket socket(ioc);
			acceptor.accept(socket);

			std::thread(&do_session_http, std::move(socket), pConn).detach();
		}
		catch (const std::exception& e) {
			std::cerr << "HTTP accept error: " << e.what() << "\n";
		}
	}
}

// ���� HTTPS �˿�
void run_https_server(net::io_context& ioc, ssl::context& ctx, unsigned short port, CConnProxyIfc* pConn) {
	tcp::acceptor acceptor(ioc, { tcp::v4(), port });

	while (true) {
		try {
			tcp::socket socket(ioc);
			acceptor.accept(socket);

			beast::ssl_stream<beast::tcp_stream> stream(std::move(socket), ctx);
			std::thread(&do_session_https, std::move(stream), pConn).detach();
		}
		catch (const std::exception& e) {
			std::cerr << "HTTPS accept error: " << e.what() << "\n";
		}
	}
}

//�����������
void OperationClass::httpTasks() {
	try {
		std::string configUrl;
		configUrl = GetExecutableDirectory();

		const unsigned short http_port = 8082;
		const unsigned short https_port = 8081;

		net::io_context ioc;
		ssl::context ctx(ssl::context::sslv23);

		// ����֤���ļ�
		ctx.use_certificate_file(configUrl + "\\server.crt", ssl::context::pem);
		// ����˽Կ�ļ�
		ctx.use_private_key_file(configUrl + "\\server.key", ssl::context::pem);
		// ���� SSL ��֤ģʽ
		ctx.set_verify_mode(ssl::verify_none);

		// �ֱ����� HTTP �� HTTPS ������
		std::thread http_thread([&]() { run_http_server(ioc, http_port, pConn); });
		std::thread https_thread([&]() { run_https_server(ioc, ctx, https_port, pConn); });

		std::cout << "Server is running on HTTP port " << http_port << " and HTTPS port " << https_port << "...\n";

		// �ȴ��߳����
		http_thread.join();
		https_thread.join();
	}
	catch (const std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << "\n";
	}

	return;
}
void OperationClass::closeDB() {
	pConn->close();
}
//�ַ���תBYTE*
BYTE* OperationClass::convertToByte(std::string content, int& len) {
	const BYTE* bytePtr = reinterpret_cast<const BYTE*>(content.c_str());
	// ���ÿ���ֽ�
	for (size_t i = 0; i < content.size(); ++i) {
		printf("%02X ", bytePtr[i]); // ��ӡΪʮ������
	}
	len = strlen(reinterpret_cast<const char*>(bytePtr));
	BYTE* data = (BYTE*)bytePtr;
	return data;
}