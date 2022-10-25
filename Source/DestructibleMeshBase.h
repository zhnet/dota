// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleComponent.h"
#include "DestructibleMeshBase.generated.h"

UCLASS()
class PROJECTUFO_API ADestructibleMeshBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADestructibleMeshBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void ApplyDamageClient(float DamageAmount, const FVector& HitLocation, const FVector& ImpulseDir, float ImpulseStrength);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Component)
	UDestructibleComponent* DestructibleComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Component)
	USceneComponent* SceneComponent = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
