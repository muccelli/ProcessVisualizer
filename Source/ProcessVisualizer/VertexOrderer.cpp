// Fill out your copyright notice in the Description page of Project Settings.


#include "VertexOrderer.h"

VertexOrderer::VertexOrderer(Graph graph, TArray<TArray<FString>> layers)
{
	this->graph = graph;
	this->layers = layers;
}

VertexOrderer::~VertexOrderer()
{
}

TPair<TArray<TArray<FString>>, TArray<GraphEdge>> VertexOrderer::OrderVertices()
{
	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> bestOrder = TPair<TArray<TArray<FString>>, TArray<GraphEdge>>();
	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> virtualized = CreateVirtualVerticesAndEdges(this->layers, this->graph.GetEdges());
	TArray<TArray<FString>> layersVirtualized = virtualized.Key;
	TArray<GraphEdge> edges = virtualized.Value;

	TArray<TArray<FString>> best = layersVirtualized;
	for (int32 i = 0; i < 24; i++)
	{
		layersVirtualized = MedianHeuristic(layersVirtualized, edges, i);
		int32 nCrossingVirtualized = Crossing(layersVirtualized, edges);
		GLog->Log(FString::FromInt(nCrossingVirtualized));

		layersVirtualized = Transpose(layersVirtualized, edges);
		nCrossingVirtualized = Crossing(layersVirtualized, edges);
		int32 nCrossingBest = Crossing(best, edges);
		GLog->Log(FString::FromInt(nCrossingVirtualized) + " > " + FString::FromInt(nCrossingBest));
		if (nCrossingBest > nCrossingVirtualized)
		{
			best = layersVirtualized;
		}
		else
		{
			layersVirtualized = best;
		}
	}
	bestOrder.Key = best;
	bestOrder.Value = edges;
	return bestOrder;
}

TPair<TArray<TArray<FString>>, TArray<GraphEdge>> VertexOrderer::CreateVirtualVerticesAndEdges(TArray<TArray<FString>> layersVirtualized, TArray<GraphEdge> edges)
{
	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> layersEdge = TPair<TArray<TArray<FString>>, TArray<GraphEdge>>();
	int32 virtualIndex = 0;
	for (int32 i = 0; i < layersVirtualized.Num() - 1; i++)
	{
		TArray<FString> currentLayer = layersVirtualized[i];
		TArray<FString> nextLayer = layersVirtualized[i + 1];
		for (FString node : layersVirtualized[i])
		{
			TArray<GraphEdge> incomingEdges = TArray<GraphEdge>();
			TArray<GraphEdge> outgoingEdges = TArray<GraphEdge>();
			for (GraphEdge ge : edges)
			{
				if (ge.GetToNode().Equals(node) && FMath::Abs(GetLayerNumber(ge.GetFromNode(), layersVirtualized) - GetLayerNumber(node, layersVirtualized)) > 1)
				{
					incomingEdges.Add(ge);
				}
				if (ge.GetFromNode().Equals(node) && FMath::Abs(GetLayerNumber(ge.GetToNode(), layersVirtualized) - GetLayerNumber(node, layersVirtualized)) > 1)
				{
					outgoingEdges.Add(ge);
				}
			}
			for (GraphEdge ge : outgoingEdges)
			{
				FString virtualNode = "virtual" + FString::FromInt(virtualIndex);
				virtualIndex++;
				layersVirtualized[i + 1].Add(virtualNode);
				TArray<GraphEdge> edgesToRemove = TArray<GraphEdge>();
				for (GraphEdge e : edges)
				{
					if (e.Equals(ge))
					{
						edgesToRemove.Add(e);
					}
				}
				for (GraphEdge e : edgesToRemove)
				{
					edges.Remove(e);
				}
				edges.Add(GraphEdge(ge.GetFromNode(), virtualNode, ge.GetLabel(), ge.IsToInvert()));
				edges.Add(GraphEdge(virtualNode, ge.GetToNode(), ge.GetLabel(), ge.IsToInvert()));
			}
			/*for (GraphEdge ge : outgoingEdges)
			{
				FString virtualNode = "virtual" + virtualIndex++;
				nextLayer.Add(virtualNode);
				edges.Remove(ge);
				edges.Add(GraphEdge(ge.GetFromNode(), virtualNode, ge.GetLabel()));
				edges.Add(GraphEdge(virtualNode, ge.GetToNode(), ge.GetLabel()));
			}*/
		}
	}
	layersEdge.Key = layersVirtualized;
	layersEdge.Value = edges;
	return layersEdge;
}

int32 VertexOrderer::GetLayerNumber(FString node, TArray<TArray<FString>> lay)
{
	int32 layerNumber = 0;
	for (TArray<FString> l : lay)
	{
		if (l.Contains(node))
		{
			return layerNumber;
		}
		layerNumber++;
	}
	return -1;
}

TArray<TArray<FString>> VertexOrderer::MedianHeuristic(TArray<TArray<FString>> layersM, TArray<GraphEdge> edges, int32 i)
{
	if (i % 2 == 0)
	{
		for (int32 r = 1; r < layersM.Num(); r++)
		{
			TMap<FString, int32> median = TMap<FString, int32>();
			for (FString node : layersM[r])
			{
				TArray<int32> P = TArray<int32>();
				for (FString n : layersM[r - 1])
				{
					for (GraphEdge ge : edges)
					{
						if (ge.GetFromNode().Equals(n) && ge.GetToNode().Equals(node))
						{
							P.Add(layersM[r - 1].IndexOfByKey(n));
							break;
						}
					}
				}
				//GLog->Log("|||||||");
				int32 medianValue = MedianValue(P);
				median.Add(node, medianValue);
				//GLog->Log("medianvalue: " + FString::FromInt(medianValue));
			}
			/*GLog->Log("Rank " + FString::FromInt(r) + " BM");
			for (FString fs : layersM[r])
			{
				GLog->Log(fs);
			}*/
			layersM[r] = SortRank(layersM[r], median);
			/*GLog->Log("Rank " + FString::FromInt(r) + " AM");
			for (FString fs : layersM[r])
			{
				GLog->Log(fs);
			}*/
		}
	}
	else
	{
		for (int32 r = layersM.Num() - 2; r > 0; r--)
		{
			TMap<FString, int32> median = TMap<FString, int32>();
			for (FString node : layersM[r])
			{
				TArray<int32> P = TArray<int32>();
				for (FString n : layersM[r + 1])
				{
					for (GraphEdge ge : edges)
					{
						if (ge.GetToNode().Equals(n) && ge.GetFromNode().Equals(node))
						{
							P.Add(layersM[r + 1].IndexOfByKey(n));
							break;
						}
					}
				}
				median.Add(node, MedianValue(P));
			}
			SortRank(layersM[r], median);
		}
	}
	return layersM;
}

TArray<TArray<FString>> VertexOrderer::Transpose(TArray<TArray<FString>> layersT, TArray<GraphEdge> edgesT)
{
	bool improved = true;
	while (improved)
	{
		improved = false;
		for (int r = 0; r < layersT.Num() - 2; r++)
		{
			for (int i = 0; i < layersT[r].Num() - 1; i++)
			{
				FString v = layersT[r][i];
				FString w = layersT[r][i + 1];
				if (Crossing(layersT, edgesT, v, w, r) > Crossing(layersT, edgesT, w, v, r))
				{
					improved = true;
					layersT[r].Swap(i, i + 1);
				}
			}
		}
	}
	return layersT;
}

int32 VertexOrderer::Crossing(TArray<TArray<FString>> layersC, TArray<GraphEdge> edgesC)
{
	int32 numberCrossing = 0;
	for (int r = 0; r < layersC.Num() - 1; r++)
	{
		if (layersC.Num() >= 2)
		{
			for (int i = 0; i < layersC[r].Num() - 1; i++)
			{
				for (int j = i + 1; j < layersC[r].Num(); j++)
				{
					if (i != j)
					{
						FString v = layersC[r][i];
						FString w = layersC[r][j];
						numberCrossing += Crossing(layersC, edgesC, v, w, r);
					}
				}
			}
		}
	}
	return numberCrossing;
}

int32 VertexOrderer::Crossing(TArray<TArray<FString>> layersC, TArray<GraphEdge> edgesC, FString n1, FString n2, int32 layerIndex)
{
	int32 numberCrossing = 0;
	TArray<FString> n1AdjacentNodes = TArray<FString>();
	TArray<FString> n2AdjacentNodes = TArray<FString>();
	for (GraphEdge ge : edgesC)
	{
		if (ge.GetFromNode().Equals(n1))
		{
			n1AdjacentNodes.Add(ge.GetToNode());
		}
		if (ge.GetFromNode().Equals(n2))
		{
			n2AdjacentNodes.Add(ge.GetToNode());
		}
	}
	TArray<int32> n1AdjacentNodesIndex = TArray<int32>();
	TArray<int32> n2AdjacentNodesIndex = TArray<int32>();
	for (FString fs : n1AdjacentNodes)
	{
		int32 index = 0;
		layersC[layerIndex + 1].Find(fs, index);
		n1AdjacentNodesIndex.Add(index);
	}
	for (FString fs : n2AdjacentNodes)
	{
		int32 index = 0;
		layersC[layerIndex + 1].Find(fs, index);
		n2AdjacentNodesIndex.Add(index);
	}

	for (int32 i = 0; i < n1AdjacentNodesIndex.Num(); i++)
	{
		for (int32 j = 0; j < n2AdjacentNodesIndex.Num(); j++)
		{
			if (n2AdjacentNodesIndex[j] < n1AdjacentNodesIndex[i])
			{
				numberCrossing++;
			}
		}
	}

	return numberCrossing;
}

int32 VertexOrderer::MedianValue(TArray<int32> P)
{
	int32 m = P.Num() / 2;
	if (P.Num() == 0)
	{
		return -1;
	}
	else if (P.Num() % 2 == 1)
	{
		return P[m];
	}
	else if (P.Num() == 2)
	{
		return (P[0] + P[1]) / 2;
	}
	else
	{
		int32 left = P[m - 1] - P[0];
		int32 right = P[P.Num() - 1] - P[m];
		return (P[m - 1] * right + P[m] * left) / (left + right);
	}
}

TArray<FString> VertexOrderer::SortRank(TArray<FString> layer, TMap<FString, int32> median)
{
	/*for (FString fs : layer)
	{
		GLog->Log("BS" + fs);
	}*/
	TMap<FString, int32> positionToKeep = TMap<FString, int32>();
	for (FString fs : layer)
	{
		if (median[fs] == -1)
		{
			positionToKeep.Add(fs, layer.IndexOfByKey(fs));
			median.Remove(fs);
		}
	}
	/*GLog->Log("BS");
	for (TPair<FString, int32> p : median)
	{
		GLog->Log(p.Key + " " + FString::FromInt(p.Value));
	}*/
	median.ValueSort([](const int32 A, const int32 B) {
		return A < B;
	});
	/*GLog->Log("AS");
	for (TPair<FString, int32> p : median)
	{
		GLog->Log(p.Key + " " + FString::FromInt(p.Value));
	}*/
	layer.Empty();
	for (TPair<FString, int32> p : median)
	{
		layer.Add(p.Key);
	}
	for (TPair<FString, int32> p : positionToKeep)
	{
		layer.Insert(p.Key, p.Value);
	}
	/*for (FString fs : layer)
	{
		GLog->Log("AS" + fs);
	}*/
	return layer;
}
