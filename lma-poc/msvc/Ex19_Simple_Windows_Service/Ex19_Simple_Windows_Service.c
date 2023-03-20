#include <stdio.h>
#include <windows.h>
#pragma warning(disable : 4996)

/*
** Mô tả:
** - Chương trình Windows Service đơn giản bằng C/C++
** - Windows Service là chương trình chạy nền/background, không đòi hỏi tương tác người dùng
** - Thường các Service sẽ phụ trách một tác vụ nào đó.
** Cài đặt Service
** $ sc.exe create "MemoryStatus" binpath= C:\MyService\QueryMemory.exe
** Gỡ bỏ Service
** $ sc delete "MemoryStatus"
** Theo mặc định thì service chưa start:
** $ sc start/stop "MemoryStatus"
** Ref:
** 1. 5 bước tạo 1 ứng dụng Windows services bằng ngôn ngữ C
**	http://vncoding.net/2015/11/26/5-buoc-tao-1-ung-dung-windows-services-bang-ngon-ngu-c/
** 2. Simple Windows Service in C++
**	https://www.codeproject.com/Articles/499465/Simple-Windows-Service-in-Cplusplus
*/

// Khoảng thời gian giữa 2 lần truy vấn memory
#define SLEEP_TIME 5000
// Đường dẫn file log
#define LOGFILE "C:\\memstatus.log"

// Struct lưu thông tin trạng thái service
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int InitService();
int WriteToLog(char* str);


void main()
{
	// Chương trình có thể có nhiều service. Ta sẽ định nghĩa 2 service, nhưng chỉ dùng 1
	// Thông tin về các service đc lưu vào một mảng kiểu struct là SERVICE_TABLE_ENTRY
	SERVICE_TABLE_ENTRY ServiceTable[2];
	// Đặt tên cho service
	ServiceTable[0].lpServiceName = L"QueryMemory";
	// Con trỏ, trỏ đến hàm main của service.
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	// Service thứ 2 không khởi tạo nên gán NULL
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	/*
	** Start the control dispatcher thread for our service
	** Services Control Manager(SCM) quản lý tất cả các service của OS và App
	** Khi SCM start service thì hàm main() này trở thành một Control dispatcher. Nó có nhiệm vụ:
	**  + Tạo mỗi thread con cho mỗi service, trong ví dụ này ta chỉ có một service => 1 thread con đc tạo
	**  + Giám sát việc thực thi của các service đc định nghĩa
	**  + Gửi "control request" từ SCM tới service. (*)
	*/
	StartServiceCtrlDispatcher(ServiceTable);
	/*
	** VD cho (*): Người dùng Stop service
	** 1. Từ SCM nhận yêu cầu dừng service
	** 2. Control dispatcher tiếp nhận. Gửi yêu cầu đến service (Hàm ControlHandler sẽ xử lý tiếp)
	** 3. Service stop, hàm StartServiceCtrlDispatcher() sẽ return;
	*/
}

// ServiceMain là entrypoint của một service
// Khi thread con tạo, nó nhảy vào đây thực thi
void ServiceMain(int argc, char** argv)
{
	int error;

	// Khai báo các thuộc tính của service hiện tại.
	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	/*
	** ServiceMain gọi hàm RegisterServiceCtrlHandler để đăng kí hàm ControlHandler
	** ControlHandler sẽ xử lý các request do "Control dispatcher" gửi về từ SCM
	*/
	hStatus = RegisterServiceCtrlHandler(
		L"QueryMemory",
		(LPHANDLER_FUNCTION)ControlHandler);
	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{
		// Registering Control Handler failed
		return;
	}
	// Initialize Service 
	error = InitService();
	if (error)
	{
		// Initialization failed
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = -1;
		SetServiceStatus(hStatus, &ServiceStatus);
		return;
	}
	// We report the running status to SCM. 
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus, &ServiceStatus);

	MEMORYSTATUS memory;
	// The worker loop of a service
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		char buffer[16];
		GlobalMemoryStatus(&memory);
		sprintf(buffer, "%d", memory.dwAvailPhys);
		int result = WriteToLog(buffer);
		if (result)
		{
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			ServiceStatus.dwWin32ExitCode = -1;
			SetServiceStatus(hStatus, &ServiceStatus);
			return;
		}

		Sleep(SLEEP_TIME);
	}
	return;
}

// Service initialization
int InitService()
{
	int result;
	result = WriteToLog("Monitoring started.");
	return(result);
}

// Control handler function
void ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		WriteToLog("Monitoring stopped.");
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &ServiceStatus);
		return;

	case SERVICE_CONTROL_SHUTDOWN:
		WriteToLog("Monitoring stopped.");
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &ServiceStatus);
		return;

	default:
		break;
	}
	// Report current status
	SetServiceStatus(hStatus, &ServiceStatus);
	return;
}

int WriteToLog(char* str)
{
	FILE* fpLog;
	fpLog = fopen(LOGFILE, "a+");
	if (fpLog == NULL)
		return -1;
	if (fprintf(fpLog, "%s\n", str) < strlen(str))
	{
		fclose(fpLog);
		return -1;
	}
	fclose(fpLog);
	return 0;
}