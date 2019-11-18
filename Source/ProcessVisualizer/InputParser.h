// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "JsonUtilities/Public/JsonUtilities.h"

#include "CoreMinimal.h"
#include "Graph.h"
#include "GraphEdge.h"
#include "CycleRemover.h"
#include "LayerAssigner.h"
#include "VertexOrderer.h"
#include "GameFramework/Actor.h"
#include "InputParser.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EScalingMethod : uint8
{
	VE_Discrete 	UMETA(DisplayName = "Discrete"),
	VE_Continuous 	UMETA(DisplayName = "Continuous"),
	VE_MinMax		UMETA(DisplayName = "MinMax")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EVisualizationType : uint8
{
	VE_Horizontal 	UMETA(DisplayName = "Horizontal"),
	VE_Vertical 	UMETA(DisplayName = "Vertical"),
	VE_HorizontalImproved UMETA(DisplayName = "HorizontalImproved")
};

USTRUCT(BlueprintType)
struct FValuesToFrequencyMap
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ValuesTofrequency")
	TMap<FString, FString> attrValuesToFrequency;

	void AddToMap(FString key, FString value)
	{
		attrValuesToFrequency.Add(key, value);
	}

	TMap<FString, FString> GetValuesToFrequency()
	{
		return attrValuesToFrequency;
	}

	void SortByValue()
	{
		if (!attrValuesToFrequency.Contains("Total") && !attrValuesToFrequency.Contains("Mean") && !attrValuesToFrequency.Contains("Median") && !attrValuesToFrequency.Contains("Min") && !attrValuesToFrequency.Contains("Max"))
		{
			attrValuesToFrequency.ValueSort([](const FString& A, const FString& B) {
				return FCString::Atof(*A) > FCString::Atof(*B);
			});
		}
	}

	TMap<FString, FString> GetValuesToPercentage()
	{
		TMap<FString, FString> valuesToPercentage = TMap<FString, FString>();
		int32 totalFrequency = 0;
		for (auto pair : attrValuesToFrequency)
		{
			totalFrequency += FCString::Atoi(*pair.Value);
		}

		for (auto pair : attrValuesToFrequency)
		{
			valuesToPercentage.Add(pair.Key, FString::SanitizeFloat((FCString::Atof(*pair.Value) / totalFrequency) * 100) + "%");
		}
		return valuesToPercentage;
	}

	FValuesToFrequencyMap()
	{
		attrValuesToFrequency = TMap<FString, FString>();
	}

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

	void CreateVerticalGraph(TSharedPtr<FJsonObject> &JsonObject, bool &retflag);

	void CreateHorizontalGraph(TSharedPtr<FJsonObject> &JsonObject, bool &retflag);

	void CreateHorizontalImprovedGraph(TSharedPtr<FJsonObject> &JsonObject, bool &retflag);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TArray<TArray<FString>> GetNodesPositioning(TArray<TSharedPtr<FJsonValue>> nodesArray, TArray<TSharedPtr<FJsonValue>> edgesArray);
	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> GetNodesPositioningImproved(TArray<TSharedPtr<FJsonValue>> nodesArray, TArray<TSharedPtr<FJsonValue>> edgesArray);

	FVector ComputeMiddleSplinePointLocation(FVector startSplinePointLocation, FVector endSplinePointLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InputJSON;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ANodeActor> NodeActorBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AEdgeActor> EdgeActorBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
	EScalingMethod ScalingMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum)
	EVisualizationType VisualizationType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NodesYDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NodesZDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XNodeDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MiddleSplinePointDeviation;
};
