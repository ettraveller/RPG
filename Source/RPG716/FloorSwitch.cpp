// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorSwitch.h"
#include <Components/BoxComponent.h>
#include <Components/StaticMeshComponent.h>
#include "TimerManager.h"


// Sets default values
AFloorSwitch::AFloorSwitch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 물체를 인식할 충돌체 Trigger 설정 
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
	//collision 설정 (다른거 무시하고 pawn만 오버랩)
	//collision preset 설정하는거
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	TriggerBox->SetBoxExtent(FVector(62.f, 62.f, 32.f));

	// 바닥 mesh 설정
	FloorSwitch = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorSwitch"));
	FloorSwitch->SetupAttachment(GetRootComponent());
	
	// 문 mesh 설정
	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(GetRootComponent());

	SwitchTime = 1.f;
	bCharacterOnSwitch = false;
}

void AFloorSwitch::CloseDoor()
{
    if(!bCharacterOnSwitch)
	{
	    LowerDoor();
	    RaiseFloorSwitch();
	}
}

// Called when the game starts or when spawned
void AFloorSwitch::BeginPlay()
{
	Super::BeginPlay();

	//AddDynamic = 함수를 오버랩이벤트에 바인딩하는것
	//까보면 델리게이트를 사용하여 구성댐
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapEnd);

	InitialDoorLocation = Door->GetComponentLocation();
	InitialSwitchLocation = FloorSwitch->GetComponentLocation();

}

// Called every frame
void AFloorSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// trigger 에 닿았을때 사용될 함수
void AFloorSwitch::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("Begin OverLap"));
	if(!bCharacterOnSwitch) bCharacterOnSwitch = true;
	RaiseDoor();
	LowerFloorSwitch();
}

// trigger 에 닿았다가 안닿게 되면 사용될 함수
void AFloorSwitch::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("End OverLap"));
	if(bCharacterOnSwitch) bCharacterOnSwitch = false;

	// timer 로 문여는함수를 조정하여, 캐릭터가 잠깐 floor 에서 벗어나도 문이 열려있게 하기 위해서
	GetWorldTimerManager().SetTimer(SwitchHandle, this, &AFloorSwitch::CloseDoor, SwitchTime);

}

// 실질적으로 문의 위치 값 조정하게 하는것
void AFloorSwitch::UpdateDoorLocation(float Z)
{
    FVector NewLocation = InitialDoorLocation;
	NewLocation.Z += Z;
	Door-> SetWorldLocation(NewLocation);
}

// 실질적으로 floor 의 위치값 조정하게 하는것
void AFloorSwitch::UpdateFloorSwitchLocation(float Z)
{
	FVector NewLocation = InitialSwitchLocation;
	NewLocation.Z += Z;
	FloorSwitch->SetWorldLocation(NewLocation);
}

