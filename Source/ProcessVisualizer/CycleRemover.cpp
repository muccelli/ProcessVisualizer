// Fill out your copyright notice in the Description page of Project Settings.


#include "CycleRemover.h"

void CycleRemover::DFSRemove(FString node)
{
	if (this->marked.Contains(node))
	{
		return;
	}

}

CycleRemover::CycleRemover(Graph graph)
{
	this->graph = graph;
	this->edges = graph.GetEdges();
}

CycleRemover::~CycleRemover()
{
}

Graph CycleRemover::RemoveCycles()
{
	for (FString node : this->graph.GetNodes())
	{
		DFSRemove(node);
	}
	return Graph(this->edges);
}
