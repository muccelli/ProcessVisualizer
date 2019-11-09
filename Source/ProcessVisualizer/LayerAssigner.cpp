// Fill out your copyright notice in the Description page of Project Settings.


#include "LayerAssigner.h"

LayerAssigner::LayerAssigner(Graph graph)
{
	this->graph = graph;
}

LayerAssigner::~LayerAssigner()
{
}

TArray<TArray<FString>> LayerAssigner::assignLayers()
{
	TArray<TArray<FString>> sorted;
	TArray<GraphEdge> edg = TArray<GraphEdge>(this->graph.GetEdges());
	TSet<FString> nod = TSet<FString>(this->graph.GetNodes());
	TArray<FString> start = this->GetVerticesWithoutIncomingEdges(edg, nod);
	while (start.Num() > 0)
	{
		sorted.Emplace(start);
		for (GraphEdge ge : edg)
		{
			if (start.Contains(ge.GetFromNode()))
			{
				edg.Remove(ge);
			}
		}
		for (FString fs : nod)
		{
			if (start.Contains(fs))
			{
				nod.Remove(fs);
			}
		}
		start = this->GetVerticesWithoutIncomingEdges(edg, nod);
	}

	return sorted;
}

TArray<FString> LayerAssigner::GetVerticesWithoutIncomingEdges(TArray<GraphEdge> edg, TSet<FString> nod)
{
	TArray<FString> nodesWOIncomingEdges = TArray<FString>();
	bool hasInEdges;
	for (FString fs : nod)
	{
		hasInEdges = false;
		for (GraphEdge ge : edg)
		{
			if (ge.GetToNode().Equals(fs))
			{
				hasInEdges = true;
				break;
			}
		}
		if (!hasInEdges)
		{
			nodesWOIncomingEdges.Add(fs);
		}
	}

	return nodesWOIncomingEdges;
}
