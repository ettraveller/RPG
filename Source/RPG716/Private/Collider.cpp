// Fill out your copyright notice in the Description page of Project Settings.


#include "Collider.h"
#include <Components/SphereComponent.h>
#include <Components/StaticMeshComponent.h>
#include <UObject/ConstructorHelpers.h>
#include <Components/InputComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "ColliderMovement.h"
#include <Math/UnrealMathUtility.h>




// Sets default values
ACollider::ACollider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
 
    //RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	//SphereComponent->SetupAttachment(GetRootComponent());
	SphereComponent->InitSphereRadius(40.0f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());

	//경로찾아서 하드코딩방법
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshComponentAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	if (MeshComponentAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshComponentAsset.Object);
		MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
		MeshComponent->SetWorldScale3D(FVector(0.8f, 0.8f, 0.8f));
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 1.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	MovementComponent = CreateDefaultSubobject<UColliderMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	CameraInput = FVector2D(0.f, 0.f);
	
	//AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ACollider::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += CameraInput.X;
	SetActorRotation(NewRotation);

	FRotator NewSpringArmRotation = SpringArm->GetComponentRotation();
	NewSpringArmRotation.Pitch = FMath::Clamp(NewSpringArmRotation.Pitch += CameraInput.Y, -80.f, -15.f);
	SpringArm->SetWorldRotation(NewSpringArmRotation);
}

// Called to bind functionality to input
void ACollider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACollider::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACollider::MoveRight);

	PlayerInputComponent->BindAxis(TEXT("CameraPitch"), this, &ACollider::CameraPitch); 
	PlayerInputComponent->BindAxis(TEXT("CameraYaw"), this, &ACollider::CameraYaw);
}

UPawnMovementComponent* ACollider::GetMovementComponent() const
{
    return MovementComponent;
}

void ACollider::MoveForward(float input) 
{
    FVector Forward = GetActorForwardVector();
	//AddMovementInput(input * Right);
	if (MovementComponent)
	{
		MovementComponent->AddInputVector(Forward * input);
	}
	
}

void ACollider::MoveRight(float input)
{
	FVector Right = GetActorRightVector();
	//AddMovementInput(input * Right);
	//addmovementInput 까보고 Movementcomponent가 없어서 사용하지 못하고 안에있는 소스 긁어서 사용
	//pawn은 movementcomponent가 character와 달리 없다.
	if (MovementComponent)
	{
		MovementComponent->AddInputVector(Right * input);
	}

}

void ACollider::CameraPitch(float AxisValue)
{
    CameraInput.Y = AxisValue;
}

void ACollider::CameraYaw(float AxisValue)
{
    CameraInput.X = AxisValue;
}

