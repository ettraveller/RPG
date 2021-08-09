// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "PickUp.generated.h"

/**
 * 
 */
UCLASS()
class RPG716_API APickUp : public AItem
{
	GENERATED_BODY()


public:
    APickUp();

	// 동전은 BP_Coins 에서 증가되게 해둠
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins")
	int32 CoinCount;*/

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnPickupBP(class AMainCharacter* Target);
	
};
