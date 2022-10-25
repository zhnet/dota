// Fill out your copyright notice in the Description page of Project Settings.


#include "UFOBase.h"

#include "DrawDebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "ProjectUFOCharacter.h"
#include "ProjectUFOCharacter.h"
#include "ProjectUFOGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Utils/ProceduralComponent.h"

class AProjectUFOGameMode;
// Sets default values
AUFOBase::AUFOBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	UFOMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UFOMesh"));
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	TrailingComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailingComponent"));
	RootComponent = SphereCollision;
	UFOMesh->SetupAttachment(RootComponent);
	ShieldMesh->SetupAttachment(UFOMesh);
	TrailingComponent->SetupAttachment(RootComponent);

	

}

// Called when the game starts or when spawned
void AUFOBase::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		AProjectUFOGameMode* CurrentGameMode = Cast<AProjectUFOGameMode>(UGameplayStatics::GetGameMode(this));
		if (IsValid(CurrentGameMode))
		{
			CurrentGameMode->PlayerPostLoginDelegate.AddDynamic(this, &AUFOBase::OnPlayerLogin);
		}
	}

}

void AUFOBase::CalSpeedAndDirection_Implementation()
{

}

void AUFOBase::UpdateLocation(float DeltaTime)
{
	FVector LastPosition = GetActorLocation();
	if(CanVelocityChange)
	{
		//计算风阻
		float AirResistanceAcceleratedSpeed = (CurrentSpeed * CurrentSpeed) * AirResistanceFactor;
		AirResistanceVelocity = CurrentVelocity.GetSafeNormal() * AirResistanceAcceleratedSpeed;
		SetCurrentAcceleratedVelocity(PlayerAcceleratedVelocity + SelfAcceleratedVelocity - AirResistanceVelocity);
		const FVector2D NewCurrentVelocity = CurrentVelocity + CurrentAcceleratedVelocity * DeltaTime;
		SetCurrentVelocity(NewCurrentVelocity);
	}
	
	FVector NewLocation = LastPosition + FVector(CurrentVelocity * DeltaTime, 0);

	//计算Z变化
	if (FMath::Abs(TargetZ - LastPosition.Z) > 0.5)
	{
		float DeltaZ = SpeedZ * DeltaTime;
		if (TargetZ > LastPosition.Z)
		{
			CurrentZ = LastPosition.Z + DeltaZ;
			CurrentZ = FMath::Min(TargetZ, CurrentZ);
		}
		else
		{
			CurrentZ = LastPosition.Z - DeltaZ;
			CurrentZ = FMath::Max(TargetZ, CurrentZ);
		}
	}


	FHitResult Result;
	// if(PlayerA.IsValid())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("before Player A location is %s") , *PlayerA->GetActorLocation().ToString());
	// 	UCharacterMovementComponent* PlayerMovementComponent = Cast<UCharacterMovementComponent>(PlayerA->GetMovementComponent());
	// 	if(IsValid(PlayerMovementComponent))
	// 	{
	// 		PlayerMovementComponent->SetMovementMode(MOVE_None);
	// 	}
	//
	// }
	SetActorLocation(NewLocation, true, &Result);

	if (Result.Actor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("HitActor is : %s"), *Result.Actor->GetName());
		DrawDebugSphere(GetWorld(), Result.Location, 30, 32, FColor::Red);
	}
	//SetActorLocation(NewLocation, true, &Result);
	HandleFloatAndRotation(GetActorLocation(), false);
	// if(PlayerA.IsValid())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("After Player A location is %s") , *PlayerA->GetActorLocation().ToString());
	// 	UCharacterMovementComponent* PlayerMovementComponent = Cast<UCharacterMovementComponent>(PlayerA->GetMovementComponent());
	// 	if(IsValid(PlayerMovementComponent))
	// 	{
	// 		PlayerMovementComponent->SetMovementMode(MOVE_Walking);
	// 	}
	// 	FVector OffSet = GetActorLocation() - LastPosition;
	// 	PlayerA->SetActorLocation(PlayerA->GetActorLocation() + OffSet, false);
	// }
}


float AUFOBase::SetCurrentSpeed(const float NewCurrentSpeed)
{
	if (NewCurrentSpeed < 0)
	{
		CurrentSpeed = 0;
	}
	else if (NewCurrentSpeed > MaxSpeed)
	{
		CurrentSpeed = MaxSpeed;
	}
	else
	{
		CurrentSpeed = NewCurrentSpeed;
	}
	CurrentVelocity = CurrentVelocity.GetSafeNormal() * CurrentSpeed;
	return CurrentSpeed;
}



float AUFOBase::SetMaxSpeed(const float NewMaxSpeed)
{
	if (NewMaxSpeed < 0)
	{
		MaxSpeed = 0;
	}
	else
	{
		MaxSpeed = NewMaxSpeed;
	}

	if (CurrentSpeed > MaxSpeed)
	{
		SetCurrentSpeed(MaxSpeed);
	}
	return MaxSpeed;
}

bool AUFOBase::SetIsProcessedByPlayer(const bool bNewIsProcessedByPlayer)
{
	if (bIsProcessedByPlayer != bNewIsProcessedByPlayer)
	{
		bIsProcessedByPlayer = bNewIsProcessedByPlayer;
		if (!bIsProcessedByPlayer)
		{
			SetPlayerAcceleratedVelocity(FVector2D::ZeroVector);
		}
	}
	return bIsProcessedByPlayer;
}


bool AUFOBase::SetShieldEnable(const bool bNewShieldEnable)
{
	bShieldEnable = bNewShieldEnable;
	return bShieldEnable;
}

bool AUFOBase::SetSkillAEnable(bool bNewSkillAEnable)
{
	bSkillAEnable = bNewSkillAEnable;
	return bSkillAEnable;
}

bool AUFOBase::SetSkillBEnable(bool bNewSkillAEnable)
{
	bSkillBEnable = bNewSkillAEnable;
	return bSkillBEnable;
}




bool AUFOBase::SetUFOEnable(bool bNewIsEnable)
{
	bUFOEnable = bNewIsEnable;
	return bUFOEnable;
}


void AUFOBase::SetUFOEnableWithDelay_Implementation(bool bNewUFOEnable, float DelayTime)
{

}

void AUFOBase::ResetUFO()
{
	SetPlayerAcceleratedVelocity(FVector2D::ZeroVector);
	SetCurrentVelocity(FVector2D::ZeroVector);
	SetSelfAcceleratedVelocity(FVector2D::ZeroVector);
	SetSkillAEnable(false);
	SetSkillBEnable(false);
	SetShieldEnable(false);
	SetCurrentHP(MaxHP);
	bIsDeath = false;
}

void AUFOBase::SetTopBoxCollision_Implementation(bool State)
{
	
}

void AUFOBase::OnPlayerASkillTrigger()
{
	SetSkillAEnable(true);
}

void AUFOBase::OnPlayerBSkillTrigger()
{
	SetSkillBEnable(true);
}

void AUFOBase::OnPlayerASkillCooldown()
{
	SetSkillAEnable(false);
}

void AUFOBase::OnPlayerBSkillCooldown()
{
	SetSkillBEnable(false);
}

void AUFOBase::OnShieldSkillTrigger()
{
	SetShieldEnable(true);
}

void AUFOBase::OnShieldSkillCooldown()
{
	SetShieldEnable(false);
}

void AUFOBase::ProcessSkillAStateChange(EProceduralState State)
{
	switch (State)
	{
	case EProceduralState::Trigger:
		OnPlayerASkillTrigger();
		break;
	case EProceduralState::Idle:
		OnPlayerASkillCooldown();
		break;
	default:
		break;
	}
}

void AUFOBase::ProcessSkillBStateChange(EProceduralState State)
{
	switch (State)
	{
	case EProceduralState::Trigger:
		OnPlayerBSkillTrigger();
		break;
	case EProceduralState::Idle:
		OnPlayerBSkillCooldown();
		break;
	case EProceduralState::Process:
		OnShieldSkillTrigger();
		break;
	case EProceduralState::Suspend:
		OnShieldSkillCooldown();
		OnPlayerASkillCooldown();
		OnPlayerBSkillCooldown();
		break;
	default:
		break;
	}
}

void AUFOBase::PlayerEnableUFO(EPlayerTag PlayerTag)
{
	if(!HasAuthority())
	{
		return;
	}
	if(PlayerTag == EPlayerTag::PlayerA)
	{
		bPlayerAEnable = true;
	}
	else
	{
		bPlayerBEnable = true;
	}
	if(bPlayerAEnable && bPlayerBEnable)
	{
		SetUFOEnable(true);
	}
}

void AUFOBase::SetUFOPlayerDisable() 
{
	bPlayerAEnable = false;
	bPlayerBEnable = false;
	SetUFOEnable(false);
}



int32 AUFOBase::GetCurrentHP() const
{
	return CurrentHP;
}

int32 AUFOBase::SetCurrentHP(int32 NewCurrentHP)
{
	if (NewCurrentHP <= 0)
	{
		this->CurrentHP = 0;
		//处理飞碟坠毁
		// AProjectUFOGameMode* CurrentGameMode =  Cast<AProjectUFOGameMode>( UGameplayStatics::GetGameMode(this));
		// CurrentGameMode->RestartGame();
		if(!bIsDeath)
		{
			DealUFODeath();

		}
	}
	else if (NewCurrentHP > MaxHP)
	{
		this->CurrentHP = MaxHP;


	}
	else
	{
		this->CurrentHP = NewCurrentHP;
	}

	return CurrentHP;
}

int32 AUFOBase::SetDamage(const int32 Damage)
{
	if (bShieldEnable)
	{
		return CurrentHP;
	}
	return SetCurrentHP(CurrentHP - Damage);
}


int32 AUFOBase::SetMaxHP(int32 NewMaxHP)
{
	if (NewMaxHP <= 0)
	{
		this->MaxHP = 0;

	}
	else
	{
		this->MaxHP = NewMaxHP;
	}
	if (CurrentHP > this->MaxHP)
	{
		SetCurrentHP(this->MaxHP);
	}
	return MaxHP;
}

FVector2D AUFOBase::SetCurrentVelocity(const FVector2D NewCurrentVelocity)
{
	const float NewCurrentSpeed = NewCurrentVelocity.Size();
	CurrentVelocity = NewCurrentVelocity;
	SetCurrentSpeed(NewCurrentSpeed);
	return CurrentVelocity;
}

FVector2D AUFOBase::SetCurrentAcceleratedVelocity(const FVector2D NewCurrentAcceleratedVelocity)
{
	const float NewCurrentAcceleratedSpeed = NewCurrentAcceleratedVelocity.Size();
	CurrentAcceleratedVelocity = NewCurrentAcceleratedVelocity;
	SetCurrentAcceleratedSpeed(NewCurrentAcceleratedSpeed);
	return CurrentAcceleratedVelocity;
}

FVector2D AUFOBase::SetPlayerAcceleratedVelocity(const FVector2D NewPlayerAcceleratedVelocity)
{
	PlayerAcceleratedVelocity = NewPlayerAcceleratedVelocity;
	return PlayerAcceleratedVelocity;
}

void AUFOBase::SetWindAngle(int32 InWindAngle)
{
	WindAngle = InWindAngle - 40;
}

void AUFOBase::SetUFOLocationWithTeleport(const FVector NewLocation)
{
	FVector PlayerAOffSet = PlayerA->GetActorLocation() - GetActorLocation();
	FVector PlayerBOffSet = PlayerB->GetActorLocation() - GetActorLocation();


	SetActorLocation(NewLocation, false, nullptr, ETeleportType::ResetPhysics);

	PlayerA->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
	PlayerB->SetActorLocation(GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AUFOBase::SetPlayerLocationOffSet, PlayerAOffSet, PlayerBOffSet);;
	GetWorldTimerManager().SetTimerForNextTick(TimerDelegate);
	HandleFloatAndRotation(NewLocation, true);
}

void AUFOBase::SetUFOLocation(const FVector NewLocation)
{
	HandleFloatAndRotation(NewLocation, true);
}

FVector2D AUFOBase::SetSelfAcceleratedVelocity(const FVector2D NewSelfAcceleratedVelocity)
{
	SelfAcceleratedVelocity = NewSelfAcceleratedVelocity;
	return SelfAcceleratedVelocity;
}

float AUFOBase::SetCurrentAcceleratedSpeed(const float NewCurrentAcceleratedSpeed)
{
	if (NewCurrentAcceleratedSpeed < 0)
	{
		CurrentAcceleratedSpeed = 0;
	}
	else if (NewCurrentAcceleratedSpeed > MaxAcceleratedSpeed)
	{
		CurrentAcceleratedSpeed = MaxAcceleratedSpeed;
	}
	else
	{
		CurrentAcceleratedSpeed = NewCurrentAcceleratedSpeed;
	}
	CurrentAcceleratedVelocity = CurrentAcceleratedVelocity.GetSafeNormal() * CurrentAcceleratedSpeed;
	return CurrentAcceleratedSpeed;
}

float AUFOBase::SetMaxAcceleratedSpeed(const float NewMaxAcceleratedSpeed)
{
	if (NewMaxAcceleratedSpeed < 0)
	{
		MaxAcceleratedSpeed = 0;

	}
	else
	{
		MaxAcceleratedSpeed = NewMaxAcceleratedSpeed;
	}
	if (CurrentAcceleratedSpeed > MaxAcceleratedSpeed)
	{
		SetCurrentAcceleratedSpeed(MaxAcceleratedSpeed);
	}
	return MaxAcceleratedSpeed;
}


void AUFOBase::OnRep_MaxHP_Implementation()
{

}

void AUFOBase::GetViewTarget(float& ArmLength, FRotator& Rotator)
{
	if (USpringArmComponent* SpringArmComponent = Cast<USpringArmComponent>(GetComponentByClass(USpringArmComponent::StaticClass())))
	{
		ArmLength = SpringArmComponent->TargetArmLength;
		Rotator = SpringArmComponent->GetRelativeRotation();
	}
}

void AUFOBase::GetForwardRotator(FRotator& Rotator)
{
	if (USpringArmComponent* SpringArmComponent = Cast<USpringArmComponent>(GetComponentByClass(USpringArmComponent::StaticClass())))
	{
		Rotator = SpringArmComponent->GetRelativeRotation();
		SetForwardRotator(Rotator);
	}
}

void AUFOBase::SetForwardRotator(FRotator& Rotator)
{
	if (USpringArmComponent* SpringArmComponent = Cast<USpringArmComponent>(GetComponentByClass(USpringArmComponent::StaticClass())))
	{
		if (CurrentVelocity.X > 1e-3)
		{
			Rotator.Yaw = FMath::Atan(CurrentVelocity.Y / CurrentVelocity.X) * 180 / 3.14159265;
		}
		else if (CurrentVelocity.X < -1e-3)
		{
			Rotator.Yaw = FMath::Atan(CurrentVelocity.Y / CurrentVelocity.X) * 180 / 3.14159265 + 180;
		}
	}
}

void AUFOBase::UpdateViewTarget()
{
	if (USpringArmComponent* SpringArmComponent = Cast<USpringArmComponent>(GetComponentByClass(USpringArmComponent::StaticClass())))
	{
		FRotator Rotator = SpringArmComponent->GetRelativeRotation();
		SetForwardRotator(Rotator);

		ChangeViewTarget(SpringArmComponent->TargetArmLength, Rotator);
	}
}

FQuat AUFOBase::RotatorToQuat(FRotator Rotator)
{
	return Rotator.Quaternion();
}

FRotator AUFOBase::QuatToRotator(FQuat Quaternion)
{
	return Quaternion.Rotator();
}

FQuat AUFOBase::LerpQuat(FQuat A, FQuat B, float Alpha)
{
	return FMath::Lerp(A, B, Alpha);
}

float AUFOBase::SafeLerpAngle(float A, float B, float Alpha)
{
	float AngleA = A;
	float AngleB = B;
	while (AngleA < 0.0f) AngleA += 360.0f;
	while (AngleB < 0.0f) AngleB += 360.0f;
	while (AngleA >= 360.0f) AngleA -= 360.0f;
	while (AngleB >= 360.0f) AngleB -= 360.0f;

	if (FMath::Abs(AngleA - AngleB) > 180.0f)
	{
		if (AngleA > 180.0f) AngleA -= 360.0f;
		if (AngleB > 180.0f) AngleB -= 360.0f;
	}

	return FMath::Lerp(AngleA, AngleB, Alpha);
}

void AUFOBase::ChangeViewTarget_Implementation(float ArmLength, FRotator Rotator)
{
	if (USpringArmComponent* SpringArmComponent = Cast<USpringArmComponent>(GetComponentByClass(USpringArmComponent::StaticClass())))
	{
		SpringArmComponent->TargetArmLength = ArmLength;
		SpringArmComponent->SetRelativeRotation(Rotator);

		if (IsValid(PlayerA) && IsValid(PlayerB))
		{
			PlayerA->SetControlRotation(Rotator);
			PlayerB->SetControlRotation(Rotator);
		}
	}
}

void AUFOBase::OnRep_CurrentHP_Implementation()
{

}

void AUFOBase::OnRep_ShieldEnable_Implementation()
{

}

void AUFOBase::OnRep_SkillAEnable_Implementation()
{
}

void AUFOBase::OnRep_SkillBEnable_Implementation()
{

}

void AUFOBase::OnRep_WindAngle_Implementation()
{

}

void AUFOBase::AUFOBase::DealUFODeath()
{
	if (!bUFOEnable)
	{
		return;
	}
	bIsDeath = true;
	AProjectUFOGameMode* CurrentGameMode = Cast<AProjectUFOGameMode>(UGameplayStatics::GetGameMode(this));
	if (IsValid(CurrentGameMode))
	{
		CurrentGameMode->SetGameState(TEXT("Dead"));
	}
	SetUFOEnable(false);
	DealUFODeathMulti();
	//SetUFOEnable(false);
}

void AUFOBase::PlaySoundAtLocationMulti_Implementation(USoundBase* Sound, FVector Location)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, FRotator::ZeroRotator);
}

void AUFOBase::OnHitMulti_Implementation(FVector HitLocation)
{
	UGameplayStatics::SpawnEmitterAtLocation(this, HitFX, HitLocation, FRotator::ZeroRotator, FVector(5.0f));

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSoundCue, HitLocation, FRotator::ZeroRotator);
}

void AUFOBase::SpawnFXWithDestroy_Implementation(UParticleSystem* FX, float DestroyTime)
{

}

void AUFOBase::DealUFODeathMulti_Implementation()
{
	AProjectUFOGameMode* CurrentGameMode = Cast<AProjectUFOGameMode>(UGameplayStatics::GetGameMode(this));
	float RespawnTime = 0;
	float ShowUITime = 0;
	if (IsValid(CurrentGameMode))
	{
		RespawnTime = CurrentGameMode->RespawnTime;
		ShowUITime = CurrentGameMode->RespawnUIStartTime;
	}
	else
	{
		RespawnTime = 5;
		ShowUITime = 2;
	}
	SpawnFXWithDestroy(DeathFX, RespawnTime);
	DealUFODeathBlueprint(RespawnTime, ShowUITime);
}

void AUFOBase::DealUFODeathBlueprint_Implementation(float RespawnTime, float ShowUITime)
{

}

void AUFOBase::OnPlayerLogin(AProjectUFOCharacter* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("AUFOBase::OnPlayerLogin"));
	if (HasAuthority() && IsValid(Player))
	{
		Player->SetViewTarget();

		if (Player->PlayerTag == EPlayerTag::PlayerA)
		{
			PlayerA = Player;
			//技能绑定后续处理
			Player->GetSkillComponent()->GetProcedure("MakeShell")->ProcedureDelegate.AddUObject(this, &AUFOBase::ProcessSkillAStateChange);
		}
		else if (Player->PlayerTag == EPlayerTag::PlayerB)
		{
			PlayerB = Player;
			Player->GetSkillComponent()->GetProcedure("MakeShell")->ProcedureDelegate.AddUObject(this, &AUFOBase::ProcessSkillBStateChange);
		}
	}
}


// Called every frame
void AUFOBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority() && bUFOEnable && bPlayerAEnable && bPlayerBEnable)
	{
		if (bViewUpdate)
		{
			UpdateViewTarget();
		}

		UpdateLocation(DeltaTime);
		if (IsValid(PlayerA) && IsValid(PlayerB))
		{
			// PlayerA->SetViewTargetClient();
			// PlayerB->SetViewTargetClient();
			if (bIsProcessedByPlayer)
			{
				CalSpeedAndDirection();
			}
		}
	}

}

void AUFOBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUFOBase, CurrentHP);
	DOREPLIFETIME(AUFOBase, MaxHP);
	DOREPLIFETIME(AUFOBase, bShieldEnable);
	DOREPLIFETIME(AUFOBase, bSkillAEnable);
	DOREPLIFETIME(AUFOBase, bSkillBEnable);
	DOREPLIFETIME(AUFOBase, WindAngle);
}

void AUFOBase::HandleFloatAndRotation(FVector CurrentLocation, bool bIsOnPlantForm)
{
	//处理贴地
	TArray<AActor*> IgnoreActors;
	FHitResult Result;
	IgnoreActors.Add(this);
	IgnoreActors.Add(PlayerA);
	IgnoreActors.Add(PlayerB);


	const FVector TraceBeginPos = CurrentLocation + FVector(0, 0, 500);
	const FVector TraceEndPos = CurrentLocation + FVector(0, 0, -(3000 + FlyingHeight));

	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceBeginPos, TraceEndPos, UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false, IgnoreActors, EDrawDebugTrace::None, Result, true);


	if (Result.Actor.IsValid() || Result.Component.IsValid())
	{
		TargetZ = Result.Location.Z + FlyingHeight;
	}
	//移动平台不做Z平滑
	if (true || bIsOnPlantForm)
	{
		CurrentZ = TargetZ;
	}
	CurrentLocation.Z = CurrentZ;
	SetActorLocation(CurrentLocation, false);
	//处理坡面旋转
	FQuat Quat = FQuat::FindBetweenNormals(FVector(0, 0, 1), Result.ImpactNormal);
	FRotator SlopeRotator = Quat.Rotator();
	SlopeRotator.Roll = FMath::ClampAngle(SlopeRotator.Roll, -MaxTiltAngle, MaxTiltAngle);
	SlopeRotator.Pitch = FMath::ClampAngle(SlopeRotator.Pitch, -MaxTiltAngle, MaxTiltAngle);
	SetActorRotation(SlopeRotator);
	SetActorRotationWithVelocity();
}

void AUFOBase::SetPlayerLocationOffSet(FVector PlayerAOffSet, FVector PlayerBOffset)
{
	PlayerA->SetActorLocation(GetActorLocation() + PlayerAOffSet, false, nullptr, ETeleportType::ResetPhysics);
	PlayerB->SetActorLocation(GetActorLocation() + PlayerBOffset, false, nullptr, ETeleportType::ResetPhysics);
}



void AUFOBase::SetActorRotationWithVelocity_Implementation()
{

}
