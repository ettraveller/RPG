// Fill out your copyright notice in the Description page of Project Settings.


#include "SpanwVolume.h"
#include <Components/BoxComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include "Engine/World.h"
#include "Critter.h"
#include "Enemy.h"
#include <AIController.h>


// Sets default values
ASpanwVolume::ASpanwVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spanwing Box"));


}

// Called when the game starts or when spawned
void ASpanwVolume::BeginPlay()
{
	Super::BeginPlay();
	
	if (Actor_1 && Actor_2 && Actor_3 && Actor_4)
	{
		SpawnArray.Add(Actor_1);
		SpawnArray.Add(Actor_2);
		SpawnArray.Add(Actor_3);
		SpawnArray.Add(Actor_4);
	}

	
}

// Called every frame
void ASpanwVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// spawn 될 액터가 생성될 위치 박스 생성 하기
FVector ASpanwVolume::GetSpawnPoint()
{
    FVector Extent = SpawnBox->GetScaledBoxExtent();
	FVector Origin = SpawnBox->GetComponentLocation();

	FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);

	return Point;
} 

// spawn 할 액터 추가하기 ( 몬스터)
void ASpanwVolume::SpawnOurActor_Implementation(UClass* ToSpawn, const FVector& Location)
{
	if (ToSpawn)
	{
		UWorld* World = GetWorld();

		FActorSpawnParameters SpawnParams;

		if (World)
		{
			AActor* Actor = World->SpawnActor<AActor>(ToSpawn, Location, FRotator(0.f), SpawnParams);

			// Enemy 인건 확인해서 Eenmy 컨트롤러 인식하게 하기
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				Enemy->SpawnDefaultController();

				AAIController* AICont = Cast<AAIController>(Enemy->GetController());
				if (AICont)
				{
					Enemy->AIController = AICont;
				}
			}
		}
	}
}


TSubclassOf<AActor> ASpanwVolume::GetSpawnActor()
{
	// begin 에서 추가한것 4가지
	if (SpawnArray.Num() > 0)
	{
		int32 Selection = FMath::RandRange(0, SpawnArray.Num() - 1);

		return SpawnArray[Selection];
	}
	else {
		return nullptr;
	}
}

