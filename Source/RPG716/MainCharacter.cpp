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

	// CameraBoom = ì¶©ë???ëê²½ì° ?ë ?´ì´ë¥??¥í´ ?¹ê?
	// Create CameraBoom( pull toward thee player if there's a collison)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	GetCapsuleComponent()->SetCapsuleSize(48.f, 88.f);
		
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);//socket = ?ì???£ì?ì???¥ì
	FollowCamera->bUsePawnControlRotation = false;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//ì¹´ë©?¼ì? ìºë¦­??ë¡í?´ì ë¶ë¦¬
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	//ìºë¦­?°ê? ?ë?¼ë¡ ?´ëë°©í¥???°ë¼ ë°©í¥?¤ì 
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

	// ENUMS ??initialize ?ëê²?
	// Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterSpeed = 15.f;
	bInterToEnemy = false;

	// Enemy healthbar HUD ???ì
	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;

	bCanDash = true;
	DashDistance = 1200.f;
	DashCoolDown = 1.0f;
	DashStopTime = 0.7f;

}

// PickupLocation ?¤ì ?ì¹???ë²ê·?ê·¸ë¦¼ ?£ì´ì£¼ê¸°
void AMainCharacter::ShowPickUpLocation()
{
// tarray forë¬??ë¦¬?ê±°??
	/*for (int32 i = 0; i < PickUpLocations.Num(); i++)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, PickUpLocations[i], 35.f, 12, FLinearColor::Blue, 5.f, 2.f);
	}*/

	//forë¬?ì¡°ê±´?°ì°ë°©ë² (auto ê¹ì? ?¬ì©)
	for (FVector Location : PickUpLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 35.f, 12, FLinearColor::Blue, 5.f, 2.f);
	}
}

// ???ë¦¬?ê²???°ë¼ sprinting ?¤í¼?ë¡ ?ë¤ê° ?¼ë° ?¬ë¦¬ê¸??¤í¼?ë¡ ?ë¤ê°
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

	// ?¤ë¥¸ë§µì¼ë¡?ë¡ë??ë°ë¡ ê²ìëª¨ëë¡??í? ì ?ê² ?ê¸°
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
	
		// stamina ?í???°ë¼ ?í ë°ë¤ë¥´ê² ? ë
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
			// WASD ?ì§ì???ì´?¼ì? Sprinting ê°?¥íê²?
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
				// WASD ?ì§ì???ì´?¼ì? Sprinting ê°?¥íê²?
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
		// forward ë°©í¥???´ëë°©í¥?¼ë¡ ?ì´?ëì§ ?ì¸
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
		// forward ë°©í¥???´ëë°©í¥?¼ë¡ ?ì´?ëì§ ?ì¸
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

// ?? ?¿ì¼ë©?health ê°?ì¤ì´?¤ê² ?ê¸°
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

// coin ë¨¹ì¼ë©?ì½ì¸ ê°??ì¹
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
	// pausemenue ?¤í?ì´?ì¼ë©??ë ?ëê²??ê¸°
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	// ì£½ì?ë???íê° ë¶ê??¥íê²íê¸°ì???¬ì ??
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
	}
}

void AMainCharacter::LMBDown()
{
    bLMBDown = true;

	// ì£½ì¼ë©??¥ë¹ì°©ì© ë¹í?±í ?ê¸°
	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}
	// pausemenu ?¤í?ê³  ?ì¼ë©??ë ?ëê²??ê¸°
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (ActiveOverlappingItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("11111111"));
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			// ë§ì°???¼ìª½ ?´ë¦­??ì°©ì©ê°??
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
// ?¸ë²¤? ë¦¬ì°?ê»ë¤ ì¼°ë¤ ?ê¸°.
=======
// ÀÎº¥Åä¸®Ã¢ ²°´Ù Ä×´Ù ÇÏ±â.
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
	// ê¸°ì¡´??weapon ??ê°ì§ê³??ë¤ê° ?¤ë¥¸ weapon ì°©ì©??BP ?ì ê¸°ì¡´??Weapon ?¬ë¼ì§?
	if (EquipWeapon)
	{
		EquipWeapon->Destroy();
	}
    EquipWeapon = WeaponToSet; 
}

// montage ë§ë¤?ëê²??¬ì©??
void AMainCharacter::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead)
	{
		bAttacking = true;
		SetInterToEnenmy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			// ê¸°ë³¸ê³µê²© 2ê°ì§ ? í ?ë¤?¼ë¡ ë²ê°?ê?ë©´ì ?ì¤ê²??ê¸°
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

	// ?ë¦¬ : ëª¬ì¤?°ì¡?°ë¤??Tarray ???´ì???for ë¬¸ì ?ë©´??ë©ì¸?¡í°? ëª¬ì¤?°ì¡?°ì ?ì¹ë³ë¡ ê°ê¹ì´ ?ì¹???ëê±?0ë²ì array ë¡ë?ê²½í???ê²ì¼ë¡??¤ì ?ë¤.
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

// level ë³?ì???ì
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

	// ??¥í  ê²ì ?ì±??ê°??£ì´ì£¼ê¸°
	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	// MapName ??ê°?¸ì¤ê¸??ë ?ì ë¶ê°?ì¸ ?¨ì´?¤ì´ ?ì
	FString MapName = GetWorld()->GetMapName();
	// ?ë ì²ë¦¬ë¥??´ì¤?¼ì? ?ì?ê² MapName (LevleName) ?¼ë¡ ?ì´
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

	// save ? ë°ë?ë¡??°ì´?°ë¤??ë¶ë¬?¤ê¸°
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
			// ë¶ë¬??weaponName ??map ?ì?ê²?¸ì? ?ì¸?ì ì°©ì©?í¤ê¸?
=======
			// ºÒ·¯¿Â weaponName ÀÌ map ¿¡ÀÖ´Â°ÍÀÎÁö È®ÀÎÈÄ¿¡ Âø¿ë½ÃÅ°±â
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

	// ì£½ì´????¥íê²?ë¡ë???´ì?ë ?íë¡?ë¡ë?ê¸°
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	// ??¥ë ?ë²¨??ê³µë°±???ë?¼ë©´ ?´ë¹ ?ë²¨?ì ë¡ë?ê¸° -> ê³µë°±?´ë©´ ê¸°ë³¸ ë§µì??ë¡ë
	if (LoadGameInstance->CharacterStats.LevelName != TEXT(""))
	{
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);

		Switchlevel(LevelName);
	}
}

// ?ë²¨ ë³ê²½í?ë ?´ì  ê°ì§ê³??ëê²ë¤ ë³ê²??ì? ?ê² ?ê¸°
void AMainCharacter::LoadGameNoSwitch()
{
	URPGSaveGame* LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));

	LoadGameInstance = Cast<URPGSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	// save ? ë°ë?ë¡??°ì´?°ë¤??ë¶ë¬?¤ê¸°
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

			// ë¶ë¬??weaponName ??map ?ì?ê²?¸ì? ?ì¸?ì ì°©ì©?í¤ê¸?
			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}

		}
	}

	// ì£½ì´????¥íê²?ë¡ë???´ì?ë ?íë¡?ë¡ë?ê¸°
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}


