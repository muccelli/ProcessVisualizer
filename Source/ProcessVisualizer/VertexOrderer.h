// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Graph.h"
#include "GraphEdge.h"

/**
 * 
 */
class PROCESSVISUALIZER_API VertexOrderer
{
private:
	Graph graph;
	TArray<TArray<FString>> layers;

public:
	VertexOrderer(Graph graph, TArray<TArray<FString>> layers);
	~VertexOrderer();

	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> OrderVertices();
	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> CreateVirtualVerticesAndEdges(TArray<TArray<FString>> layers, TArray<GraphEdge> edges);
	int32 GetLayerNumber(FString node, TArray<TArray<FString>> layers);

	TArray<TArray<FString>> MedianHeuristic(TArray<TArray<FString>> layers, TArray<GraphEdge> edges, int32 i);
	TArray<TArray<FString>> Transpose(TArray<TArray<FString>> layers, TArray<GraphEdge> edges);
	int32 Crossing(TArray<TArray<FString>> layers, TArray<GraphEdge> edges);
	int32 Crossing(TArray<TArray<FString>> layers, TArray<GraphEdge> edges, FString n1, FString n2, int32 layerIndex);
	int32 MedianValue(TArray<int32> P);
	TArray<FString> SortRank(TArray<FString> layer, TMap<FString, int32> median);
};
