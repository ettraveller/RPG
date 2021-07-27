// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include <Blueprint/UserWidget.h>
#include <Math/Vector2D.h>


void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();


    // 기본 HUD viewport 에 보이게 하는것
    if (HUDOverlayAsset)
    {
        HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
    }
    HUDOverlay->AddToViewport();
    HUDOverlay->SetVisibility(ESlateVisibility::Visible);

    // 몬스터 HP Bar viewport에 보이게 하는것
    if (WEnemyHealthBar)
    {
        EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
        if (EnemyHealthBar)
        {
            EnemyHealthBar->AddToViewport();
            EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        EnemyHealthBar->SetAlignmentInViewport(Alignment);
    }

    // PauseMenu viewport 에 보이게 하는것
	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
            PauseMenu->AddToViewport();
            PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}

	}

	// Inventory viewport 에 보이게 하는것
	if (WInventory)
	{
		Inventory = CreateWidget<UUserWidget>(this, WInventory);
		if (Inventory)
		{
			Inventory->AddToViewport();
			Inventory->SetVisibility(ESlateVisibility::Hidden);
		}

	}
}


void AMainPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (EnemyHealthBar)
    {
        FVector2D PositionInViewport;
        ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
        PositionInViewport.Y = 50.f;

        FVector2D SizeInViewport(300.f, 25.f);

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
    }
}

void AMainPlayerController::DisplayEnemyHealthBar()
{
    if (EnemyHealthBar)
	{
        bEnemyHealthBarVisible = true;
        EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
    }

}


void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
        PauseMenu->SetVisibility(ESlateVisibility::Visible);

        // 게임 실행시 마우스크 커서가 보이진 않지만 커서가 있는것처럼 해서 pausemenu 버튼들누를수 있게 하는것
        FInputModeGameAndUI InputModeGameAndUI;
        SetInputMode(InputModeGameAndUI);
        // 밑에를 해줘야지 마우스크 커서가 viewport 에서 보임
        bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
    if (PauseMenu)
	{
        GameModeOnly();
		bShowMouseCursor = false;

		bPauseMenuVisible = false;
		//PauseMenu->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::TogglePauseMenu()
{
    // 뒤에 Implementation 붙이면 애니메이션 실행 안됨으로 부모클래스로 호출될수 있게 두기
    if (bPauseMenuVisible)
    {
        RemovePauseMenu();
    }
    else {
        DisplayPauseMenu();
    }

}

void AMainPlayerController::GameModeOnly()
{
	// pasuemenu 들이 사라지면 마우스 커서도 사라지고, 일반적인 게임모드로 바꾸기
	FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);
}

void AMainPlayerController::DisplayInventoryMenu_Implementation()
{
	if (Inventory)
	{
		bInventoryMenu = true;
        Inventory->SetVisibility(ESlateVisibility::Visible);

		// 게임 실행시 마우스크 커서가 보이진 않지만 커서가 있는것처럼 해서 pausemenu 버튼들누를수 있게 하는것
		FInputModeGameAndUI InputModeGameAndUI;
		SetInputMode(InputModeGameAndUI);
		// 밑에를 해줘야지 마우스크 커서가 viewport 에서 보임
		bShowMouseCursor = true;
	}

}

void AMainPlayerController::RemoveInvnetoryMenu_Implementation()
{
	if (Inventory)
	{
		GameModeOnly();
		bShowMouseCursor = false;

		bInventoryMenu = false;
		Inventory->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::ToggleInventoryMenu()
{
	if (bInventoryMenu)
	{
		RemoveInvnetoryMenu_Implementation();
	}
	else {
		DisplayInventoryMenu_Implementation();
	}
}
