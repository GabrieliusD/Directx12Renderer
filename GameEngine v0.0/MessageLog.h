#pragma once
#include <Windows.h>
#include <iostream>
#include <system_error>
#include <string>
#include <DirectXMath.h>

using namespace DirectX;
class MessageLog
{
private:
	MessageLog() = default;
	MessageLog(MessageLog& other) = delete;

	bool saveMessage = true;
public:
	static MessageLog& GetInstance()
	{
		static MessageLog instance;
		return instance;
	}
	static void PrintMessage(std::wstring message, HRESULT result) {
		GetInstance().PrintMessageImp(message, result);
	}
	static void PrintMatrix(XMMATRIX& matrix)
	{
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, matrix);
		PrintMatrix(m);
	}
	static void PrintMatrix(const XMFLOAT4X4& matrix)
	{
		
		GetInstance().PrintMatrixImp(matrix);
	}

private:
	
	void PrintMessageImp(std::wstring message, HRESULT result) {
		std::string hMessage = std::system_category().message(result);
		std::wstring whMess(hMessage.begin(), hMessage.end());
		std::wcout << message << " result:" << whMess << std::endl;
	}
	
	void PrintMatrixImp(const XMFLOAT4X4& matrix)
	{
		std::cout << "----------------" << std::endl;
		std::cout <<"|" << matrix._11 << "|"<< matrix._12 << "|" << matrix._13 << "|" << matrix._14 << "|"<< std::endl;
		std::cout << "----------------" << std::endl;
		std::cout << "|" << matrix._21 << "|" << matrix._22 << "|" << matrix._23 << "|" << matrix._24 << "|" << std::endl;
		std::cout << "----------------" << std::endl;
		std::cout << "|" << matrix._31 << "|" << matrix._32 << "|" << matrix._33 << "|" << matrix._34 << "|" << std::endl;
		std::cout << "----------------" << std::endl;
		std::cout << "|" << matrix._41 << "|" << matrix._42 << "|" << matrix._43 << "|" << matrix._44 << "|" << std::endl;
		std::cout << "----------------" << std::endl;

	}
};

