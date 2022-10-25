// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectUFOGameMode.h"

#include "EngineUtils.h"
#include "ProjectUFOCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "UFOBase.h"
#include "ProjectUFOPrivate.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

#include "Utils/ProceduralComponent.h"
#include "Utils/GameStateFramework.h"
#include "ProjectUFOPrivate.h"

DEFINE_LOG_CATEGORY(LOGUFORuntime);


void AProjectUFOGameMode::SetRandTargetWindAngle()
{
	LastWindAngle = TargetWindAngle;
	TargetWindAngle = FMath::RandRange(0, 360);
	CurrentTime = 0;
	float WindDuration = FMath::RandRange(MinWindDuration, MaxWindDuration);
	GetWorldTimerManager().SetTimer(WindTimerHandle, this, &AProjectUFOGameMode::SetRandTargetWindAngle, WindDuration, false, 5);
}

void AProjectUFOGameMode::SetCurrentWindForce(float DeltaSeconds)
{
	if (bEnableWind)
	{
		CurrentTime += DeltaSeconds;
		
		if (IsValid(CurrentUFO))
		{
			float TimeRate = FMath::Min<float>((CurrentTime / WindChangeTime), 1);
			int32 CurrentAngle = LastWindAngle + (TargetWindAngle - LastWindAngle) * TimeRate;
			FVector2D NewSelfAcceleratedVelocity = FVector2D(FMath::Cos(FMath::DegreesToRadians(CurrentAngle)), FMath::Sin(FMath::DegreesToRadians(CurrentAngle))) * WindForce;
			CurrentUFO->SetSelfAcceleratedVelocity(NewSelfAcceleratedVelocity);
			CurrentUFO->SetWindAngle(CurrentAngle);
		}
	}
}

void AProjectUFOGameMode::ClearWindForce()
{
	if (IsValid(CurrentUFO))
	{
		CurrentUFO->SetSelfAcceleratedVelocity(FVector2D::ZeroVector);
	}
}

void AProjectUFOGameMode::SetPlayerStart(const FString& Tag)
{
	UWorld* World = GetWorld();

	if (!Tag.IsEmpty())
	{
		const FName IncomingPlayerStartTag = FName(*Tag);
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* Start = *It;
			if (Start && Start->PlayerStartTag == IncomingPlayerStartTag)
			{
				CurrentPlayerStart = Start;
			}
		}
	}
}

bool AProjectUFOGameMode::SetGameState(const FString& GameStateClassName)
{
	return GameStateFramework->ChangeStateTo(GameStateClassName);
}

bool AProjectUFOGameMode::RestartGame()
{
	return SetGameState(TEXT("Restart"));
}

bool AProjectUFOGameMode::ResetPlayers()
{
	if (CurrentUFO  && PlayerA && PlayerB)
	{
		CurrentUFO->SetTopBoxCollision(false);
		if (bRestart && CurrentPlayerStart)
		{
			CurrentUFO->SetUFOLocationWithTeleport(CurrentPlayerStart->GetActorLocation());
			CurrentUFO->ResetUFO();
		}

		CurrentUFO->SetCurrentVelocity(FVector2D(0.0f, 0.0f));

		PlayerA->SetActorLocation(CurrentUFO->GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
		PlayerB->SetActorLocation(CurrentUFO->GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
		
		return true;
	}
	else return false;
}

bool AProjectUFOGameMode::ResetRequired()
{
	if (PlayerA && PlayerB && CurrentUFO)
	{
		FVector PlayerAOffset = PlayerA->GetActorLocation() - CurrentUFO->GetActorLocation();
		FVector PlayerBOffset = PlayerB->GetActorLocation() - CurrentUFO->GetActorLocation();
		bool bReset = false;
		if(abs(PlayerAOffset.X) > OffsetLimit.X || abs(PlayerAOffset.Y) > OffsetLimit.Y || abs(PlayerAOffset.Z) > OffsetLimit.Z)
		{
			bReset = true;
		}
		else if(abs(PlayerBOffset.X) > OffsetLimit.X || abs(PlayerBOffset.Y) > OffsetLimit.Y || abs(PlayerBOffset.Z) > OffsetLimit.Z)
		{
			bReset = true;
		}
		
		bRestart = !bReset;
		return bReset;
	}
	return false;
}

bool AProjectUFOGameMode::PostResetPlayers()
{
	if (CurrentUFO && PlayerA && PlayerB )
	{
		CurrentUFO->SetCurrentVelocity(FVector2D(0.0f, 0.0f));

		PlayerA->SetActorLocation(CurrentUFO->GetActorLocation() + GetSpawnOffset(EPlayerTag::PlayerA), false, nullptr, ETeleportType::ResetPhysics);
		PlayerB->SetActorLocation(CurrentUFO->GetActorLocation() + GetSpawnOffset(EPlayerTag::PlayerB), false, nullptr, ETeleportType::ResetPhysics);
		CurrentUFO->SetTopBoxCollision(true);
		return true;
	}
	else return false;
}

void AProjectUFOGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//分配玩家Tag

	AProjectUFOCharacter* PlayerCharacter = Cast<AProjectUFOCharacter>(NewPlayer->GetPawn());
	if(IsValid(PlayerCharacter))
	{
		CurrentUFO =  Cast<AUFOBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AUFOBase::StaticClass()));
		if(IsValid(CurrentUFO))
		{
			if (!PlayerA)
			{
				PlayerA = PlayerCharacter;
				PlayerA->PlayerTag = EPlayerTag::PlayerA;
				PlayerA->SetActorLocation(CurrentUFO->GetActorLocation() + GetSpawnOffset(EPlayerTag::PlayerA));		//处理玩家出生位置偏移与镜头
				SetPause(NewPlayer);
				DealSingleCharacterWithDelay(PlayerA, 0.01);
			}
			else if (!PlayerB)
			{
				PlayerB = PlayerCharacter;
				PlayerB->PlayerTag = EPlayerTag::PlayerB;
				PlayerB->SetActorLocation(CurrentUFO->GetActorLocation() + GetSpawnOffset(EPlayerTag::PlayerB));		//处理玩家出生位置偏移与镜头
				ClearPause();

				PlayerA->SetGameTeamMate(PlayerB);
				PlayerB->SetGameTeamMate(PlayerA);
				DealSingleCharacterWithDelay(PlayerB, 0.01);

				DealCharacterWithDelay(1);
			}
			
			//当前绑定有问题 后续排查
			//NewPlayer->SetViewTarget(CurrentUFO);
			// FTimerHandle CameraBindHandle;
			// FTimerDelegate MyDelegate;
			// MyDelegate.BindLambda([=]()
			// {
			// 	NewPlayer->SetViewTargetWithBlend(CurrentUFO,0.5);
			// });
			// GetWorldTimerManager().SetTimer(CameraBindHandle, MyDelegate, 3.0, false);

			PlayerPostLoginDelegate.Broadcast(PlayerCharacter);
		}
	}
}

//供单机测试
void AProjectUFOGameMode::PostLoginInBluePrint(APlayerController* NewPlayer)
{
	if(!IsValid(NewPlayer))
	{
		return;
	}
	
	AProjectUFOCharacter* PlayerCharacter = Cast<AProjectUFOCharacter>(NewPlayer->GetPawn());
	if(IsValid(PlayerCharacter))
	{
		CurrentUFO =  Cast<AUFOBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AUFOBase::StaticClass()));
		if(IsValid(CurrentUFO))
		{
			if (!PlayerA)
			{
				PlayerA = PlayerCharacter;
				PlayerA->PlayerTag = EPlayerTag::PlayerA;
				PlayerA->SetActorLocation(CurrentUFO->GetActorLocation() + GetSpawnOffset(EPlayerTag::PlayerA));		//处理玩家出生位置偏移与镜头
			}
			else if (!PlayerB)
			{
				PlayerB = PlayerCharacter;
				PlayerB->PlayerTag = EPlayerTag::PlayerB;
				PlayerB->SetActorLocation(CurrentUFO->GetActorLocation() + GetSpawnOffset(EPlayerTag::PlayerB));		//处理玩家出生位置偏移与镜头

				PlayerA->SetGameTeamMate(PlayerB);
				PlayerB->SetGameTeamMate(PlayerA);
			}
			
			//当前绑定有问题 后续排查
			//NewPlayer->SetViewTarget(CurrentUFO);
			// FTimerHandle CameraBindHandle;
			// FTimerDelegate MyDelegate;
			// MyDelegate.BindLambda([=]()
			// {
			// 	NewPlayer->SetViewTargetWithBlend(CurrentUFO,0.5);
			// });
			// GetWorldTimerManager().SetTimer(CameraBindHandle, MyDelegate, 3.0, false);

			PlayerPostLoginDelegate.Broadcast(PlayerCharacter);
		}
	}
}

FVector AProjectUFOGameMode::GetSpawnOffset(EPlayerTag PlayerTag)
{
	if(PlayerTag == EPlayerTag::PlayerA)
	{
		return  PlayerSpawnPositionOffSet;
	}
	if(PlayerTag == EPlayerTag::PlayerB)
	{
		FVector TempOffSet =  -PlayerSpawnPositionOffSet;
		TempOffSet.Z = PlayerSpawnPositionOffSet.Z;
		return TempOffSet;
	}
	return FVector::ZeroVector;
}

void AProjectUFOGameMode::DealSingleCharacterWithDelay_Implementation(ACharacter* Character, float DelayTime)
{
	
}

void AProjectUFOGameMode::DealCharacterWithDelay_Implementation(float DelayTime)
{
	
}

AProjectUFOGameMode::AProjectUFOGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Character/Player/BP_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AProjectUFOGameMode::BeginPlay()
{
	Super::BeginPlay();
	float WindDuration = FMath::RandRange(MinWindDuration, MaxWindDuration);
	GetWorldTimerManager().SetTimer(WindTimerHandle, this, &AProjectUFOGameMode::SetRandTargetWindAngle, WindDuration, false, 5);
	GameStateFramework = Cast<AGameStateFramework>(GWorld->SpawnActor(AGameStateFramework::StaticClass()));

	AWindState* WindState = Cast<AWindState>(GWorld->SpawnActor(AWindState::StaticClass()));
	ARestartState* RestartState = Cast<ARestartState>(GWorld->SpawnActor(ARestartState::StaticClass()));
	APlayState* PlayState = Cast<APlayState>(GWorld->SpawnActor(APlayState::StaticClass()));
	ADeadState* DeadState = Cast<ADeadState>(GWorld->SpawnActor(ADeadState::StaticClass()));

	WindState->SetClassName(TEXT("Wind"));
	RestartState->SetClassName(TEXT("Restart"));
	PlayState->SetClassName(TEXT("Play"));
	DeadState->SetClassName(TEXT("Dead"));

	GameStateFramework->AddGameState(WindState);
	GameStateFramework->AddGameState(RestartState);
	GameStateFramework->AddGameState(PlayState);
	GameStateFramework->AddGameState(DeadState);


	SetGameState("Restart");
}

void AProjectUFOGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	GameStateFramework->OnExecute(DeltaSeconds);
	if (ResetRequired()) SetGameState(TEXT("Restart"));
	if (TimeRemained > 0.0f) TimeRemained -= DeltaSeconds;
}
