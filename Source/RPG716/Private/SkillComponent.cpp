// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"
#include "Skill.h"

// Sets default values for this component's properties
USkillComponent::USkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	InitialAvailableSkillsPoints = 7;
}


// Called when the game starts
void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	//Reseting the level of each skill
	for (auto Skill : SkillsArray)
	Skill->GetDefaultObject<ASkill>()->ResetLevel();

	AvailableSkillPoints = InitialAvailableSkillsPoints;
	
}


UTexture* USkillComponent::GetSkillTexture(int32 SkillNum)
{
	UE_LOG(LogTemp, Warning, TEXT("Image232323323323"));
	if (SkillsArray.IsValidIndex(SkillNum))
	{
		UE_LOG(LogTemp, Warning, TEXT("Image"));
		return SkillsArray[SkillNum]->GetDefaultObject<ASkill>()->GetSkillTexture();
	}
	UE_LOG(LogTemp, Warning, TEXT("NoImage"));
	return nullptr;
}

int32 USkillComponent::GetSkillLevel_Fire(int32 SkillNum)
{
	if (SkillsArray.IsValidIndex(SkillNum))
	{
		UE_LOG(LogTemp, Warning, TEXT("Getskill"));
		return SkillsArray[SkillNum]->GetDefaultObject<ASkill>()->GetLevel_Fire();
	}
	return 0;
}
int32 USkillComponent::GetSkillLevel_Posision(int32 SkillNum)
{
	if (SkillsArray.IsValidIndex(SkillNum))
	{
		UE_LOG(LogTemp, Warning, TEXT("Getskill"));
		return SkillsArray[SkillNum]->GetDefaultObject<ASkill>()->GetLevel_Poision();
	}
	return 0;
}
ASkill* USkillComponent::GetSkillByType(ESkillType SkillType)
{
	for (auto It : SkillsArray)
	{
		ASkill* Skill = It->GetDefaultObject<ASkill>();
		if (Skill->GetSkillType() == SkillType) return Skill;
	}
	return nullptr;
}

int32 USkillComponent::AdvanceSkillLevel_Fire(ASkill* SkillToLevelUp)
{
	UE_LOG(LogTemp, Warning, TEXT("advan_F1"));

	if (SkillToLevelUp && AvailableSkillPoints > 0 && !SkillToLevelUp->IsMaxLevel_Fire())
	{
		UE_LOG(LogTemp, Warning, TEXT("advan_F2"));
		AvailableSkillPoints--;
		SkillToLevelUp->AdvanceLevel_Fire();
		return SkillToLevelUp->GetLevel_Fire();

	}
	else if (SkillToLevelUp) return SkillToLevelUp->GetLevel_Fire();
	else return 0;
}
int32 USkillComponent::AdvanceSkillLevel_Posion(ASkill* SkillToLevelUp)
{
	UE_LOG(LogTemp, Warning, TEXT("advan_P1"));

	if (SkillToLevelUp && AvailableSkillPoints > 0 && !SkillToLevelUp->IsMaxLevel_Poision())
	{
		UE_LOG(LogTemp, Warning, TEXT("advan_P2"));
		AvailableSkillPoints--;
		SkillToLevelUp->AdvanceLevel_Poision();
		return SkillToLevelUp->GetLevel_Poision();

	}
	else if (SkillToLevelUp) return SkillToLevelUp->GetLevel_Poision();
	else return 0;
}

void USkillComponent::ResetSkillPoints()
{
	AvailableSkillPoints = InitialAvailableSkillsPoints;
	for (auto It : SkillsArray)
	{
	    UE_LOG(LogTemp, Warning, TEXT("Reset"));
		It->GetDefaultObject<ASkill>()->ResetLevel();
	}
}

// Called every frame
void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

