// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Graph.h"
#include "GraphEdge.h"

/**
 * 
 */
class PROCESSVISUALIZER_API CycleRemover
{
private:
	Graph graph;
	TSet<FString> stack;
	TSet<FString> marked;
	TArray<GraphEdge> edges;

	void DFSRemove(FString node);
public:
	CycleRemover(Graph graph);
	~CycleRemover();

	Graph RemoveCycles();
};
