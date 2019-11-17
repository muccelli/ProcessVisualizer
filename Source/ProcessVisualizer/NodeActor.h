// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "InputParser.h"
#include "NodeActor.generated.h"

UCLASS()
class PROCESSVISUALIZER_API ANodeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANodeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FValuesToFrequencyMap> Attributes;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, float> Durations;

	UPROPERTY()
	USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText WidgetText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Significance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SignificanceScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PercentageFrequencyToTotal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeScale;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetNodeLabelAndTransform();
};
