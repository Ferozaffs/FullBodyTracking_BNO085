// Fill out your copyright notice in the Description page of Project Settings.


#include "SerialReader.h"
#include <string>

// Sets default values for this component's properties
USerialReader::USerialReader()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	m_overlappedRead = new OVERLAPPED();

	FMemory::Memset(m_overlappedRead, 0, sizeof(OVERLAPPED));
}

USerialReader::~USerialReader()
{
	Close();

	delete m_overlappedRead;
}

void USerialReader::Close()
{
	if (!m_hComm)
	{
		return;
	}

	if (m_overlappedRead->hEvent != NULL)
	{
		CloseHandle(m_overlappedRead->hEvent);
	}
	CloseHandle(m_hComm);
	m_hComm = NULL;
}

bool USerialReader::SetupSerialRead(int port, int baudRate)
{
	std::wstring portName = L"COM";
	portName += std::to_wstring(port);

	m_hComm = CreateFile(portName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (m_hComm == NULL)
	{
		unsigned long dwError = GetLastError();
		UE_LOG(LogTemp, Error, TEXT("Failed to open port COM%d. Error: %08X"), port, dwError);
		return false;
	}

	COMMTIMEOUTS CommTimeOuts;
	//CommTimeOuts.ReadIntervalTimeout = 10;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 10;
	SetCommTimeouts(m_hComm, &CommTimeOuts);

	m_overlappedRead->hEvent = CreateEvent(NULL, true, false, NULL);

	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
	GetCommState(m_hComm, &dcb);
	dcb.BaudRate = baudRate;
	dcb.ByteSize = 8;

	if (!SetCommState(m_hComm, &dcb) || !SetupComm(m_hComm, 10000, 10000) ||
		m_overlappedRead->hEvent == NULL)
	{
		unsigned long dwError = GetLastError();
		if (m_overlappedRead->hEvent != NULL)
		{
			CloseHandle(m_overlappedRead->hEvent);
		}
		CloseHandle(m_hComm);
		m_hComm = NULL;
		UE_LOG(LogTemp, Error, TEXT("Failed to setup port COM%d. Error: %08X"), port, dwError);
		return false;
	}
		
	return true;
}

// Called every frame
void USerialReader::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Read();
}

bool USerialReader::Read()
{
	if (m_hComm != nullptr)
	{	
		TArray<uint8> bytes;
		uint8 byte = 0x0;
		bool readStatus = false;
		unsigned long dwBytesRead, dwErrorFlags;
		COMSTAT comStat;	

		ClearCommError(m_hComm, &dwErrorFlags, &comStat);
		if (!comStat.cbInQue)
		{
			return false;
		}

		do
		{
			do
			{
				readStatus = ReadFile(m_hComm, &byte, 1, &dwBytesRead, m_overlappedRead);

				if (!readStatus)
				{
					if (GetLastError() == ERROR_IO_PENDING)
					{
						WaitForSingleObject(m_overlappedRead->hEvent, 2000);
					}
					else
					{
						bytes.Add(0x0);
						break;
					}
				}

				if (byte == '\n' || byte == '\0' || dwBytesRead == 0)
				{
					// when Terminator is \n, we know we're expecting lines from Arduino. But those
					// are ended in \r\n. That means that if we found the line Terminator (\n), our previous
					// character could be \r. If it is, we remove that from the array.
					if (bytes.Num() > 0 && byte == '\n' && bytes.Top() == '\r')
					{
						bytes.Pop(false);
					}

					bytes.Add(0x0);
					break;
				}
				else
				{
					bytes.Add(byte);
				}

			} while (byte != 0x0 && (byte != '\n' || byte != '\0'));

			auto charConverted = FUTF8ToTCHAR((ANSICHAR*)bytes.GetData());
			m_readData.push_back(charConverted.Get());
		} while (dwBytesRead > 0);

		return true;
	}	

	return false;
}

TArray<FString> USerialReader::PopReadData()
{
	TArray<FString> serialData;
	for (auto data : m_readData)
	{
		serialData.Add(data);
	}

	m_readData.clear();
	return serialData;
}

