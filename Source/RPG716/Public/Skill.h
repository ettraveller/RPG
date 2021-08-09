// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Skill.generated.h"


UENUM(BlueprintType)
enum class ESkillType : uint8
{
	FireBall,
	PoisionBall,
};

UCLASS()
class RPG716_API ASkill : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkill();

private:

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	/*Increases the level by one - clamps on max level*/
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	void AdvanceLevel_Fire();
	

	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	void AdvanceLevel_Poision();

	/*Resets the level of the skill - 0 means that the player will not be able to fire*/
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	void ResetLevel();

	UPROPERTY(EditDefaultsOnly)
	class USkillComponent* SkillComp;
	

	/*Returns the level of the current skill*/
	int32 GetLevel_Fire() { return CurrentLevel_fire; }
	int32 GetLevel_Poision() { return CurrentLevel_Poision; }


	/*Returns the skill's texture*/
	UFUNCTION(BlueprintCallable, Category = TLSkillsTree)
	UTexture* GetSkillTexture() { return SkillTexture; }

	/*Returns the skill type*/
	ESkillType GetSkillType() { return SkillType; }

	/*Returns true if the level is maxed out*/
	bool IsMaxLevel_Fire() { return CurrentLevel_fire == MaxLevel_Fire; }
	bool IsMaxLevel_Poision() { return CurrentLevel_Poision == MaxLevel_Poision; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Damage;
	/*Sphere comp used for collision*/
protected:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* SphereComp;

	/*This component is used to simulate the movement of our skill*/
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComp;

	/*The particle comp which emits the active particle system*/
	UPROPERTY(VisibleAnywhere)
	class UParticleSystemComponent* ParticleComp;

	/*The particle system for our projectile when traveling*/
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ProjectileFX;

	/*The particle system for our collision*/
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ProjectileCollisionFX;

	/*The skill texture*/
	UPROPERTY(EditDefaultsOnly)
	UTexture* SkillTexture;

	/*The time (after a collision has happened) that our skill will get destroyed*/
	UPROPERTY(EditAnywhere)
	float DestroyDelay = 0.5f;

	/*The skill type of the skill*/
	UPROPERTY(EditDefaultsOnly)
	ESkillType SkillType;

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	AController* SkillInstigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill | Sounds")
	class USoundCue* ExplosionSound;
	

	

private:

	int32 CurrentLevel_fire = 1;
	int32 CurrentLevel_Poision = 1;

	int32 MaxLevel_Fire = 5;
	int32 MaxLevel_Poision = 5;

public:	
	// Called every frame
	
	virtual void Tick(float DeltaTime) override;

};
