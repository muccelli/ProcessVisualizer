// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GraphEdge.h"

/**
 * 
 */
class PROCESSVISUALIZER_API Graph
{
private:
	TSet<FString> nodes;
	TArray<GraphEdge> edges;
public:
	Graph();
	Graph(TArray<GraphEdge> edges);
	~Graph();
	TArray<GraphEdge> GetEdges();
	TSet<FString> GetNodes();
};
