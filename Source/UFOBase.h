// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUFOCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"
#include "Utils/ProceduralComponent.h"
#include "UFOBase.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class ACharacter;
class AProjectUFOCharacter;

UCLASS()
class PROJECTUFO_API AUFOBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUFOBase();

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void CalSpeedAndDirection();
	
	void UpdateLocation(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* UFOMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	UStaticMeshComponent* ShieldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	USphereComponent* SphereCollision;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* DeathFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* HitFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	UParticleSystemComponent* TrailingComponent;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	// UCameraComponent* UFOCamera;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_ShieldEnable, Category = Base)
	bool bShieldEnable = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_SkillAEnable, Category = Base)
	bool bSkillAEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	bool bUFOEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_SkillBEnable, Category = Base)
	bool bSkillBEnable = false;

	bool bIsDeath = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_WindAngle, Category = Base)
	int32 WindAngle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	int32 UFORadius = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	float MaxTiltAngle = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	USoundCue* HitSoundCue = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = speed)
	FVector2D CurrentVelocity = FVector2D::ZeroVector;

	//当前加速度受以下两个加速度影响
	UPROPERTY(BlueprintReadWrite, Category = speed)
	FVector2D CurrentAcceleratedVelocity = FVector2D::ZeroVector;

	//玩家控制产生的加速度
	UPROPERTY(BlueprintReadWrite, Category = speed)
	FVector2D PlayerAcceleratedVelocity = FVector2D::ZeroVector;

	//自带加速度
	UPROPERTY(BlueprintReadWrite, Category = speed)
	FVector2D SelfAcceleratedVelocity = FVector2D::ZeroVector;

	//自带加速度
	UPROPERTY(BlueprintReadWrite, Category = speed)
	FVector2D AirResistanceVelocity = FVector2D::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, Category = speed)
	float CurrentAcceleratedSpeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = speed)
	float MaxAcceleratedSpeed = 500;
	
	UPROPERTY(BlueprintReadOnly, Category = speed)
	float CurrentSpeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = speed)
	float AirResistanceFactor = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = speed)
	float MaxSpeed = 300;
	
	UPROPERTY(EditAnywhere ,BlueprintReadWrite, Category = Base)
	bool bIsProcessedByPlayer = true;

	UPROPERTY(BlueprintReadWrite)
	float UnProcessedTime = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	float Elastic = 0.6;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AProjectUFOCharacter* PlayerA;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AProjectUFOCharacter* PlayerB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bViewUpdate = false;

	float TargetZ = 0;
	
	float CurrentZ = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = speed)
	float SpeedZ = 100;

	bool bPlayerAEnable = true;
	bool bPlayerBEnable = true;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_CurrentHP, Category = Base)
	int32 CurrentHP = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_MaxHP, Category = Base)
	int32 MaxHP = 100;
	
	UPROPERTY(BlueprintReadWrite)
	int32 LocalHP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	float FlyingHeight = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = speed)
	bool CanVelocityChange = true;
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void ChangeViewTarget(float ArmLength, FRotator Rotator);

	UFUNCTION(BlueprintCallable)
	void PlayerEnableUFO(EPlayerTag PlayerTag);

	UFUNCTION(BlueprintCallable)
	void SetUFOPlayerDisable() ;
	
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentHP() const;
	
	UFUNCTION(BlueprintCallable)
	int32 SetCurrentHP(int32 NewCurrentHP);
	
	UFUNCTION(BlueprintCallable)
	int32 SetDamage(int32 Damage);

	UFUNCTION(BlueprintCallable)
	int32 SetMaxHP(int32 NewMaxHP);

	UFUNCTION(BlueprintCallable)
	FVector2D SetCurrentVelocity(const FVector2D NewCurrentVelocity);

	UFUNCTION(BlueprintCallable)
	FVector2D SetCurrentAcceleratedVelocity(const FVector2D NewCurrentAcceleratedVelocity);

	UFUNCTION(BlueprintCallable)
	FVector2D SetPlayerAcceleratedVelocity(const FVector2D NewPlayerAcceleratedVelocity);

	UFUNCTION(BlueprintCallable)
	void SetWindAngle(int32 InWindAngle);
	
	UFUNCTION(BlueprintCallable)
	void SetUFOLocationWithTeleport(const FVector NewLocation);
	
	UFUNCTION(BlueprintCallable)
	void SetUFOLocation(const FVector NewLocation);

	UFUNCTION(BlueprintCallable)
	FVector2D SetSelfAcceleratedVelocity(const FVector2D NewSelfAcceleratedVelocity);

	UFUNCTION(BlueprintCallable)
	float SetCurrentAcceleratedSpeed(const float NewCurrentAcceleratedSpeed);

	UFUNCTION(BlueprintCallable)
	float SetMaxAcceleratedSpeed(const float NewMaxAcceleratedSpeed);

	UFUNCTION(BlueprintCallable)
	void GetViewTarget(float& ArmLength, FRotator& Rotator);

	UFUNCTION(BlueprintCallable)
	void GetForwardRotator(FRotator& Rotator);

	void SetForwardRotator(FRotator& Rotator);

	UFUNCTION(BlueprintCallable)
	void UpdateViewTarget();

	UFUNCTION(BlueprintCallable)
	FQuat RotatorToQuat(FRotator Rotator);

	UFUNCTION(BlueprintCallable)
	FRotator QuatToRotator(FQuat Quaternion);

	UFUNCTION(BlueprintCallable)
	FQuat LerpQuat(FQuat A, FQuat B, float Alpha);

	UFUNCTION(BlueprintCallable)
	float SafeLerpAngle(float A, float B, float Alpha);
	
	UFUNCTION(BlueprintCallable)
	float SetCurrentSpeed(const float NewCurrentSpeed);

	UFUNCTION(BlueprintCallable)
	float SetMaxSpeed(const float NewMaxSpeed);

	UFUNCTION(BlueprintCallable)
	bool SetIsProcessedByPlayer(const bool bNewIsProcessedByPlayer);
	
	UFUNCTION(BlueprintCallable)
	bool SetShieldEnable(const bool bNewShieldEnable);
	
	UFUNCTION(BlueprintCallable)
	bool SetSkillAEnable(bool bNewSkillAEnable);
	
	UFUNCTION(BlueprintCallable)
	bool SetSkillBEnable(bool bNewSkillAEnable);
	
	UFUNCTION(BlueprintCallable)
	bool SetUFOEnable(bool bNewUFOEnable);
	

	UFUNCTION(BlueprintNativeEvent)
	void SetUFOEnableWithDelay(bool bNewUFOEnable,float DelayTime);

	UFUNCTION(BlueprintCallable)
	void ResetUFO();

	UFUNCTION(BlueprintNativeEvent)
	void SetTopBoxCollision(bool State);
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_MaxHP();

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_CurrentHP();

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_ShieldEnable();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_SkillAEnable();

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_SkillBEnable();

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_WindAngle();

	UFUNCTION(BlueprintNativeEvent)
	void SetActorRotationWithVelocity();

	UFUNCTION()
	void DealUFODeath();
	
	UFUNCTION(NetMulticast, Reliable)
	void DealUFODeathMulti();


	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void PlaySoundAtLocationMulti(USoundBase* Sound, FVector Location);
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void OnHitMulti(FVector HitLocation);

	UFUNCTION(BlueprintNativeEvent)
	void SpawnFXWithDestroy(UParticleSystem* FX, float DestroyTime);

	UFUNCTION(BlueprintNativeEvent)
	void DealUFODeathBlueprint(float RespawnTime, float ShowUITime);

	UFUNCTION(BlueprintCallable)
	void OnPlayerLogin(AProjectUFOCharacter* Player);
	
	UFUNCTION()
	void OnPlayerASkillTrigger();
	UFUNCTION()
	void OnPlayerBSkillTrigger();
	UFUNCTION()
	void OnPlayerASkillCooldown();
	UFUNCTION()
	void OnPlayerBSkillCooldown();
	UFUNCTION()
	void OnShieldSkillTrigger();
	UFUNCTION()
	void OnShieldSkillCooldown();
	
	void ProcessSkillAStateChange(EProceduralState State);

	void ProcessSkillBStateChange(EProceduralState State);

public:
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps( TArray< class FLifetimeProperty > & OutLifetimeProps ) const override;

private:
	//处理漂浮与坡面旋转
	void HandleFloatAndRotation(FVector CurrentLocation, bool bIsOnPlantForm);

	UFUNCTION()
	void SetPlayerLocationOffSet(FVector PlayerAOffSet, FVector PlayerBOffset);

	
	
};



