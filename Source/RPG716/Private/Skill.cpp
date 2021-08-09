// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill.h"
#include <Components/SphereComponent.h>
#include <GameFramework/ProjectileMovementComponent.h>
#include <Particles/ParticleSystemComponent.h>
#include <GameFramework/Actor.h>
#include "Enemy.h"
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundCue.h>
#include "SkillComponent.h"


// Sets default values
ASkill::ASkill()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(FName("SphereComp"));

	SphereComp->SetCollisionProfileName(FName("BlockAll"));

	SetRootComponent(SphereComp);

	//Initializing the movement component and its properties
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovementComp"));

	ProjectileMovementComp->ProjectileGravityScale = 0.f;
	ProjectileMovementComp->InitialSpeed = 3000.f;

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(FName("ParticleComp"));
	ParticleComp->SetupAttachment(SphereComp);
	
	Damage = 30;
}

void ASkill::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ProjectileCollisionFX)
	{
		//Activate the collision FX and start the destroy timer
		ParticleComp->SetTemplate(ProjectileCollisionFX);
		ParticleComp->Activate(true);
		if(OtherActor)
		{
		    AEnemy* Enemy = Cast<AEnemy>(OtherActor);

			if (Enemy)
			{
			    UGameplayStatics::ApplyDamage(Enemy, Damage, SkillInstigator, this, DamageTypeClass);
			}
			if (ExplosionSound)
			{
				UGameplayStatics::PlaySound2D(this, ExplosionSound);
			}
		}

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;

		TimerDel.BindLambda([&]()
			{
				Destroy();
			});

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, DestroyDelay, false);
	}
}

// Called when the game starts or when spawned
void ASkill::BeginPlay()
{
	Super::BeginPlay();

	SphereComp->OnComponentHit.AddDynamic(this, &ASkill::OnHit);

	if (ProjectileFX)
	{
		ParticleComp->SetTemplate(ProjectileFX);
		ParticleComp->Activate();
	}
	
}

void ASkill::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//Used in order to have a visual feedback in the editor when we 
	//assign a new particle
	if (ProjectileFX)
	{
		ParticleComp->SetTemplate(ProjectileFX);
		ParticleComp->Activate();
	}
}


void ASkill::AdvanceLevel_Fire()
{
	if (CurrentLevel_fire + 1 >= MaxLevel_Fire)
	{
		CurrentLevel_fire = MaxLevel_Fire;
	}
	else
	{
	    CurrentLevel_fire++;
	}
}


void ASkill::AdvanceLevel_Poision()
{
	if (CurrentLevel_Poision + 1 >= MaxLevel_Poision)
	{
		CurrentLevel_Poision = MaxLevel_Poision;
	}
	else
	{
		CurrentLevel_Poision++;
	}
}


void ASkill::ResetLevel()
{
	 CurrentLevel_fire = 1;
	 CurrentLevel_Poision = 1; 
	 //SkillComp->AvailableSkillPoints = SkillComp->InitialAvailableSkillsPoints;
}

// Called every frame
void ASkill::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

