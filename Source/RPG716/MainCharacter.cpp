// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include <Math/RotationMatrix.h>
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include <Kismet/KismetSystemLibrary.h>
#include "Weapon.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include <Animation/AnimMontage.h>
#include "Sound/SoundCue.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "Enemy.h"
#include "MainPlayerController.h"
#include <GameFramework/Character.h>
#include "RPGSaveGame.h"
#include "ItemStorage.h"


// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// CameraBoom = 충돌???�는경우 ?�레?�어�??�해 ?��?
	// Create CameraBoom( pull toward thee player if there's a collison)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	GetCapsuleComponent()->SetCapsuleSize(48.f, 88.f);
		
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);//socket = ?�셋???�을?�있???�소
	FollowCamera->bUsePawnControlRotation = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//카메?��? 캐릭??로테?�션 분리
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	//캐릭?��? ?�동?�로 ?�동방향???�라 방향?�정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 840.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.3f;

	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 300.f;
	Stamina = 120.f;
	Coins = 0;

	RunningSpeed = 600.f;
	SprintSpeed = 1000.f;

	bShiftKeyDown = false;
	bLMBDown = false;
	bESCDown = false;
	bInventoryDown = false;

	// ENUMS ??initialize ?�는�?
	// Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterSpeed = 15.f;
	bInterToEnemy = false;

	// Enemy healthbar HUD ???�요
	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;

	bCanDash = true;
	DashDistance = 1200.f;
	DashCoolDown = 1.0f;
	DashStopTime = 0.7f;

}

// PickupLocation ?�의 ?�치???�버�?그림 ?�어주기
void AMainCharacter::ShowPickUpLocation()
{
// tarray for�??�리?�거??
	/*for (int32 i = 0; i < PickUpLocations.Num(); i++)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, PickUpLocations[i], 35.f, 12, FLinearColor::Blue, 5.f, 2.f);
	}*/

	//for�?조건?�산방법 (auto 까�? ?�용)
	for (FVector Location : PickUpLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 35.f, 12, FLinearColor::Blue, 5.f, 2.f);
	}
}

// ???�리?�것???�라 sprinting ?�피?�로 ?�다가 ?�반 ?�리�??�피?�로 ?�다가
void AMainCharacter::SetMovementStatus(EMovementStatus Status)
{
    MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMainCharacter::ShiftKeyDown()
{
    bShiftKeyDown = true;
}

void AMainCharacter::ShiftKeyUp()
{
    bShiftKeyDown = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	UKismetSystemLibrary::DrawDebugSphere(this, GetActorLocation() + FVector(0.f, 0.f, 75.f), 35.f, 12, FLinearColor::Blue, 5.f, 2.f);

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	LoadGameNoSwitch();

	// ?�른맵으�?로드??바로 게임모드�??�환?�수 ?�게 ?�기
	if (MainPlayerController)
	{
		MainPlayerController->GameModeOnly();
	}
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}
    
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	
		// stamina ?�태???�라 ?�태 바다르게 ?�때
	case EStaminaStatus::ESS_Normal:
		//shiftKey Down
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
		        SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
			    Stamina -= DeltaStamina;
			}
			// WASD ?�직임???�어?��? Sprinting 가?�하�?
			if (bMovingForward || bMovingRight)
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else {
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		//shiftKeyUp
		else
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
			    Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_BelowMinimum:
		//shiftKey Down
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				// WASD ?�직임???�어?��? Sprinting 가?�하�?
				if (bMovingForward || bMovingRight)
				{
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else {
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
			
		}
		//shiftKeyUp
		else
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
			    SetMovementStatus(EMovementStatus::EMS_Normal);
				Stamina += DeltaStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_Exhausted:
		//shiftKey Down
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		//shiftKeyUp
		else
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	case EStaminaStatus::ESS_ExhaustedRecovering:
		//shiftKey Down
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		//shiftKeyUp
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	default:
		break;
	}

	if (bInterToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotaionYaw(CombatTarget->GetActorLocation());
		FRotator InterRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterSpeed);
		
		SetActorRotation(InterRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AMainCharacter::StopJumping);

	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AMainCharacter::ShiftKeyUp);

	PlayerInputComponent->BindAction(TEXT("LMB"), IE_Pressed, this, &AMainCharacter::LMBDown);
	PlayerInputComponent->BindAction(TEXT("LMB"), IE_Released, this, &AMainCharacter::LMBUp);

	PlayerInputComponent->BindAction(TEXT("ESC"), IE_Pressed, this, &AMainCharacter::ESCDown);
	PlayerInputComponent->BindAction(TEXT("ESC"), IE_Released, this, &AMainCharacter::ESCUp);


	PlayerInputComponent->BindAction(TEXT("Inventory"), IE_Pressed, this, &AMainCharacter::InventoryDown);
	PlayerInputComponent->BindAction(TEXT("Inventory"), IE_Released, this, &AMainCharacter::InventoryUp);


	PlayerInputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &AMainCharacter::Dash);
	

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &AMainCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AMainCharacter::LookUp);


}

bool AMainCharacter::CanMove(float Value)
{

	if (MainPlayerController)
	{
		return (Value != 0.0) && (!bAttacking) &&
			(MovementStatus != EMovementStatus::EMS_Dead) &&
			(!MainPlayerController->bPauseMenuVisible);
	}
	return false;
			
}

void AMainCharacter::MoveForward(float Value)
{
	bMovingForward = false;
	if (CanMove(Value))
	{
		// forward 방향???�느방향?�로 ?�어?�는지 ?�인
	    const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}
}

void AMainCharacter::MoveRight(float Value)
{
	bMovingRight = false;
	if (CanMove(Value))
	{
		// forward 방향???�느방향?�로 ?�어?�는지 ?�인
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;
	}
}


void AMainCharacter::Turn(float Value)
{
	if (CanMove(Value))
	{
		AddControllerYawInput(Value);
	}

}


void AMainCharacter::LookUp(float Value)
{
	if (CanMove(Value))
	{
		AddControllerPitchInput(Value);
	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
   AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void AMainCharacter::Dash()
{
    
	if (bCanDash && (Stamina >= 30))
	{
	    GetCharacterMovement()->BrakingFrictionFactor = 0.f;
		FVector DashLocation = FollowCamera->GetForwardVector();
		LaunchCharacter(FVector(DashLocation.X, DashLocation.Y, 0).GetSafeNormal() * DashDistance, true, true);
		Stamina -= 30.f;
		bCanDash = false;
		GetWorldTimerManager().SetTimer(DashTimer, this, &AMainCharacter::StopDash, DashStopTime, false);
	}

}


void AMainCharacter::StopDash()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(DashTimer, this, &AMainCharacter::ResetDash, DashCoolDown, false);
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
}


void AMainCharacter::ResetDash()
{
    bCanDash = true;
}

// ??�� ?�으�?health �?줄어?�게 ?�기
void AMainCharacter::DecrementHealth(float Amount)
{
	if (Health - Amount <= 0.f)
	{
	    Health -= Amount;
	    Die();
	}
	else
	{
		Health -= Amount;
	}
}



float AMainCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f) {
		Health -= DamageAmount;
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy) {
				Enemy->bHasValidTarget = false;
			}
		}

	}
	else {
		Health -= DamageAmount;
	}
	
	return DamageAmount;
}

// coin 먹으�?코인 �??�승
void AMainCharacter::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}


void AMainCharacter::IncrementHealth(float Amount)
{

	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else {
		
		Health += Amount;

	}

}

void AMainCharacter::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) {

		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}


void AMainCharacter::Jump()
{
	// pausemenue ?�행?�어?�으�??�동 ?�되�??�기
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	// 죽었?�때???�프가 불�??�하게하기위???�정??
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
	}
}

void AMainCharacter::LMBDown()
{
    bLMBDown = true;

	// 죽으�??�비착용 비활?�화 ?�기
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}
	// pausemenu ?�행?�고 ?�으�??�동 ?�되�??�기
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (ActiveOverlappingItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("11111111"));
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			// 마우???�쪽 ?�릭??착용가??
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if(EquipWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("22222222"));
	    Attack();
	}
}

void AMainCharacter::LMBUp()
{
    bLMBDown = false;
}


void AMainCharacter::ESCDown()
{
	bESCDown = true;
	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}


void AMainCharacter::ESCUp()
{
	bESCDown = false;
}


<<<<<<< Updated upstream
// ?�벤?�리�?껐다 켰다 ?�기.
=======
// �κ��丮â ���� �״� �ϱ�.
>>>>>>> Stashed changes
void AMainCharacter::InventoryDown()
{
	bInventoryDown = true;
	if (MainPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory Down"));
		MainPlayerController->ToggleInventoryMenu();
	}
}


void AMainCharacter::InventoryUp()
{
	bInventoryDown = false;
	
}

void AMainCharacter::SetEquipWeapon(AWeapon* WeaponToSet)
{
	// 기존??weapon ??가지�??�다가 ?�른 weapon 착용??BP ?�서 기존??Weapon ?�라�?
	if (EquipWeapon)
	{
		EquipWeapon->Destroy();
	}
    EquipWeapon = WeaponToSet; 
}

// montage 만들?�던�??�용??
void AMainCharacter::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		bAttacking = true;
		SetInterToEnenmy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			// 기본공격 2가지 ?�형 ?�덤?�로 번갈?��?면서 ?�오�??�기
			 int32 Section = FMath::RandRange(0, 1);
			 switch (Section)
			 {
			 case 0:
				 AnimInstance->Montage_Play(CombatMontage, 2.2f);
				 AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				 UE_LOG(LogTemp, Warning, TEXT("Case 0"));
				 break;

			 case 1:
				 AnimInstance->Montage_Play(CombatMontage, 1.8f);
				 AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				 UE_LOG(LogTemp, Warning, TEXT("Case 1"));
				 break;

			 default:
				 break;
			 }
			 /*AnimInstance->Montage_Play(CombatMontage, 1.35f);
			 AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);*/
		}
	}  
	if (EquipWeapon->SwingSound)
	{
	    //UGameplayStatics::PlaySound2D(this, EquipWeapon->SwingSound);
	}
}

void AMainCharacter::AttackEnd()
{
    bAttacking = false;
	SetInterToEnenmy(false);
	if (bLMBDown)
	{
		Attack();
	}
}

void AMainCharacter::PlaySwingSound()
{
    if(EquipWeapon->SwingSound)
	{
	     UGameplayStatics::PlaySound2D(this,EquipWeapon->SwingSound);
	}
}


void AMainCharacter::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}


void AMainCharacter::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	// ?�리 : 몬스?�액?�들??Tarray ???��???for 문을 ?�면??메인?�터?� 몬스?�액?�의 ?�치별로 가까운 ?�치???�는�?0번의 array 로�?경하???�겟으�??�정?�다.
	if (OverlappingActors.Num() == 0) return;
	AEnemy* CloasetEnemy = Cast<AEnemy>(OverlappingActors[0]);

	if (OverlappingActors.Num() == 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	if (CloasetEnemy)
	{
		FVector Location = GetActorLocation();
		float MinDistance = (CloasetEnemy->GetActorLocation() - Location).Size();
		for (auto Actor: OverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					CloasetEnemy = Enemy;
				}
			}
		}
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(CloasetEnemy);
		bHasCombatTarget = true;
	}
}

void AMainCharacter::SetInterToEnenmy(bool Interp)
{
	bInterToEnemy = Interp;
}

FRotator AMainCharacter::GetLookAtRotaionYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// level 변?�시???�요
void AMainCharacter::Switchlevel(FName LevelName)
{
	UWorld* World = GetWorld();

	if (World)
	{
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMainCharacter::SaveGame()
{
	URPGSaveGame* SaveGameInstance = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));

	// ?�?�할 게임 ?�성??�??�어주기
	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	// MapName ??가?�오�??�나 ?�에 부가?�인 ?�어?�이 ?�음
	FString MapName = GetWorld()->GetMapName();
	// ?�래 처리�??�줘?��? ?�수?�게 MapName (LevleName) ?�로 ?�옴
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;
	

	if (EquipWeapon)
	{
		SaveGameInstance->CharacterStats.WeaponName = EquipWeapon->Name;
	}

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMainCharacter::LoadGame(bool SetPosition)
{
	URPGSaveGame* LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));

	LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	// save ?� 반�?�??�이?�들??불러?�기
	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

<<<<<<< Updated upstream
			// 불러??weaponName ??map ?�있?�것?��? ?�인?�에 착용?�키�?
=======
			// �ҷ��� weaponName �� map ���ִ°����� Ȯ���Ŀ� �����Ű��
>>>>>>> Stashed changes
			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}

		}
	}

	if (SetPosition) {
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	// 죽어???�?�한�?로드???�아?�는 ?�태�?로드?�기
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	// ?�?�된 ?�벨??공백???�니?�면 ?�당 ?�벨?�서 로드?�기 -> 공백?�면 기본 맵에??로드
	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

		Switchlevel(LevelName);
	}
}

// ?�벨 변경후?�도 ?�전 가지�??�는것들 변�??��? ?�게 ?�기
void AMainCharacter::LoadGameNoSwitch()
{
	URPGSaveGame* LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));

	LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	// save ?� 반�?�??�이?�들??불러?�기
	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			// 불러??weaponName ??map ?�있?�것?��? ?�인?�에 착용?�키�?
			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}

		}
	}

	// 죽어???�?�한�?로드???�아?�는 ?�태�?로드?�기
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}


