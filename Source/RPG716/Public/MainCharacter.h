// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.h"
#include "PickUp.h"
#include "MainCharacter.generated.h"



//단, 여기서 주의해야할 점이 있는데, 언리얼 열거형을 만들때 반드시 일반적인 enum이 아닌 enum class로 만들어야 한다는 점이다.
//만약 enum class로 만들지 않고 일반적인 enum으로 만들어서 UENUM() 매크로를 붙이고 컴파일을 하면 에러가 발생해서 컴파일에 실패한다.
//그리고 UENUM은 uint8만을 지원하기 때문에 이 부분도 빠뜨리지 않고 넣어주어야 한다.




UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
// UMETA = 리플렉션과 블루프린트에 쓸것

    EMS_Normal UMETA(DisplayName = "EMS_Normal"),
    EMS_Sprinting UMETA(DisplayName = "EMS_Sprinting"),
    EMS_Dead UMETA(DisplayName = "EMS_Dead"),

    EMS_Max UMETA(DisplayName = "EMS_DefaultMax"),

};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
    ESS_Normal UMETA(DisplayName = "ESS_Normal"),
	ESS_BelowMinimum UMETA(DisplayName = "ESS_BelowMinimum"),
	ESS_Exhausted UMETA(DisplayName = "ESS_Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ESS_ExhaustedRecovering"),

	ESS_Max UMETA(DisplayName = "ESS_DefaultMax"),

};

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateInvnetoryDelegate, const TArray<AItem*>&, InvnetoryItems);


//Character의 rootcomp는 capsule밖에 안댐
UCLASS()
class RPG716_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();
	
	// Weapon 데이터 저장시에 필요 -> Weapon 데이터들이 저장되어 있는 BP_ItemStorage 를 save 해야함!
	UPROPERTY(EditDefaultsOnly, Category = "SaveData")
	TSubclassOf<class AItemStorage> WeaponStorage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	FORCEINLINE void setHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	// Enemy healthbar 몬스터에 나타내게 하기 위해서
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	// Enemy hp 띄우기 위해서 필요
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	TArray<FVector> PickUpLocations;

	// Tarray 에 저장한 pcikuplocation 들을 표시해줄 함수
	UFUNCTION(BlueprintCallable)
	void ShowPickUpLocation();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status;}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	float InterSpeed;
	bool bInterToEnemy;
	void SetInterToEnenmy(bool Interp);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	FRotator GetLookAtRotaionYaw(FVector Target);

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	// movement 상태 변경 및 running speed 설정
	//set SetMovementStatus and runnigspeed
	void SetMovementStatus(EMovementStatus Status);

	// sprinting 모드에 사용됨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintSpeed;

	bool bShiftKeyDown;

	// down 이 눌리면 sprinting 모드로 전환
	//pressed 
	void ShiftKeyDown();
	// up 이 눌리면 sprinting 모드 해제
	//released 
	void ShiftKeyUp();

	// 플레이어 뒷쪽에서 카메라 확인가능
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	////////
	// Player Stats
	////////
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 Coins;



	// 인벤토리 Tarray
	//TArray<AItem*> _inventory;
	TArray<AItem*> Inventory;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// forward, backward input 값 불러오기
	//playerinput
	void MoveForward(float Value);

	// Y 측 값 불러오기
	void MoveRight(float Value);

	void Turn(float Value);
	void LookUp(float Value);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void Dash();
	void StopDash();
	void ResetDash();
	
	UPROPERTY()
	FTimerHandle DashTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dash")
	float DashDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dash")
	float DashCoolDown;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Dash")
	float DashStopTime;

	bool bCanDash;


	// 움직임 상태를 확인하여 sprint 가능하게 하기
	bool bMovingForward;
	bool bMovingRight;
	
	// 움직임 체크해야하는것들 체크하는 함수
	bool CanMove(float Value);

	
	//HUD
	void DecrementHealth(float Amount);
	
	// 공격당하면 health 줄게하기
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);

	// Potion 사용시 health 증가
	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	// Potion 사용시 stamina 증가
	UFUNCTION(BlueprintCallable)
	void IncrementStamina(float Amount);
	void Die();

	virtual void Jump() override;

	//Sound
	void LMBDown();
	void LMBUp();
	bool bLMBDown;

	//PauseMenu
	void ESCDown();
	void ESCUp();
	bool bESCDown;

	//inventory
	void InventoryDown();
	void InventoryUp();
	bool bInventoryDown;

	//Pickup
	void PickUpItemDown();
	void PickUpItemUp();
	bool bPickUpItem;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// 장비 착용할때 필요
	//SetEquipWeapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class AWeapon* EquipWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class AItem* ActiveOverlappingItem;

	// Weapon 장비 객체를 따로 받아옴
	void SetEquipWeapon(AWeapon* WeaponToSet);
	FORCEINLINE AWeapon* GetEquipWeapon() {return EquipWeapon; }
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) {ActiveOverlappingItem = Item; }
	//void SetActiveOverlappingItem(AActor* OtherActor);

	// 포션 사용 여부 확인시에 필요
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class APickUp* PickUpItem;

	FString GetPotionNameValue;

	UFUNCTION(BlueprintCallable)
	void SetPotion(APickUp* PickUpToSet, FString PositonName) {PickUpItem = PickUpToSet; GetPotionNameValue = PositonName;}
	FORCEINLINE APickUp* GetPickUp() { return PickUpItem;}

	UFUNCTION(BlueprintCallable)
	FString GetPotionName() {return GetPotionNameValue;}


	// 장비 득템시 HUD
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equip")
	UFUNCTION(BlueprintCallable)
	bool CallItemEquip(bool result, AItem* Item);

	/*UFUNCTION(BlueprintCallable)
	AItem* CallItemEquip(AItem* Item);*/
	
	void UnCallItemEquip();

	AWeapon* AttachWeapon;
	UFUNCTION(BlueprintCallable)
	void setWeapon(AWeapon* Weapon) {AttachWeapon = Weapon;}
	FORCEINLINE AWeapon* GetWeapon() {return AttachWeapon;}

	UFUNCTION(BlueprintCallable)
	void EquipOn();

	UFUNCTION(BlueprintCallable)
	void EquipSave();

	UFUNCTION(BlueprintCallable)
	void EquipOnInThumbnail();

	// 물약 득템시 HUD
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equip")
	void CallItemPotion(AItem* Item);

	void UnCallItemPotion();

	UFUNCTION(BlueprintCallable)
	void PotionUse(AItem* Item);

	UFUNCTION(BlueprintCallable)
	void PotionUseInThumbnail();

	UFUNCTION(BlueprintCallable)
	void PotionSave();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	void Attack();

	// 움직이거나 점프할때는 공격 못하게 하기 BP에서 통제
	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	// CombatMontage 만들어 둔것 사용하기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	// 캐릭터 죽는 anim 만들기
	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	// 두마리의 몬스터가 있을시 한마리 죽이면 다른 한마리로 공격대상 바꾸게 하기
	void UpdateCombatTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> EnemyFilter;

	// level별 전환시에 필요
	void Switchlevel(FName LevelName);

	// 게임 저장시에 필요
	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	void LoadGameNoSwitch();

	
	////인벤토리에 아이템 추가시 
	//void AddToInventory(AItem* Item);

	//UFUNCTION(BlueprintCallable)
	//void UpdateInventory();

	//UPROPERTY(BlueprintAssignable, Category = "Item | PickUp")
	//FUpdateInvnetoryDelegate FUpdateInvnetoryDelegate;

	// 아이템 추가하기.
	bool AddItem(class AItem* Item);

	//모든 아이템 빼기
	void DropAllInventory();

	UFUNCTION(BlueprintCallable)
	void DropItem(class AItem* Item);

	// 인벤토리 드롭후에 인벤토리에서 제거하기 위해
	bool RemoveItemFromInventory(class AItem* Item);

	// BP 에서 인벤토리 아이템 가져오는 Tarray 함수
	UFUNCTION(BlueprintCallable)
	TArray<class AItem*> GetInventoryItems();

	// BP 에서 인벤토리 아이템 갯수 가져오는 변수
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentInventoryCount();

	// 아이템을 가지고 있는지 여부 확인
	bool CheckIfClientHasItem(class AItem* Item);


};
