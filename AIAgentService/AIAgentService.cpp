#include <sstream>
#include <iostream>
#include <fstream> 
#include <string>
#include <windows.h>
#include <objbase.h>
#include <wbemidl.h>
#include <comdef.h>
#include <mutex>
#include <cstdlib>
#include <tlhelp32.h>
#include <stdio.h>
#include <Cstring>
#include <thread>
#include <cstdlib>
#include <future>
#include <chrono>
#include "OperationClass.h"
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")

const int SLEEP_TIME = 5000;
BOOL bFlag = TRUE;

SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
SERVICE_STATUS m_ServiceStatus;
HANDLE g_StopEvent = NULL;
// 互斥体防止多实例
HANDLE g_ServiceMutex = NULL;
void WINAPI ServiceMain(int argc, char* argv[]);
//DWORD WINAPI ServiceCtrlHandler(DWORD Opcode);
DWORD WINAPI ServiceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

LPVOID myContext;
using namespace std;
// wmi namespace
HRESULT hr = S_OK;

static WNDCLASS wc = { 0 };

int WriteToLog();
void WINAPI ServiceMain(int argc, char* argv[]);
DWORD WINAPI ServiceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
// 停止服务
void StopService() {
	if (g_StopEvent) {
		SetEvent(g_StopEvent); // 通知线程停止
	}
	m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
}
std::string cmdPopen(const std::string& cmdLine) {
	char buffer[1024] = { '\0' };
	FILE* pf = NULL;
	pf = _popen(cmdLine.c_str(), "r");
	if (NULL == pf) {
		printf("open pipe failed\n");
		return std::string("");
	}
	std::string ret;
	while (fgets(buffer, sizeof(buffer), pf)) {
		ret += buffer;
	}
	_pclose(pf);
	return ret;
}

LRESULT CALLBACK WindowProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
);
HWND createMsgWin() {
	HINSTANCE lvhInstance;
	lvhInstance = GetModuleHandle(NULL);
	WNDCLASS lvwcCls;
	lvwcCls.cbClsExtra = 0;
	lvwcCls.cbWndExtra = 0;
	lvwcCls.hCursor = LoadCursor(lvhInstance, IDC_ARROW);
	lvwcCls.hIcon = LoadIcon(lvhInstance, IDI_APPLICATION);
	lvwcCls.lpszMenuName = NULL;
	lvwcCls.style = CS_HREDRAW | CS_VREDRAW;
	lvwcCls.hbrBackground = (HBRUSH)COLOR_WINDOW;
	lvwcCls.lpfnWndProc = WindowProc;
	lvwcCls.lpszClassName = (L"RenderWindow");
	lvwcCls.hInstance = lvhInstance;

	RegisterClass(&lvwcCls);

	HWND lvhwndWin = CreateWindowA(
		("RenderWindow"),
		"Zombie",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		30,
		20,
		NULL,
		NULL,
		lvhInstance,
		NULL);

	//去标题栏  

	return lvhwndWin;
}


LRESULT CALLBACK WindowProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
)
{
	//cout << "MSG:" << uMsg << ",wParam:" << wParam << ",lParam:" << lParam << endl;
	switch (uMsg)
	{

	case WM_POWERBROADCAST:
	{
		if (wParam == PBT_POWERSETTINGCHANGE) {
			POWERBROADCAST_SETTING* lvpsSetting = (POWERBROADCAST_SETTING*)lParam;
			byte lvStatus = *(lvpsSetting->Data);
			std::string cmdLine(R"("tasklist | findstr OSD.exe")");
			std::string tmp = cmdPopen(cmdLine);
			
			if (lvStatus == 0) {

				cout << "Monitor is turn off" << endl;
				if (tmp.size() != 0)
				{
					ShellExecute(NULL, L"open", L"\"C:\\Program Files\\OSD\\SU_OSD_KILL.bat\"", NULL, NULL, SW_SHOWNORMAL);
					cout << "OSD down" << endl;
					Sleep(2000);

				}

			}
			//cout << (int)lvStatus << endl;

		}
		break;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
HWND mhMsgRec;

BOOL WINAPI ConsoleHandler(DWORD pvdwMsgType)
{
	if (pvdwMsgType == CTRL_C_EVENT)
	{
		PostMessage(mhMsgRec, WM_DESTROY, 0, 0);
		return TRUE;
	}
	else if (pvdwMsgType == CTRL_CLOSE_EVENT)
	{
		PostMessage(mhMsgRec, WM_DESTROY, 0, 0);
		return TRUE;
	}
	return FALSE;
}

void runTasks() {
	OperationClass operation;
	//初始化数据库
	operation.ConnectDB();

}
void WINAPI ServiceMain(int argc, char* argv[])
{
	m_ServiceStatusHandle = RegisterServiceCtrlHandler(L"AI_AGENT_EVENT", (LPHANDLER_FUNCTION)ServiceCtrlHandler);
	if (!m_ServiceStatusHandle)
	{
		return;
	}
	MEMORYSTATUS memstatus;
	m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	m_ServiceStatus.dwWin32ExitCode = 0;
	m_ServiceStatus.dwServiceSpecificExitCode = 0;
	m_ServiceStatus.dwCheckPoint = 0;
	m_ServiceStatus.dwWaitHint = 0;

	SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
	//m_ServiceStatusHandle = RegisterServiceCtrlHandler(L"SU_SLEEP_TEST", (LPHANDLER_FUNCTION)ServiceCtrlHandler);
	// 创建互斥体，防止多实例
	g_ServiceMutex = CreateMutex(NULL, TRUE, L"MyServiceMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		std::cerr << "服务已运行，退出。" << std::endl;
		m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
		return;
	}

	// 初始化停止事件
	g_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!g_StopEvent) {
		m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
		return;
	}

	m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);


	//mhMsgRec = createMsgWin();
	//HPOWERNOTIFY lvhpNotify = RegisterPowerSettingNotification(mhMsgRec, &GUID_CONSOLE_DISPLAY_STATE, DEVICE_NOTIFY_WINDOW_HANDLE);
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);

	//重新开启一个线程，用于和主程序之间做管道的操作
	// 启动子线程
	std::thread worker(runTasks);

	// 主线程可以做其他事情
	std::this_thread::sleep_for(std::chrono::seconds(5));

	// 等待子线程完成
	worker.detach(); // 分离线程，使其在后台运行
	// 清理资源
	if (g_StopEvent) {
		CloseHandle(g_StopEvent);
	}
	if (g_ServiceMutex) {
		ReleaseMutex(g_ServiceMutex);
		CloseHandle(g_ServiceMutex);
	}
	bool lvbRet;
	MSG lvMSG;
	while ((lvbRet = GetMessage(&lvMSG, NULL, 0, 0)) != 0)
	{
		TranslateMessage(&lvMSG);
		DispatchMessage(&lvMSG);
		if (lvMSG.message == WM_DESTROY) {
			break;
		}
	}
	// 清理资源
	if (g_StopEvent) {
		CloseHandle(g_StopEvent);
	}
	if (g_ServiceMutex) {
		ReleaseMutex(g_ServiceMutex);
		CloseHandle(g_ServiceMutex);
	}
	//UnregisterPowerSettingNotification(lvhpNotify);
	//CloseWindow(mhMsgRec);
}
DWORD ServiceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {

	switch (dwControl) {

	case SERVICE_CONTROL_STOP:
		//终止PcManagerStatusCheck.exe进程
		m_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
		StopService();
		//ExitProcess(0); // 确保服务退出
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		//bFlag = FALSE;
		//m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
		break;
	default:
		ostringstream oStr;
		oStr << "lpHandlerProc dwControl=" << dwControl;
		break;
	}
	SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
	return 0;
}
int main()
{
	int ret = SetProcessShutdownParameters(0x3FF, 0);
	if (ret == 0) {
	}
	SERVICE_TABLE_ENTRY DispatchTable[2];
	DispatchTable[0].lpServiceName = _bstr_t("AI_AGENT_EVENT");
	DispatchTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	DispatchTable[1].lpServiceName = NULL;
	DispatchTable[1].lpServiceProc = NULL;
	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		int errCode = GetLastError();
		// 启动失败的处理
		return errCode;
	}
}