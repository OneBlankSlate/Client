#pragma once
#include <Windows.h>
#include <iostream>
#include <tchar.h>
using namespace std;
enum
{
	CLIENT_LOGIN,				// ���߰�
	CLIENT_GET_OUT_REQUIRE,
	CLIENT_GET_OUT_REPLY,
	CLIENT_REMOTE_MESSAGE_REQUIRE,
	CLIENT_REMOTE_MESSAGE_REPLY,
	CLIENT_REMOTE_MESSAGE_COMPLETE,
	CLIENT_SHUT_DOWN_REQUIRE,
	CLIENT_SHUT_DOWN_REPLY,
	CLIENT_CMD_MANAGER_REQUIRE,
	CLIENT_CMD_MANAGER_REPLY,
	CLIENT_PROCESS_MANAGER_REQUIRE,
	CLIENT_PROCESS_MANAGER_REPLY,
	CLIENT_PROCESS_MANAGER_REFRESH_REQUIRE,
	CLIENT_PROCESS_MANAGER_KILL_REQUIRE,
	CLIENT_OPEN_PROCESS_REQUIRE,
	CLIENT_WINDOW_MANAGER_REQUIRE,
	CLIENT_WINDOW_MANAGER_REPLY,
	CLIENT_WINDOW_MANAGER_REFRESH_REQUIRE,
	CLIENT_REMOTE_CONTROLLER_REQUIRE,
	CLIENT_REMOTE_CONTROLLER_REPLY,
	CLIENT_REMOTE_CONTROLLER_FIRST_SCREEN,
	CLIENT_REMOTE_CONTROLLER_NEXT_SCREEN,
	CLIENT_REMOTE_CONTROLLER_CONTROL_REQUIRE,
	CLIENT_REMOTE_CONTROLLER_BLOCK_INPUT_REQUIRE,
	CLIENT_REMOTE_CONTROLLER_GET_CLIPBOARD_REQUIRE,
	CLIENT_REMOTE_CONTROLLER_SET_CLIPBOARD_REQUIRE,
	CLIENT_REMOTE_CONTROLLER_GET_CLIPBOARD_REPLY,
	CLIENT_REMOTE_CONTROLLER_SET_CLIPBOARD_REPLY,
	CLIENT_FILE_MANAGER_REQUIRE,
	CLIENT_FILE_MANAGER_REPLY,
	CLIENT_FILE_MANAGER_FILE_LIST_REQUIRE,
	CLIENT_FILE_MANAGER_FILE_LIST_REPLY,
	CLIENT_FILE_MANAGER_SEND_FILE_INFORMATION,
	CLIENT_FILE_MANAGER_TRANSFER_MODE_REQUIRE,        //�ڿͻ����з������������ļ�
	CLIENT_FILE_MANAGER_SET_TRANSFER_MODE,
	CLIENT_FILE_MANAGER_FILE_DATA,
	CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE,
	CLIENT_AUDIO_MANAGER_REQUIRE,
	CLIENT_AUDIO_MANAGER_REPLY,
	CLIENT_AUDIO_MANAGER_RECORD_DATA,
	CLIENT_VIDEO_MANAGER_REQUIRE,
	CLIENT_SERVICE_MANAGER_REQUIRE,
	CLIENT_SERVICE_MANAGER_REPLY,
	CLIENT_SERVICE_MANAGER_CONFIG_REQUIRE,
	CLIENT_REGISTER_MANAGER_REQUIRE,
	CLIENT_REGISTER_MANAGER_REPLY,
	CLIENT_REGISTER_MANAGER_DATA_CONTINUE,
	CLIENT_REGISTER_MANAGER_PATH_DATA_REPLY,
	CLIENT_REGISTER_MANAGER_KEY_DATA_REPLY,
	CLIENT_GO_ON,
	CLIENT_PROCESS_QUERY_REQUIRE,
	CLIENT_REMOTE_CHANGE_MEMORY_REQUIRE,
	CLIENT_FIRST_SCAN_REQUIRE,
	CLIENT_FIRST_SCAN_REPLY,
	CLIENT_SCAN_AGAIN_REPLY,
	CLIENT_SCAN_AGAIN_REQUIRE,
	CLIENT_EDIT_VALUE_REQUIRE
};