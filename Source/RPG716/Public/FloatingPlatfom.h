// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingPlatfom.generated.h"

UCLASS()
class RPG716_API AFloatingPlatfom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloatingPlatfom();

    //Mesh for platform
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	FVector StartPoint;

	UPROPERTY(EditAnywhere, meta = (MakeEditWidget = "true"))
	FVector EndPoint;

	// 움직일때 이동간의 스피드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	float InterporSpeed;

	// 이동하고 다시 제자리에 올 타임
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	float InterporTime;

	FTimerHandle InteporTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
	bool bInterping;

	float Distance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ToggleInterping();

	//Swaping
	void SwapVectors(FVector& Vec1, FVector& Vec2);
};
