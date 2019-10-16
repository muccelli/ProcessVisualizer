// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "JsonUtilities/Public/JsonUtilities.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputParser.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EScalingMethod : uint8
{
	VE_Discrete 	UMETA(DisplayName = "Discrete"),
	VE_Continuous 	UMETA(DisplayName = "Continuous"),
	VE_MinMax		UMETA(DisplayName = "MinMax")
};

UCLASS()
class PROCESSVISUALIZER_API AInputParser : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInputParser();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TArray<TArray<FString>> GetNodesPositioning(TArray<TSharedPtr<FJsonValue>>, TArray<TSharedPtr<FJsonValue>>);

	FVector ComputeMiddleSplinePointLocation(FVector startSplinePointLocation, FVector endSplinePointLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InputJSON;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ANodeActor> NodeActorBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AEdgeActor> EdgeActorBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
	EScalingMethod ScalingMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NodesYDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NodesZDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XNodeDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MiddleSplinePointDeviation;
};
