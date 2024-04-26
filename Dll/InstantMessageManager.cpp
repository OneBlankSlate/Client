#include "pch.h"
#include "InstantMessageManager.h"
#include "Common.h"
#include"dllmain.h"
#include"resource.h"
#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")
CHAR   __BufferData[0x1000] = { 0 };
CIocpClient* __IocpClient = NULL;
#define WINDOW_WIDTH		220     //���ص�
#define WINDOW_HEIGHT		150
#define ID_TIMER_POP_WINDOW		1
#define ID_TIMER_DELAY_DISPLAY	2 
#define ID_TIMER_CLOSE_WINDOW	3 

int __TimeEvent = 0;
CInstantMessageManager::CInstantMessageManager(CIocpClient* IocpClient) :CManager(IocpClient)
{
	//�ش����ݰ���������
	BYTE	IsToken = CLIENT_REMOTE_MESSAGE_REPLY;
	IocpClient->OnSending((char*)&IsToken, 1);



	__IocpClient = IocpClient;   //��ȫ�ֱ����ǲ�Ӧ�õ�  ������Ϣ�ص�����ͻ��ˣ�û�н��棩�봰����Դ����������� 

	//�ú����Ǹ�����ʵ��
	WaitingForDialogOpen();   //�ȴ�������������



}
CInstantMessageManager::~CInstantMessageManager()
{
	_tprintf(_T("~CInstantMessageManage()\r\n"));
}

void CInstantMessageManager::HandleIo(PBYTE BufferData, ULONG_PTR BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		//�ú����Ǹ�����ʵ��
		NotifyDialogIsOpen();
		break;
	}
	default:
	{
		//���Զ����Ϣ����������
		memcpy(__BufferData, BufferData, BufferLength);

		//����һ��Dialog��
		DialogBoxA(__InstanceHandle, MAKEINTRESOURCE(IDD_INSTANT_MESSAGE_DIALOG),
			NULL, DialogProcedure);  //SDK   C   MFC  C++
		break;
	}
	}
}
int CALLBACK DialogProcedure(HWND DialogHwnd, unsigned int Message,
	WPARAM ParameterData1, LPARAM ParameterData2)
{

	switch (Message)
	{
	case WM_INITDIALOG:
	{
		OnInitDialog(DialogHwnd);   //���ڱ�����֮��ͻ���ô˺���
		break;
	}
	case WM_TIMER:
	{
		OnTimerDialog(DialogHwnd);
		break;
	}

	}

	return 0;   //false  һ��Ҫע��
}
VOID OnInitDialog(HWND DialogHwnd)
{
	MoveWindow(DialogHwnd, 0, 0, 0, 0, TRUE);  //���Ƚ������Ƶ���Ļ֮��

	//��IDC_EDIT_DIALOG_REMOTE_MESSAGE_MANAGER_MAIN�ؼ�����������
	SetDlgItemText(DialogHwnd, IDC_INSTANT_MESSAGE_EDIT, __BufferData);  //�򴰿ڿؼ���set�ı�

	memset(__BufferData, 0, sizeof(__BufferData));

	__TimeEvent = ID_TIMER_POP_WINDOW;    //ʱ���¼�
	//����ʱ��  ����ʱ
	SetTimer(DialogHwnd, __TimeEvent, 1, NULL);

	PlaySound(MAKEINTRESOURCE(IDR_WAVE),
		__InstanceHandle, SND_ASYNC | SND_RESOURCE | SND_NODEFAULT);
}
VOID OnTimerDialog(HWND DialogHwnd)   //ʱ�ӻص�
{
	RECT  Rect;
	static int Height = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &Rect, 0);
	int y = Rect.bottom - Rect.top;
	//int x = Rect.right - Rect.left ;
	//x = x - WIN_WIDTH;            

	int x = 0;
	switch (__TimeEvent)
	{
	case ID_TIMER_CLOSE_WINDOW:
	{
		if (Height >= 0)
		{
			Height -= 5;
			MoveWindow(DialogHwnd, x, y - Height, WINDOW_WIDTH, Height, TRUE);
		}
		else
		{
			KillTimer(DialogHwnd, ID_TIMER_CLOSE_WINDOW);

			//֪ͨ���������Լ��������µ�����
			//BYTE IsToken = CLIENT_REMOTE_MESSAGE_COMPLETE;			   
			//__IOCPClient->OnSending((char*)&IsToken, 1);

			//�رյ�ǰ�Ի���
			EndDialog(DialogHwnd, 0);
		}
		break;
	}

	case ID_TIMER_DELAY_DISPLAY:
	{
		KillTimer(DialogHwnd, ID_TIMER_DELAY_DISPLAY);
		__TimeEvent = ID_TIMER_CLOSE_WINDOW;
		SetTimer(DialogHwnd, __TimeEvent, 5, NULL);
		break;
	}
	case ID_TIMER_POP_WINDOW:
	{
		if (Height <= WINDOW_HEIGHT)
		{
			Height += 3;
			MoveWindow(DialogHwnd, x, y - Height, WINDOW_WIDTH, Height, TRUE);
		}
		else
		{
			KillTimer(DialogHwnd, ID_TIMER_POP_WINDOW);
			__TimeEvent = ID_TIMER_DELAY_DISPLAY;
			SetTimer(DialogHwnd, __TimeEvent, 7000, NULL);
		}
		break;
	}
	}
}

