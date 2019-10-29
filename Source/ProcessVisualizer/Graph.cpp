// Fill out your copyright notice in the Description page of Project Settings.


#include "Graph.h"

Graph::Graph()
{
}

Graph::Graph(TArray<GraphEdge> edges)
{
	this->edges = edges;
	for (GraphEdge ge : this->edges)
	{
		this->nodes.Add(ge.GetFromNode());
		this->nodes.Add(ge.GetToNode());
	}
}

Graph::~Graph()
{
}

TArray<GraphEdge> Graph::GetEdges()
{
	return this->edges;
}

TSet<FString> Graph::GetNodes()
{
	return this->nodes;
}
