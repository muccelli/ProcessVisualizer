// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Classes/Components/SplineComponent.h"
#include "NodeActor.h"
#include "EdgeActor.generated.h"

UCLASS()
class PROCESSVISUALIZER_API AEdgeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEdgeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* Spline;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FText WidgetText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ANodeActor* FromNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ANodeActor* ToNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Significance;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetEdgeProperties();

};
