// Fill out your copyright notice in the Description page of Project Settings.


#include "ColliderMovement.h"

void UColliderMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //pawn소유자가 null, updatedcomp가 null, update가skip 되는지 checking
    if(!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
    {
        return;
    }
    //초기화.노말라이징
    FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f);

    //인풋이 있을때 이동하는거
    if(!DesiredMovementThisFrame.IsNearlyZero())
    {
        FHitResult Hit;
        SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

        //벽같은거에 부딪쳤을때 미끄러져 이동하는거 
        if(Hit.IsValidBlockingHit())
        { 
            SlideAlongSurface(DesiredMovementThisFrame, 1.f - Hit.Time, Hit.Normal, Hit);
            UE_LOG(LogTemp, Warning, TEXT("Blocking!"));
        }
    }
}