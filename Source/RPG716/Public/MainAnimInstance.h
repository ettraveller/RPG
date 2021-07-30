// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class RPG716_API UMainAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
    //similar to beginplay
	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable, Category = "AnimationProperties")
    void UpDateAnimationProerties();
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class APawn* Pawn;

	// sprinting ¿¡ »ç¿ëµÊ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class AMainCharacter* Main;
};
