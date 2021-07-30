// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            Enemy = Cast<AEnemy>(Pawn);
        }
    }
}

void UEnemyAnimInstance::UpDateAnimationProerties()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            // 적인지 판별하여 적끼리는 움직이다 마주쳤을시 겹쳐지게 하기
            Enemy = Cast<AEnemy>(Pawn);
        }
    }
    
    if (Pawn)
    {
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();
    }
}
