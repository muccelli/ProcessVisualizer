// Fill out your copyright notice in the Description page of Project Settings.


#include "GraphEdge.h"

GraphEdge::GraphEdge(FString from, FString to)
{
	this->fromNode = from;
	this->toNode = to;
	this->label = from + " -> " + to;
}

GraphEdge::~GraphEdge()
{
}

FString GraphEdge::GetFromNode()
{
	return this->fromNode;
}

FString GraphEdge::GetToNode()
{
	return this->toNode;
}

FString GraphEdge::GetLabel()
{
	return this->label;
}

bool GraphEdge::Equals(GraphEdge edge)
{
	return this->fromNode == edge.GetFromNode()
		&& this->toNode == edge.GetToNode();
}
