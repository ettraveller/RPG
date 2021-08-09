// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill.h"
#include "SkillComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPG716_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkillComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:
	/*The Available Skill Points which can be spent in total*/
	UPROPERTY(EditDefaultsOnly)
	int32 AvailableSkillPoints;
	/*Returns the new level of the skill*/
	
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	int32 AdvanceSkillLevel_Fire(ASkill* SkillToLevelUp);
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	int32 AdvanceSkillLevel_Posion(ASkill* SkillToLevelUp);

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class ASkill* Skill2;

	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
		ASkill* ReturnSkill() { return Skill2; }*/

	/*Resets the skill points and unlearns all the skills*/
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	void ResetSkillPoints();

	/*An array which contains all the available skills*/
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<ASkill>> SkillsArray;

	/*Returns the texture of the given skill's index (searches SkillsArray)*/
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	UTexture* GetSkillTexture(int32 SkillNum);

	
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	int32 GetSkillLevel_Fire(int32 SkillNum);
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	int32 GetSkillLevel_Posision(int32 SkillNum);

	/*Returns the first matching skill from SkillsArray*/
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	ASkill* GetSkillByType(ESkillType SkillType);

	/*The amount of available skill points when starting the game*/
	UPROPERTY(EditDefaultsOnly)
	int32 InitialAvailableSkillsPoints;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
