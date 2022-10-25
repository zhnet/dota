// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary1.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUFO_API UMyBlueprintFunctionLibrary1 : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Server Function")
        static FString Connected(FString str);
	UFUNCTION(BlueprintCallable, Category = "Server Function")
		static void DisConnected(FString str);
};
