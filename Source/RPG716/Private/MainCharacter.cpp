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

	// CameraBoom = 충돌이 있는경우 플레이어를 향해 당김
	// Create CameraBoom( pull toward thee player if there's a collison)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	GetCapsuleComponent()->SetCapsuleSize(48.f, 88.f);
		
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);//socket = 에셋을 넣을수있는 장소
	FollowCamera->bUsePawnControlRotation = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//카메라와 캐릭터 로테이션 분리
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	//캐릭터가 자동으로 이동방향에 따라 방향설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 840.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.3f;

	MaxHealth = 100.f;
	Health = 70.f;
	MaxStamina = 300.f;
	Stamina = 120.f;
	Coins = 0;

	RunningSpeed = 600.f;
	SprintSpeed = 1000.f;

	bShiftKeyDown = false;
	bLMBDown = false;
	bESCDown = false;
	bInventoryDown = false;
	bPickUpItem = false;

	// ENUMS 들 initialize 하는것
	// Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterSpeed = 15.f;
	bInterToEnemy = false;

	// Enemy healthbar HUD 시 필요
	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;

	bCanDash = true;
	DashDistance = 1200.f;
	DashCoolDown = 1.0f;
	DashStopTime = 0.7f;

}

// PickupLocation 들의 위치에 디버그 그림 넣어주기
void AMainCharacter::ShowPickUpLocation()
{
	// tarray for문 돌리는거임
	/*for (int32 i = 0; i < PickUpLocations.Num(); i++)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, PickUpLocations[i], 35.f, 12, FLinearColor::Blue, 5.f, 2.f);
	}*/

	//for문 조건연산방법 (auto 까지 사용)
	for (FVector Location : PickUpLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 35.f, 12, FLinearColor::Blue, 5.f, 2.f);
	}
}

// 키 눌리는것에 따라 sprinting 스피드로 했다가 일반 달리기 스피드로 했다가
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

	// 다른맵으로 로드시 바로 게임모드로 전환될수 있게 하기
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
	
		// stamina 상태에 따라 상태 바다르게 할때
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
			// WASD 움직임이 있어야지 Sprinting 가능하게
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
				// WASD 움직임이 있어야지 Sprinting 가능하게
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
		// forward 방향이 어느방향으로 되어있는지 확인
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
		// forward 방향이 어느방향으로 되어있는지 확인
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

// 폭탄 닿으면 health 값 줄어들게 하기
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

// coin 먹으면 코인 값 상승
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

void AMainCharacter::IncrementStamina(float Amount)
{

	if (Stamina + Amount >= MaxStamina)
	{
		Stamina = MaxStamina;
	}
	else {

		Stamina += Amount;

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
	// pausemenue 실행되어있으면 작동 안되게 하기
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	// 죽었을때는 점프가 불가능하게하기위해 재정의
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
	}
}

void AMainCharacter::LMBDown()
{
    bLMBDown = true;

	//
	UE_LOG(LogTemp, Warning, TEXT("()()("));
	// 죽으면 장비착용 비활성화 하기
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}
	// pausemenu 실행되고 있으면 작동 안되게 하기
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;


	if (ActiveOverlappingItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("%%%%%%%%%%%%%%%%"));
		//UE_LOG(LogTemp, Warning, TEXT("11111111"));
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("^^^^^^"));
			Weapon->Equip(this);
			// 마우스 왼쪽 클릭시 착용가능
			SetActiveOverlappingItem(nullptr);
			MainPlayerController->UnableItemEquipMenu();
		}
	}
	else if(EquipWeapon)
	{
		//UE_LOG(LogTemp, Warning, TEXT("22222222"));
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



// 인벤토리창 껐다 켰다 하기.
void AMainCharacter::InventoryDown()
{
	bInventoryDown = true;
	if (MainPlayerController)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Inventory Down"));
		MainPlayerController->ToggleInventoryMenu();
	}
}


void AMainCharacter::InventoryUp()
{
	bInventoryDown = false;
	
}

void AMainCharacter::PickUpItemDown()
{
	bPickUpItem = true;

}

void AMainCharacter::PickUpItemUp()
{
	bPickUpItem = false;
}

void AMainCharacter::SetEquipWeapon(AWeapon* WeaponToSet)
{
	// 기존의 weapon 을 가지고 있다가 다른 weapon 착용시 BP 에서 기존의 Weapon 사라짐
	if (EquipWeapon)
	{
		EquipWeapon->Destroy();
	}
    EquipWeapon = WeaponToSet; 
}




bool AMainCharacter::CallItemEquip(bool result, AItem* Item)
{
	bool bEquipOn = result;
	MainPlayerController->ViewItemEquipMenu();
	FVector WLocation = Item->GetActorLocation();
	MainPlayerController->WeaponLocation = WLocation;
	if (Item != nullptr) {

		UE_LOG(LogTemp, Warning, TEXT("%d dfsfwef" ), bEquipOn);
	}
	return bEquipOn;
	
}


void AMainCharacter::UnCallItemEquip()
{
	if (MainPlayerController) {
		MainPlayerController->UnableItemEquipMenu();
	}
}

void AMainCharacter::EquipOn()
{
	//CPPEquipOn();

	if (GetWeapon() != nullptr) {

		AItem* Main = Cast<AItem>(GetWeapon());
		SetActiveOverlappingItem(Main);
		LMBDown();
		
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No have Actorxxxxx"));
	}
}



void AMainCharacter::EquipSave()
{
	if (GetWeapon() != nullptr) {

		AItem* Main = Cast<AItem>(GetWeapon());
		AddItem(Main);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No have Actorxxxxx"));
	}

}

void AMainCharacter::EquipOnInThumbnail()
{

	if (GetWeapon() != nullptr) {

		AItem* Main = Cast<AItem>(GetWeapon());
		SetActiveOverlappingItem(Main);
		LMBDown();
		RemoveItemFromInventory(GetWeapon());
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No have Actorxxxxx"));
	}
}

void AMainCharacter::CallItemPotion(AItem* Item)
{
	if (MainPlayerController) {
		MainPlayerController->ViewItemPotionMenu();
		FVector WLocation = Item->GetActorLocation();
		MainPlayerController->PotionLocation = WLocation;
	}
}

void AMainCharacter::UnCallItemPotion()
{
	if (MainPlayerController) {
		MainPlayerController->UnableItemPotionMenu();
	}
}

void AMainCharacter::PotionUse(AItem* Item)
{
	GetPickUp()->Destroy();
	MainPlayerController->UnableItemPotionMenu();
}

void AMainCharacter::PotionUseInThumbnail()
{
	RemoveItemFromInventory(GetPickUp());

}

void AMainCharacter::PotionSave()
{
	if (GetPickUp() != nullptr) {

		AItem* Main = Cast<AItem>(GetPickUp());
		AddItem(Main);
		MainPlayerController->UnableItemPotionMenu();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No have Actorxxxxx"));
	}
}

// montage 만들었던것 사용시
void AMainCharacter::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		bAttacking = true;
		SetInterToEnenmy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			// 기본공격 2가지 유형 랜덤으로 번갈아가면서 나오게 하기
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

	// 원리 : 몬스터액터들을 Tarray 에 담은후 for 문을 돌면서 메인액터와 몬스터액터의 위치별로 가까운 위치에 있는걸 0번의 array 로변경하여 타겟으로 설정한다.
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

// level 변환시에 필요
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

	// 저장할 게임 속성들 값 넣어주기
	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;
	SaveGameInstance->CharacterStats.InventoryArray = Inventory;
	


	// MapName 을 가져오긴 하나 앞에 부가적인 단어들이 있음
	FString MapName = GetWorld()->GetMapName();
	// 아래 처리를 해줘야지 순수하게 MapName (LevleName) 으로 나옴
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

	// save 와 반대로 데이터들을 불러오기
	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;
	Inventory = LoadGameInstance->CharacterStats.InventoryArray;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			// 불러온 weaponName 이 map 에있는것인지 확인후에 착용시키기
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

	// 죽어서 저장한것 로드시 살아있는 상태로 로드하기
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	// 저장된 레벨이 공백이 아니라면 해당 레벨에서 로드하기 -> 공백이면 기본 맵에서 로드
	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

		Switchlevel(LevelName);
	}
}

// 레벨 변경후에도 이전 가지고 있는것들 변경 되지 않게 하기
void AMainCharacter::LoadGameNoSwitch()
{
	URPGSaveGame* LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));

	LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	// save 와 반대로 데이터들을 불러오기
	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;
	Inventory = LoadGameInstance->CharacterStats.InventoryArray;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			// 불러온 weaponName 이 map 에있는것인지 확인후에 착용시키기
			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}

		}
	}

	// 죽어서 저장한것 로드시 살아있는 상태로 로드하기
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

bool AMainCharacter::AddItem(AItem* Item)
{
	
	//Inventory.RemoveAt(0);
	Inventory.Add(Item);
	
	Item->Inventory(true);

	Item->SetActorHiddenInGame(true);
	Item->SetActorEnableCollision(false);

	//Item->Destroy();
	

	for (AItem* PickUp : Inventory) {
		UE_LOG(LogTemp, Warning, TEXT("Item add : %s"), *PickUp->GetName());
	}

	// 비교 하는중에 getname() e

	UE_LOG(LogTemp, Warning, TEXT("END OF ITEMS"));
	
	return false;
}

// 인벤토리에 있는 아이템 모두 내놓게 하기
void AMainCharacter::DropAllInventory()
{
	for (AItem* Pickup : Inventory) {

		DropItem(Pickup);
	}
	Inventory.Empty();
}

void AMainCharacter::DropItem(AItem* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("Drop!!!"));

	FVector Location = GetOwner()->GetActorLocation();
	Location.X += FMath::FRandRange(-50.0f, 100.0f);
	Location.Y += FMath::FRandRange(-50.0f, 100.0f);

	// { 여기서 부터 다음번 가로까지는 높은 모서리같은곳에서 죽었을시 가지고있는 아이템 공중에 계속 띄우지 않고 떨어트리게 하는것.
	FVector EndRay = Location;
	EndRay.Z -= 500.0f;

	FHitResult HitResult;
	FCollisionObjectQueryParams ObjQuery;
	FCollisionQueryParams CollsionParams;
	CollsionParams.AddIgnoredActor(GetOwner());

	GetWorld()->LineTraceSingleByObjectType(

		OUT HitResult,
		Location,
		EndRay,
		ObjQuery,
		CollsionParams

	);

	if (HitResult.ImpactPoint != FVector::ZeroVector) {
		Location = HitResult.ImpactPoint;
	}
	// }
	Item->SetActorLocation(Location);
	// 그리고 인벤토리에서 나온 아이템은 픽업 안되게 해둠
	Item->Inventory(false);

	RemoveItemFromInventory(Item);
}

// 아이템을 가지고 있으면서 드롭버튼을 누르면 아이템 삭제
bool AMainCharacter::RemoveItemFromInventory(AItem* Item)
{

	int32 Counter = 0;
	for (AItem* Pickup : Inventory)
	{
		if (Pickup == Item) {

			Inventory.RemoveAt(Counter);
			return true;

		}
		++Counter;
	}
	return false;
}

TArray<class AItem*> AMainCharacter::GetInventoryItems()
{
	return Inventory;
}

int32 AMainCharacter::GetCurrentInventoryCount()
{
	return  Inventory.Num() - 1;
}

bool AMainCharacter::CheckIfClientHasItem(AItem* Item)
{
	for (AItem* Pickup : Inventory)
	{
		if (Pickup == Item) {

			return true;

		}
	}
	return false;
}
