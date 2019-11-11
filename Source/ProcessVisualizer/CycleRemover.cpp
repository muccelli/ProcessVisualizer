// Fill out your copyright notice in the Description page of Project Settings.


#include "CycleRemover.h"

void CycleRemover::DFSRemove(FString node)
{
	if (this->marked.Contains(node))
	{
		return;
	}
	this->marked.Add(node);
	this->stack.Add(node);
	for (GraphEdge ge : graph.GetOutgoingEdgesFor(node))
	{
		if (this->stack.Contains(ge.GetToNode()))
		{
			TArray<GraphEdge> edgesToRemove = TArray<GraphEdge>();
			TArray<GraphEdge> edgesToAdd = TArray<GraphEdge>();
			for (GraphEdge edge : this->edges)
			{
				if (edge.Equals(ge))
				{
					edgesToAdd.Add(GraphEdge(edge.GetToNode(), edge.GetFromNode(), true));
					edgesToRemove.Add(edge);
					break;
				}
			}
			GLog->Log("Edges to add: " + FString::FromInt(edgesToAdd.Num()));
			GLog->Log("Edges to remove: " + FString::FromInt(edgesToRemove.Num()));
			for (GraphEdge edge : edgesToAdd)
			{
				this->edges.Add(edge);
			}
			for (GraphEdge edge : edgesToRemove)
			{
				this->edges.Remove(edge);
			}
		}
		else if (!this->marked.Contains(ge.GetToNode()))
		{
			this->DFSRemove(ge.GetToNode());
		}
	}
	this->stack.Remove(node);
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
	Graph g = Graph(this->edges);
	/*for (GraphEdge ge : g.GetEdges())
	{
		GLog->Log("To invert?");
		GLog->Log(ge.IsToInvert() ? TEXT("true") : TEXT("false"));
	}*/
	return g;
}
