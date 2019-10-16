// Fill out your copyright notice in the Description page of Project Settings.


#include "EdgeActor.h"

// Sets default values
AEdgeActor::AEdgeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;
}

// Called when the game starts or when spawned
void AEdgeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEdgeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEdgeActor::SetEdgeProperties_Implementation()
{
}

