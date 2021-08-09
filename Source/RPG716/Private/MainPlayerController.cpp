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
			UE_LOG(LogTemp, Warning, TEXT("display!!!!!!!!"));
			Inventory->AddToViewport();
			Inventory->SetVisibility(ESlateVisibility::Hidden);
		}

	}

	// ItemEquip viewport 에 보이게 하는것
	if (WItemEquip)
	{
		ItemEquip = CreateWidget<UUserWidget>(this, WItemEquip);
		if (ItemEquip)
		{
			ItemEquip->AddToViewport();
			ItemEquip->SetVisibility(ESlateVisibility::Hidden);
		}

	}

	// ItemPotion viewport 에 보이게 하는것
	if (WItemPotion)
	{
		ItemPotion = CreateWidget<UUserWidget>(this, WItemPotion);
		if (ItemPotion)
		{
			ItemPotion->AddToViewport();
			ItemPotion->SetVisibility(ESlateVisibility::Hidden);
		}

	}
}


void AMainPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (EnemyHealthBar)
    {
		// Enemyhealthbar combattarget 위치에 닿을때마다 나타나게 하기
        FVector2D PositionInViewport;
        ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
        PositionInViewport.Y = 30.f;

        FVector2D SizeInViewport(300.f, 25.f);

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
    }

	if (ItemEquip)
	{
		// Weapon 아이템에 닿을시 위젯 아이템 근처에 나오게 하기 (Y값은 설정하면 너무 높아져서 일단 빼둠)
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(WeaponLocation, PositionInViewport);
		//PositionInViewport.Y = 20.f;

		FVector2D SizeInViewport(300.f, 25.f);

		ItemEquip->SetPositionInViewport(PositionInViewport);
		ItemEquip->SetDesiredSizeInViewport(SizeInViewport);
	}

	if (ItemPotion)
	{
		// Weapon 아이템에 닿을시 위젯 아이템 근처에 나오게 하기 (Y값은 설정하면 너무 높아져서 일단 빼둠)
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(PotionLocation, PositionInViewport);
		//PositionInViewport.Y = 20.f;

		FVector2D SizeInViewport(300.f, 25.f);

		ItemPotion->SetPositionInViewport(PositionInViewport);
		ItemPotion->SetDesiredSizeInViewport(SizeInViewport);
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
		Inventory = CreateWidget<UUserWidget>(this, WInventory);
		Inventory->AddToViewport();
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
		Inventory->RemoveFromParent();
	}
}

void AMainPlayerController::ToggleInventoryMenu()
{
	if (bInventoryMenu)
	{
		RemoveInvnetoryMenu();
	}
	else {
		DisplayInventoryMenu();
	}
}

void AMainPlayerController::DisplayItemEquipMenu_Implementation()
{
	if (ItemEquip)
	{
		bItemEquipMenu = true;
		ItemEquip->SetVisibility(ESlateVisibility::Visible);

		// 게임 실행시 마우스크 커서가 보이진 않지만 커서가 있는것처럼 해서 pausemenu 버튼들누를수 있게 하는것
		FInputModeGameAndUI InputModeGameAndUI;
		SetInputMode(InputModeGameAndUI);
		// 밑에를 해줘야지 마우스크 커서가 viewport 에서 보임
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemoveItemEquipMenu_Implementation()
{
	if (ItemEquip)
	{
		GameModeOnly();
		bShowMouseCursor = false;

		bItemEquipMenu = false;
		ItemEquip->SetVisibility(ESlateVisibility::Hidden);
	}
}

//void AMainPlayerController::ToggleItemEquipMenu()
//{
//	if (bItemEquipMenu)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Toggle!!!!!!!!"));
//		RemoveItemEquipMenu_Implementation();
//	}
//	else {
//		UE_LOG(LogTemp, Warning, TEXT("Toggle!!!!@@@@@@!!!!"));
//		DisplayItemEquipMenu_Implementation();
//	}
//}

void AMainPlayerController::ViewItemEquipMenu()
{
	DisplayItemEquipMenu_Implementation();
}

void AMainPlayerController::UnableItemEquipMenu()
{
	RemoveItemEquipMenu_Implementation();
}

void AMainPlayerController::DisplayItemPotionMenu_Implementation()
{
	if (ItemPotion)
	{
		UE_LOG(LogTemp, Warning, TEXT("display!!!!!!!!"));
		bItemPotionMenu = true;
		ItemPotion->SetVisibility(ESlateVisibility::Visible);

		// 게임 실행시 마우스크 커서가 보이진 않지만 커서가 있는것처럼 해서 pausemenu 버튼들누를수 있게 하는것
		FInputModeGameAndUI InputModeGameAndUI;
		SetInputMode(InputModeGameAndUI);
		// 밑에를 해줘야지 마우스크 커서가 viewport 에서 보임
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemoveIItemPotionMenu_Implementation()
{
	if (ItemPotion)
	{
		UE_LOG(LogTemp, Warning, TEXT("Remove!!!!!!!!"));
		GameModeOnly();
		bShowMouseCursor = false;

		bItemPotionMenu = false;
		ItemPotion->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::ViewItemPotionMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Toggle!!!!!!!!"));
	DisplayItemPotionMenu_Implementation();
}

void AMainPlayerController::UnableItemPotionMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Toggle!!!!####!!!!"));
	RemoveIItemPotionMenu_Implementation();
}

