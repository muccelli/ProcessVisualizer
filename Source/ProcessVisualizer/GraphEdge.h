// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PROCESSVISUALIZER_API GraphEdge
{
private:
	FString fromNode;
	FString toNode;
	FString label;
public:
	GraphEdge(FString from, FString to);
	~GraphEdge();

	FString GetFromNode();
	FString GetToNode();
	FString GetLabel();

	bool Equals(GraphEdge edge);
};
