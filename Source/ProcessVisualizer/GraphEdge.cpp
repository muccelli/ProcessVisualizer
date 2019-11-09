// Fill out your copyright notice in the Description page of Project Settings.


#include "GraphEdge.h"

GraphEdge::GraphEdge(FString from, FString to)
{
	this->fromNode = from;
	this->toNode = to;
	this->label = from + " -> " + to;
	this->toInvert = false;
}

GraphEdge::GraphEdge(FString from, FString to, FString label)
{
	this->fromNode = from;
	this->toNode = to;
	this->label = label;
	this->toInvert = false;
}

GraphEdge::GraphEdge(FString from, FString to, bool toInvert)
{
	this->fromNode = from;
	this->toNode = to;
	this->label = from + " -> " + to;
	this->toInvert = toInvert;
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

bool GraphEdge::IsToInvert()
{
	return this->toInvert;
}

bool GraphEdge::Equals(GraphEdge edge)
{
	return this->fromNode == edge.GetFromNode()
		&& this->toNode == edge.GetToNode()
		&& this->toInvert == edge.IsToInvert();
}
