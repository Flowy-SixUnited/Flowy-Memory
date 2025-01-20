#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include "OperationClass.h"

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;
HANDLE hServiceStopEvent = nullptr;

// 服务名称
const TCHAR SERVICE_NAME[] = _T("AIAgentService");

// 日志函数
void WriteToLog(const std::string& message) {
    std::ofstream log("C:\\MyCppService.log", std::ios::app);
    if (log.is_open()) {
        log << message << std::endl;
    }
}

void runTasks() {
    OperationClass operation;
    //初始化数据库
    operation.ConnectDB();

}
// 服务控制处理函数
void WINAPI ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
    case SERVICE_CONTROL_STOP:
        WriteToLog("Service stopping...");
        ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetEvent(hServiceStopEvent);
        break;

    default:
        break;
    }
}

// 服务主函数
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    try {
        std::cout << "Service is starting..." << std::endl;
        ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
        if (ServiceStatusHandle == nullptr) {
            return;
        }

        // 初始化服务状态
        ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwServiceSpecificExitCode = 0;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;

        SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

        // 模拟初始化任务
        WriteToLog("Service is starting...");
        hServiceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (hServiceStopEvent == nullptr) {
            ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
            return;
        }

        // 设置服务为运行状态
        ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

        // 主服务循环
        WriteToLog("Service is running...");
        runTasks();
        while (WaitForSingleObject(hServiceStopEvent, 1000) == WAIT_TIMEOUT) {
            WriteToLog("Service is doing work...");
        }

        // 服务停止
        WriteToLog("Service is stopped.");
        CloseHandle(hServiceStopEvent);
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
    }
    catch (const std::exception& ex) {
        std::cerr << "Service failed to start: " << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Service failed to start due to unknown error." << std::endl;
    }

}

// 服务入口
int _tmain(int argc, TCHAR* argv[]) {
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        { (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        std::cerr << "Failed to start service control dispatcher: " << GetLastError() << std::endl;
        return 1;
    }

    return 0;
}
