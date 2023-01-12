// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/PoseableMeshComponent.h"

#include <vector>

#include "Avatar.generated.h"

UCLASS()
class FULLBODYTRACKING_API AAvatar : public ACharacter
{
	GENERATED_BODY()

public:
	enum EBoneIndex
	{
		Head		= 0,
		Torso		= 1,
		Arm_Upper_Left,
		Arm_Upper_Right,
		Arm_Lower_Left,
		Arm_Lower_Right,
		Hand_Left,
		Hand_Right,
		Hip,
		Leg_Upper_Left,
		Leg_Upper_Right,
		Leg_Lower_Left,
		Leg_Lower_Right,
		Foot_Left,
		Foot_Right,

		Invalid
	};

	AAvatar();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Avatar")
	void ParseSensorData(TArray<FString> sensorData);

protected:
	virtual void BeginPlay() override;

private:

	struct BoneData
	{
		EBoneIndex index;
		FName name;

		FQuat orientation;

		BoneData(EBoneIndex inIndex, FName inName)
			: index(inIndex)
			, name(inName){}
	};

	void SetupAvatar();
	void PopulateBoneVector();

	UPoseableMeshComponent* poseableMesh;

	std::vector<BoneData> m_bones;


};
