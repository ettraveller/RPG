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

	//���� ������ ���� ������
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	// ���� HP ǥ��
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;

	// RESUME / SAVEGAME / LOADGAME / QUIT HUD ����� 
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	bool bEnemyHealthBarVisible;

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	FVector EnemyLocation;

	bool bPauseMenuVisible;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayPauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void TogglePauseMenu();

	void GameModeOnly();

protected:

    virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};