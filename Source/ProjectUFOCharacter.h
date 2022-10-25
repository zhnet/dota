// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectUFOCharacter.generated.h"

enum class EProceduralState;
class FProceduralComponent;
class USpringArmComponent;
class UCameraComponent;

UENUM(BlueprintType)
enum class EPlayerTag : uint8
{
	PlayerA,
	PlayerB
};

UCLASS(Blueprintable, config=Game)
class AProjectUFOCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AProjectUFOCharacter();

	virtual void PostInitializeComponents() override;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	// Custom Functions @Jefflian
	void OnMakeShell(EProceduralState State);
	void OnShelled(EProceduralState State, float RemainTime);

	void OnStartDizzy(EProceduralState State);
	void OnDizzying(EProceduralState State, float RemainTime);

	void TriggerMakeShell();
	void ReleaseMakeShell();
	void ProcessMakeShell();

	UFUNCTION()
	void OnActorBump(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
	void ProcessDizzy();

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_PlayerTag();

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_IsPauseByOther();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void ShowGuidUIMulti(int32 Index);

	UFUNCTION(BlueprintNativeEvent)
	void ShowGuidUIBluePrint(int32 Index);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void EnableUFO();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ReStartGame();
	
	void SetViewTarget();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void SetViewTargetClient();

	UFUNCTION(Client, Reliable)
	void SetControlRotation(FRotator Rotator);
	
	bool IsSkillTriggered();

	void SetGameTeamMate(AProjectUFOCharacter* InGameTeamMate) { GameTeamMate = InGameTeamMate; }

	virtual void GetLifetimeReplicatedProps( TArray< class FLifetimeProperty > & OutLifetimeProps ) const override;

	TSharedPtr<FProceduralComponent> GetSkillComponent() { return SkillComponent; }

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void OnTriggerSkill();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void OnReleaseSkill();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void OnGamePause();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void OnGameContinue();

	void PauseTeamMate();
	void ContinueTeamMate();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void GamePause();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void GameContinue();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void ChangeSkeletalMeshClient();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ShowLoadingUI(bool State);

	UFUNCTION(BlueprintNativeEvent)
	void ShowLoadingUIBluePrint(bool State);
	
	UFUNCTION(BlueprintNativeEvent)
	void ChangeSkeletalMesh();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void SetWidgetVisibility(const TArray<bool>& Visibility);

	UFUNCTION(BlueprintNativeEvent)
	void SetWidgetVisibilityBlueprint(const TArray<bool>& Visibility);

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_PlayerTag)
	EPlayerTag PlayerTag = EPlayerTag::PlayerA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_SkillEnergy, Category = Skill)
	float SkillEnergy = 1.0f;

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_SkillEnergy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsPauseByOther)
	bool IsPauseByOther = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill)
	float ShellTriggerTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill)
	float ShellTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill)
	float ShellCD = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill)
	float CollideMinVelocity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill)
	float LaunchStrength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill)
	float DizzyTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Skill)
	float FreezeTime = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skill)
	bool ReleaseAllowed = true;

private:

	AProjectUFOCharacter*					GameTeamMate;

	TSharedPtr<FProceduralComponent>		SkillComponent;
	bool									bSkillTriggered;

	FVector									LaunchPower;
	bool									EnableToMove;
};

