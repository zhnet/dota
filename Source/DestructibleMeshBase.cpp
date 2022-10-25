// Fill out your copyright notice in the Description page of Project Settings.


#include "DestructibleMeshBase.h"
// Sets default values
ADestructibleMeshBase::ADestructibleMeshBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	
	DestructibleComponent = CreateDefaultSubobject<UDestructibleComponent>(TEXT("DestructibleComponnet"));

	RootComponent = SceneComponent;
	DestructibleComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADestructibleMeshBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADestructibleMeshBase::ApplyDamageClient_Implementation(float DamageAmount, const FVector& HitLocation,
	const FVector& ImpulseDir, float ImpulseStrength)
{
	DestructibleComponent->ApplyDamage(DamageAmount, HitLocation, ImpulseDir, ImpulseStrength);
}

// Called every frame
void ADestructibleMeshBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

