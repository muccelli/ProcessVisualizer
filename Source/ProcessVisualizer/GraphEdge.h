// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PROCESSVISUALIZER_API GraphEdge
{
private:
	
public:
	FString fromNode;
	FString toNode;
	FString label;
	bool toInvert;

	GraphEdge(FString from, FString to);
	GraphEdge(FString from, FString to, FString label, bool toInvert);
	GraphEdge(FString from, FString to, bool toInvert);
	~GraphEdge();

	FString GetFromNode();
	FString GetToNode();
	FString GetLabel();
	bool IsToInvert();

	bool Equals(GraphEdge edge);
	FORCEINLINE bool operator==(const GraphEdge &edge) const
	{
		return this->fromNode == edge.fromNode
			&& this->toNode == edge.toNode
			&& this->toInvert == edge.toInvert;
	}
};
