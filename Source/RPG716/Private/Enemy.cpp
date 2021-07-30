// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include <Components/SphereComponent.h>
#include <AIController.h>
#include "MainCharacter.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Components/BoxComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Engine/SkeletalMeshSocket.h>
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"
#include <GameFramework/Character.h>


// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(700.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(100.f);
	
	// 몬스터 팔에 (특정부위) 에 collision 설치시
	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));
    //컴포넌트를 다른 컴포넌트나 소켓에 붙힐때 사용
	//근데 오류 무엇? 뭔지모르겠다..
	//CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("BearSocket"));

	bOverlappingCombatSphere = false;

	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 30.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	bHasValidTarget = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);
	//AgroSphere 영역일때는 폭발물에 닿아도 터지지 않게 하기
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	//Mesh 와 캡슐컴포넌트는 카메라와 충돌없게 하기
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// 몬스터 지역에 있을시
void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if(OtherActor && Alive())
	{

	    AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			MoveToTarget(Main);
		}
	}
    
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 공격상태가 아닐시 일반 몬스터 지역에 입장시 공격하러 오다가 일정 반경 이상이면 움직임 멈춤
	if (OtherActor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			bHasValidTarget = false;
			if (Main->CombatTarget == this)
			{
				Main->SetCombatTarget(nullptr);
			}
			Main->setHasCombatTarget(false);

			Main->UpdateCombatTarget();
			
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController)
			{
				AIController->StopMovement();
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		// 몬스터가 공격하게 만들기 (SpiderAnim_BP 의 Attacking)
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		{
		if (Main)
		{
			bHasValidTarget = true;
			Main->SetCombatTarget(this);
			Main->setHasCombatTarget(true);
			
			Main->UpdateCombatTarget();

		    CombatTarget = Main;
		    bOverlappingCombatSphere = true;
			
			float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
			
			//Attack();
		    //SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			//attacking animation구현은 블루프린트에서
			
		}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 몬스터의 반경에서 멀어지면 몬스터가 캐릭터 쫒아오게 하기
	if (OtherActor && OtherComp)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			/*SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
			MoveToTarget(Main);*/
			// 전투상태인지를 먼저 확인
			
			bOverlappingCombatSphere = false;
			MoveToTarget(Main);
			CombatTarget = nullptr;

			if (Main->CombatTarget == this)
			{
				Main->SetCombatTarget(nullptr);
				Main->bHasCombatTarget = false;
				Main->UpdateCombatTarget();

			}

			// 몬스터 액터의 healthbar 없애기
			if (Main->MainPlayerController)
			{
				USkeletalMeshComponent* MainMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if (MainMesh)
				{
					Main->MainPlayerController->RemoveEnemyHealthBar();
				}
			}

			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

//cs파일에 AIModule 추가해야댐 target 따라가게 만들기
void AEnemy::MoveToTarget(class AMainCharacter* Target)
{
    SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MoveToTarget()"));
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		// 충돌 반경 설정시
		MoveRequest.SetAcceptanceRadius(30.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		//캐릭터 이동경로에 포인트를 찍어서 선으로 연결하여 이동하기때문에 몬스터가 모서리부분을 부드럽게 돌수있다.
		// 
		//  CRASH!!!  NavMesh의 범위를 넘어가서 collision이벤트가 발생되고 pathpoint 를 찾을라그러면 스마트포인터 오류가난다.
		// 해결
		// 
		//TArray<FNavPathPoint> PathPoint = NavPath->GetPathPoints();

		/*auto PathPoint = NavPath->GetPathPoints();
		for (auto Point : PathPoint)
		{
			FVector Location = Point.Location;
			UKismetSystemLibrary::DrawDebugSphere(this, Location ,25.f, 8, FLinearColor::Blue, 10.f, 0.5f);
		}*/

	}
}
void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			if (Main->HitParticle)
			{
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket)
				{
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticle, SocketLocation, FRotator::ZeroRotator, false);
				}

			}
			if (Main->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (DamageTypeClass)
			{
				// 공격 하면 캐릭터가 피 닳게 하기
				UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
			}
		}

	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    
}

void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UGameplayStatics::PlaySound2D(this, SwingSound);

}

void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::Attack()
{
	if (Alive() && bHasValidTarget)
	{
		if (AIController)
		{
			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking)
		{
			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(CombatMontage, 1.2f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);

			}
	
		} 
	}
    
}

void AEnemy::AttackEnd()
{
    bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		// 적의 공격시간을 설정함으로써, 랜덤시간으로 공격이 나옴 -> 도망가거나 공격을 할수있게
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
		//Attack();
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f) {
		Health -= DamageAmount;
		Die(DamageCauser);

	}
	else {
		Health -= DamageAmount;
	}

	return DamageAmount;

}

void AEnemy::Die(AActor* Causer)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.2f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);

	}
	

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	AMainCharacter* Main = Cast<AMainCharacter>(Causer);
	if (Main)
	{
		Main->UpdateCombatTarget();
	}

}

void AEnemy::DeathEnd()
{
	// 죽으면 애니메이션 과 스켈레탈 업데이트 막기
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear()
{
	Destroy();
}
