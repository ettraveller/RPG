// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SkillDataTable.generated.h"


USTRUCT(BlueprintType)
struct FLevelUpTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillLevelUp")
	int32 CurrentLevel_FireBall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillLevelUp")
	int32 CurrentLevel_PoisionBall;
};


UCLASS()
class RPG716_API ASkillDataTable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkillDataTable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
