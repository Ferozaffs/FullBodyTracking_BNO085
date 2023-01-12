// Fill out your copyright notice in the Description page of Project Settings.


#include "Avatar.h"
#include <string>

// Sets default values
AAvatar::AAvatar()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

void AAvatar::SetupAvatar()
{
    USkeletalMeshComponent* skeletalMesh = GetMesh();
    poseableMesh = NewObject<UPoseableMeshComponent>(this, UPoseableMeshComponent::StaticClass());
    if (poseableMesh)
    {
        poseableMesh->RegisterComponent();
        FVector relativeLoc = GetTransform().InverseTransformPosition(skeletalMesh->GetComponentLocation());
        poseableMesh->SetWorldLocation(relativeLoc);
        poseableMesh->SetWorldRotation(FQuat::Identity);
        poseableMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        poseableMesh->SetSkeletalMesh(skeletalMesh->SkeletalMesh);
        poseableMesh->SetVisibility(true);
        skeletalMesh->SetVisibility(false);
    }
}

void AAvatar::PopulateBoneVector()
{

}

// Called when the game starts or when spawned
void AAvatar::BeginPlay()
{
    Super::BeginPlay();

    SetupAvatar();
    PopulateBoneVector();

    poseableMesh->SetBoneRotationByName("upperarm_r", FRotator::MakeFromEuler(FVector(180.0f, 0.0f, 0.0f)), EBoneSpaces::ComponentSpace);
    poseableMesh->SetBoneRotationByName("lowerarm_r", FRotator::MakeFromEuler(FVector(180.0f, 0.0f, 0.0f)), EBoneSpaces::ComponentSpace);

    m_bones.push_back(BoneData(EBoneIndex::Head, "upperarm_r"));
    m_bones.push_back(BoneData(EBoneIndex::Torso, "lowerarm_r"));
}



// Called every frame
void AAvatar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (auto& bone : m_bones)
    {
        poseableMesh->SetBoneRotationByName(bone.name, bone.orientation.Rotator(), EBoneSpaces::WorldSpace);
    }  
}

void AAvatar::ParseSensorData(TArray<FString> sensorData)
{
    for (FString data : sensorData)
    {
        std::vector<std::string> splitString;
        size_t start;
        size_t end = 0;

        std::string dataAsString = std::string(TCHAR_TO_UTF8(*data));
        while ((start = dataAsString.find_first_not_of('\t', end)) != std::string::npos)
        {
            end = dataAsString.find('\t', start);
            splitString.push_back(dataAsString.substr(start, end - start));
        }

        FQuat transformQuat = FQuat::MakeFromEuler(FVector(90.0f, 0.0f, 90.0f));

        if (splitString.size() == 5)
        {
            for (auto& bone : m_bones)
            {
                if (splitString[0].find("Sensor") != std::string::npos
                    && splitString[0].find(std::to_string(static_cast<int>(bone.index))) != std::string::npos)
                {
                    bone.orientation.W = std::stof(splitString[1].c_str());
                    bone.orientation.X = std::stof(splitString[2].c_str());
                    bone.orientation.Y = std::stof(splitString[3].c_str());
                    bone.orientation.Z = std::stof(splitString[4].c_str());

                    bone.orientation = transformQuat * bone.orientation * transformQuat.Inverse();
                    bone.orientation = bone.orientation * FQuat::MakeFromEuler(FVector(0.0f, 180.0f, 0.0f));
                }
            }
        }
    }
}