// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Graph.h"
#include "GraphEdge.h"

/**
 * 
 */
class PROCESSVISUALIZER_API LayerAssigner
{
private:
	Graph graph;
public:
	LayerAssigner(Graph graph);
	~LayerAssigner();

	TArray<TArray<FString>> assignLayers();
	TArray<FString> GetVerticesWithoutIncomingEdges(TArray<GraphEdge> edg, TSet<FString> nod);
};
