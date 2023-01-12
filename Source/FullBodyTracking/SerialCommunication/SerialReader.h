// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/UnrealString.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/MinWindows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <vector>

#include "SerialReader.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FULLBODYTRACKING_API USerialReader : public UActorComponent
{
	GENERATED_BODY()

public:	
	USerialReader();
	~USerialReader();

	UFUNCTION(BlueprintCallable, Category = "Serial")
	void Close();

	UFUNCTION(BlueprintCallable, Category = "Serial")
	bool SetupSerialRead(int port, int baudRate);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Serial")
	TArray<FString> PopReadData();

private:
	bool Read();

	HANDLE m_hComm;
	OVERLAPPED* m_overlappedRead;

	std::vector<FString> m_readData;
};
