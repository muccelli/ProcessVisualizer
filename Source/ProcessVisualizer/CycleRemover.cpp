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
			for (GraphEdge edge : this->edges)
			{
				if (edge.Equals(ge))
				{
					this->edges.Add(GraphEdge(edge.GetToNode(), edge.GetFromNode(), true));
					this->edges.Remove(edge);
					break;
				}
			}
		}
		else if (!this-marked.Contains(ge.GetToNode()))
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
	return Graph(this->edges);
}
