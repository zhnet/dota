// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUFOCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "Utils/GameStateFramework.h"
#include "ProjectUFOGameMode.generated.h"

class AUFOBase;
class APlayerStart;

class AProjectUFOCharacter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerPostLoginDelegate, AProjectUFOCharacter*, Player);

UCLASS()
class AProjectUFOGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = wind)
	float WindForce = 400;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = wind)
	float MaxWindDuration = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = wind)
	float MinWindDuration = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = wind)
	int32 LastWindAngle = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = wind)
	int32 TargetWindAngle =0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = wind)
	float WindChangeTime = 0.5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = wind)
	bool bEnableWind = false;
	FTimerHandle WindTimerHandle;

	float CurrentTime;

	AUFOBase* CurrentUFO;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StartTag)
	TArray<FString> LevelStartTags;

	UPROPERTY(Transient)
	APlayerStart* CurrentPlayerStart = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	FVector PlayerSpawnPositionOffSet = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	AProjectUFOCharacter* PlayerA = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	AProjectUFOCharacter* PlayerB = nullptr;
	
	int32 CurrentPlayerNum = 0;

	float TimeRemained = 0.0f;

	bool bRestart = true;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable)
	void PostLoginInBluePrint(APlayerController* NewPlayer);

	FVector GetSpawnOffset(EPlayerTag PlayerTag);
public:

	UFUNCTION(BlueprintNativeEvent)
	void DealCharacterWithDelay(float DelayTime);

	UFUNCTION(BlueprintNativeEvent)
	void DealSingleCharacterWithDelay(ACharacter* Character, float DelayTime);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	float RespawnTime = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	float RespawnUFOStartTime = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	float RespawnUIStartTime = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player)
	FVector OffsetLimit = FVector(400, 400, 400);
	
	AProjectUFOGameMode();

	AUFOBase* GetCurrentUFO() { return CurrentUFO; }
	void SetCurrentUFO(AUFOBase* InCurrentUFO) { CurrentUFO = InCurrentUFO; }

	void SetRandTargetWindAngle();
	
	void SetCurrentWindForce(float DeltaSeconds);

	void ClearWindForce();

	UFUNCTION(BlueprintCallable)
	void SetRestart(bool bInRestart) { bRestart = bInRestart; }

	UFUNCTION(BlueprintCallable)
	bool GetRestart() { return bRestart; }

	UFUNCTION(BlueprintCallable)
	void SetPlayerStart(const FString& Tag);

	UFUNCTION(BlueprintCallable)
	bool SetGameState(const FString& GameStateClassName);

	UFUNCTION(BlueprintCallable)
	bool RestartGame();

	UFUNCTION(BlueprintCallable)
	bool ResetPlayers();

	UFUNCTION(BlueprintCallable)
	bool ResetRequired();

	void SetTimer(float TimeInput) { TimeRemained = TimeInput; }
	bool IsTimeOut() { return TimeRemained < 0; }

	bool PostResetPlayers();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable)
	FPlayerPostLoginDelegate PlayerPostLoginDelegate;

	UPROPERTY(Transient)
	AGameStateFramework* GameStateFramework;
};