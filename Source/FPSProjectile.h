// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "FPSProjectile.generated.h"


class UProjectileMovementComponent;
class USphereComponent;
class UParticleSystem;


UCLASS()
class AFPSProjectile : public AActor
{
	GENERATED_BODY()

protected:

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	UParticleSystem* ExplosionFX;

	UFUNCTION(BlueprintCallable)
	void Explode();
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void ExplodeClient();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	int32 Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	USoundCue* ExplodeCue;
public:

	AFPSProjectile();

	virtual void BeginPlay() override;
	

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }

	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

