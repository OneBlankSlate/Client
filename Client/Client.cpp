#include"Client.h"
void _tmain()
{
	_tprintf(_T("ServerAddress:%s\r\n"), __ConnectInfo.SeverAddress);
	_tprintf(_T("ConnectPort:%d\r\n"), __ConnectInfo.ConnectPort);
	//����Dll.dllģ��
	HMODULE  ModuleHandle = (HMODULE)LoadLibrary(_T("Dll.dll"));   //�����Լ�д��ģ��
	if (ModuleHandle == NULL)
	{
		return;
	}
	//��ȡһ��Dllģ���е�һ����������
	LPFN_CLIENTRUN ClientRun =
		(LPFN_CLIENTRUN)GetProcAddress(ModuleHandle, "ClientRun");

	//û�л�ȡ��Dll.dll�еĵ�������
	if (ClientRun == NULL)
	{
		FreeLibrary(ModuleHandle);   //�ͷ�ģ�� �˳�
		return;
	}

	else
	{
		ClientRun(__ConnectInfo.SeverAddress, __ConnectInfo.ConnectPort);
	}



	_tprintf(_T("Input AnyKey To Exit\r\n"));
	_gettchar();
	FreeLibrary(ModuleHandle);   //�ͷ�ģ��

}

