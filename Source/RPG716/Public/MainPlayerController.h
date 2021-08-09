// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <Blueprint/UserWidget.h>
#include <Templates/SubclassOf.h>
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RPG716_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    
	//Reference to UMG asset
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	//위젯 생성후 변수 담을곳
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	// 몬스터 HP 표헌
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;

	// RESUME / SAVEGAME / LOADGAME / QUIT HUD 만들기 
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	// Inventory HUD 만들기
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WInventory;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* Inventory;

	// 장비 픽업시 착용할지 인벤토리 담을지 선택하는 HUD
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WItemEquip;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* ItemEquip;

	// 포션 픽업시 사용할지 인벤토리 담을지 선택하는 HUD
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WItemPotion;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* ItemPotion;

	bool bEnemyHealthBarVisible;

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	// overlap 된 몬스터 enemy 위치
	FVector EnemyLocation;

	// overlap 된 weapon 액터 위치
	FVector WeaponLocation;

	// overlap 된 Potion 액터 위치
	FVector PotionLocation;

	bool bPauseMenuVisible;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayPauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable)
	void GameModeOnly();

	// 인벤토리 창 이벤트
	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void DisplayInventoryMenu();

	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void RemoveInvnetoryMenu();

	void ToggleInventoryMenu();

	bool bInventoryMenu;

	// 장비 득템시 착용 여부 HUD
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayItemEquipMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveItemEquipMenu();

	//void ToggleItemEquipMenu();

	void ViewItemEquipMenu();

	void UnableItemEquipMenu();

	bool bItemEquipMenu;


	// 포션 득템시 사용 여부 HUD
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayItemPotionMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemoveIItemPotionMenu();

	//void ToggleItemPotionMenu();

	void ViewItemPotionMenu();

	void UnableItemPotionMenu();

	bool bItemPotionMenu;

protected:

    virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
