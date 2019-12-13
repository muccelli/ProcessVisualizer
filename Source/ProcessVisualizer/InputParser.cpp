// Fill out your copyright notice in the Description page of Project Settings.


#include "InputParser.h"
#include "NodeActor.h"
#include "EdgeActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"

// Sets default values
AInputParser::AInputParser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInputParser::BeginPlay()
{
	Super::BeginPlay();
	
	const FString JsonFilePath = FPaths::ProjectContentDir() + "/Input/" + InputJSON + ".json";
	FString JsonString; //Json converted to FString

	FFileHelper::LoadFileToString(JsonString, *JsonFilePath);

	//Create a json object to store the information from the json string
	//The json reader is used to deserialize the json object later on
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		switch (VisualizationType)
		{
		case EVisualizationType::VE_Horizontal:
		{
			bool retflag;
			CreateHorizontalGraph(JsonObject, retflag);
			if (retflag) return;
		}
			break;
		case EVisualizationType::VE_Vertical:
		{
			bool retflag;
			CreateVerticalGraph(JsonObject, retflag);
			if (retflag) return;
		}
			break;
		case EVisualizationType::VE_HorizontalImproved:
		{
			bool retflag;
			CreateHorizontalImprovedGraph(JsonObject, retflag);
			if (retflag) return;
		}
		break;
		default:
			break;
		}
	}
	else
	{
		GLog->Log("couldn't deserialize");
	}
}

// Called every frame
void AInputParser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<TArray<FString>> AInputParser::GetNodesPositioning(TArray<TSharedPtr<FJsonValue>> nodesArray, TArray<TSharedPtr<FJsonValue>> edgesArray)
{
	TArray<TArray<FString>> nodesPositioning;

	// add the first layer
	TArray<FString> indexVector;
	indexVector.Emplace(nodesArray[0]->AsObject()->GetStringField("label"));

	nodesPositioning.Emplace(indexVector);

	// loop for at max each node
	for (int32 i = 0; i < nodesArray.Num(); i++)
	{
		// if the i-th layer contains at least one node
		if (nodesPositioning[i].Num() > 0)
		{
			TArray<FString> layerVector = nodesPositioning[i];
			TArray<FString> childrenLayerVector;
			// for each node in that layer
			for (FString fs : layerVector)
			{
				// look for edges coming from that node and add its "children" to the next layer
				for (int32 j = 0; j < edgesArray.Num(); j++)
				{
					if (edgesArray[j]->AsObject()->GetStringField("fromNode").Equals(fs))
					{
						// check if the possible child is already present in the layered graph
						bool alreadyPresentFlag = false;
						for (int32 x = 0; x < nodesPositioning.Num(); x++)
						{
							if (nodesPositioning[x].Contains(edgesArray[j]->AsObject()->GetStringField("toNode")))
							{
								alreadyPresentFlag = true;
								break;
							}
							if (alreadyPresentFlag)
							{
								break;
							}
						}
						if (!alreadyPresentFlag && !childrenLayerVector.Contains(edgesArray[j]->AsObject()->GetStringField("toNode")))
						{
							childrenLayerVector.Emplace(edgesArray[j]->AsObject()->GetStringField("toNode"));
						}
					}
				}
			}
			nodesPositioning.Emplace(childrenLayerVector);
		}
		else
		{
			break;
		}
	}









	//GLog->Log(FString::FromInt(nodesPositioning[nodesPositioning.Num()-1].Num()));
	nodesPositioning.RemoveAt(nodesPositioning.Num() - 1);
	//GLog->Log(FString::FromInt(nodesPositioning[nodesPositioning.Num() - 1].Num()));

	for (int32 i = 0; i < nodesPositioning.Num(); i++)
	{
		for (int32 j = 0; j < nodesPositioning[i].Num(); j++)
		{
			GLog->Log("Layer " + FString::FromInt(i) + ": " + nodesPositioning[i][j]);
		}
	}

	return nodesPositioning;
}

TPair<TArray<TArray<FString>>, TArray<GraphEdge>> AInputParser::GetNodesPositioningImproved(TArray<TSharedPtr<FJsonValue>> nodesArray, TArray<TSharedPtr<FJsonValue>> edgesArray)
{
	GLog->Log("Getting nodes positions...");

	TArray<TArray<FString>> nodesPositioning;
	TArray<GraphEdge> edges = TArray<GraphEdge>();
	for (int32 i = 0; i < edgesArray.Num(); i++)
	{
		edges.Add(GraphEdge(edgesArray[i]->AsObject()->GetStringField("fromNode"), edgesArray[i]->AsObject()->GetStringField("toNode"), edgesArray[i]->AsObject()->GetStringField("label"), false));
	}

	TSet<FString> nodes = TSet<FString>();
	for (int32 i = 0; i < nodesArray.Num(); i++)
	{
		nodes.Add(nodesArray[i]->AsObject()->GetStringField("label"));
	}
	Graph graph = Graph(edges, nodes);

	GLog->Log("Number of nodes = " + FString::FromInt(nodes.Num()));
	GLog->Log("Number of edges = " + FString::FromInt(edges.Num()));

	CycleRemover cr = CycleRemover(graph);
	graph = cr.RemoveCycles();

	GLog->Log("Number of nodes after cycle removing = " + FString::FromInt(graph.GetNodes().Num()));
	GLog->Log("Number of edges after cycle removing = " + FString::FromInt(graph.GetEdges().Num()));

	LayerAssigner la = LayerAssigner(graph);
	nodesPositioning = la.assignLayers();

	for (int32 i = 0; i < nodesPositioning.Num(); i++)
	{
		GLog->Log("Layer " + FString::FromInt(i));
		for (int32 j = 0; j < nodesPositioning[i].Num(); j++)
		{
			GLog->Log("Layer " + FString::FromInt(i) + ": " + nodesPositioning[i][j]);
		}
	}

	GLog->Log("Number of layers after layers assigning = " + FString::FromInt(nodesPositioning.Num()));

	VertexOrderer vo = VertexOrderer(graph, nodesPositioning);
	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> positioningEdgesPair = vo.OrderVertices();

	GLog->Log("Number of layers after vertex ordering = " + FString::FromInt(positioningEdgesPair.Key.Num()));

	for (int32 i = 0; i < positioningEdgesPair.Key.Num(); i++)
	{
		GLog->Log("Layer " + FString::FromInt(i));
		for (int32 j = 0; j < positioningEdgesPair.Key[i].Num(); j++)
		{
			GLog->Log("Layer " + FString::FromInt(i) + ": " + positioningEdgesPair.Key[i][j]);
		}
	}

	return positioningEdgesPair;
}

FVector AInputParser::ComputeMiddleSplinePointLocation(FVector startSplinePointLocation, FVector endSplinePointLocation)
{

	FVector direction = (startSplinePointLocation - endSplinePointLocation);
	float distance = direction.Size();

	direction.Normalize();

	float halfDistance = distance * 0.5;

	FVector middleSplinePointLocation = startSplinePointLocation - (direction*halfDistance);

	middleSplinePointLocation.X += FMath::RandRange(-MiddleSplinePointDeviation, MiddleSplinePointDeviation);
	middleSplinePointLocation.Y += FMath::RandRange(-MiddleSplinePointDeviation, MiddleSplinePointDeviation);
	middleSplinePointLocation.Z += FMath::RandRange(0, 1);

	return middleSplinePointLocation;
}

void AInputParser::CreateVerticalGraph(TSharedPtr<FJsonObject> &JsonObject, bool &retflag)
{
	retflag = true;
	//Check if you can spawn objects
	if (!NodeActorBP) return;

	TSharedPtr<FJsonObject> FuzzyModelObject = JsonObject->GetObjectField("FuzzyModel");

	TArray<TSharedPtr<FJsonValue>> nodesArray = FuzzyModelObject->GetArrayField("nodes");
	TArray<TSharedPtr<FJsonValue>> edgesArray = FuzzyModelObject->GetArrayField("edges");

	// Define positioning of the nodes
	TArray<TArray<FString>> NodesPositioning = GetNodesPositioning(nodesArray, edgesArray);

	// Find min and max significance in the nodes array
	int32 Max = 0;
	int32 Min = 99999;
	for (int32 index = 0; index < nodesArray.Num(); index++)
	{
		if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") > Max)
		{
			Max = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}
		if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") < Min)
		{
			Min = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}
	}

	FVector* location = new FVector(1000, 0, 1000);

	for (int32 i = 0; i < NodesPositioning.Num(); i++)
	{
		location->X = FMath::RandRange(-XNodeDeviation, XNodeDeviation);
		location->Y = (NodesPositioning[i].Num() - 1)*(-NodesYDistance / 2);
		for (int32 j = 0; j < NodesPositioning[i].Num(); j++)
		{
			FString label = NodesPositioning[i][j];
			int32 significance = 0;
			for (int32 index = 0; index < nodesArray.Num(); index++)
			{
				if (nodesArray[index]->AsObject()->GetStringField("label").Equals(label))
				{
					significance = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
				}
			}

			UWorld* const World = GetWorld();
			if (World)
			{
				//ANodeActor* Node = (ANodeActor*) World->SpawnActor(ANodeActor::StaticClass(), location);
				ANodeActor* Node = (ANodeActor*)World->SpawnActor(NodeActorBP, location);
#if WITH_EDITOR
				Node->SetFolderPath("Nodes");
#endif
				Node->Label = label;

				// set scale
				float scale;

				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (significance < Min + (Max - Min) / 5)
					{
						scale = 0.2f;
					}
					else if (significance < Min + (Max - Min) * 2 / 5)
					{
						scale = 0.4f;
					}
					else if (significance < Min + (Max - Min) * 3 / 5)
					{
						scale = 0.6f;
					}
					else if (significance < Min + (Max - Min) * 4 / 5)
					{
						scale = 0.8f;
					}
					else if (significance <= Min + (Max - Min))
					{
						scale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					scale = ((float(significance) - float(Min)) / float(Max)) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				case EScalingMethod::VE_MinMax:

					scale = ((float(significance) - float(Min)) / (float(Max) - float(Min))) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				default:
					break;
				}

				Node->SignificanceScale = scale;

				Node->LabelText = FText::AsCultureInvariant(label);

				Node->SetNodeProperties();

			}
			location->X += FMath::RandRange(-XNodeDeviation, XNodeDeviation);
			location->Y += NodesYDistance;
		}
		location->Y = 0;
		location->Z -= NodesZDistance;
	}

	FVector* graphLocation = new FVector(0, 0, ((location->Z + NodesZDistance) + 1000) / 2);
	SetActorLocation(*graphLocation);



	// Find min and max significance in the edges array
	Max = 0;
	Min = 99999;
	for (int32 index = 0; index < edgesArray.Num(); index++)
	{
		if (edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance") > Max)
		{
			Max = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}
		if (edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance") < Min)
		{
			Min = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}
	}

	location = new FVector(0, 0, 0);
	for (int32 index = 0; index < edgesArray.Num(); index++)
	{

		FString label = edgesArray[index]->AsObject()->GetStringField("label");
		int32 significance = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		FString fromNode = edgesArray[index]->AsObject()->GetStringField("fromNode");
		FString toNode = edgesArray[index]->AsObject()->GetStringField("toNode");

		UWorld* const World = GetWorld();
		if (World)
		{
			ANodeActor* FromNode = nullptr;
			ANodeActor* ToNode = nullptr;

			FVector startingLocation;
			FVector endingLocation;

			for (TActorIterator<ANodeActor> ActorItr(World); ActorItr; ++ActorItr)
			{
				if (ActorItr->Label == fromNode)
				{
					FromNode = *ActorItr;
					startingLocation = ActorItr->GetActorLocation();
				}

				if (ActorItr->Label == toNode)
				{
					ToNode = *ActorItr;
					endingLocation = ActorItr->GetActorLocation();
				}
			}

			if (FromNode && ToNode)
			{
				AEdgeActor* Edge = (AEdgeActor*)World->SpawnActor(EdgeActorBP, location);
#if WITH_EDITOR
				Edge->SetFolderPath("Edges");
#endif
				Edge->Label = label;
				Edge->FromNode = FromNode;
				Edge->ToNode = ToNode;
				Edge->Spline->SetLocationAtSplinePoint(0, startingLocation, ESplineCoordinateSpace::Local);
				Edge->Spline->SetLocationAtSplinePoint(Edge->Spline->GetNumberOfSplinePoints() - 1, endingLocation, ESplineCoordinateSpace::Local);

				Edge->Spline->AddSplinePointAtIndex(ComputeMiddleSplinePointLocation(Edge->Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local), Edge->Spline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local)), 1, ESplineCoordinateSpace::Local);

				float scale;
				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (significance < Min + (Max - Min) / 5)
					{
						scale = 0.2f;
					}
					else if (significance < Min + (Max - Min) * 2 / 5)
					{
						scale = 0.4f;
					}
					else if (significance < Min + (Max - Min) * 3 / 5)
					{
						scale = 0.6f;
					}
					else if (significance < Min + (Max - Min) * 4 / 5)
					{
						scale = 0.8f;
					}
					else if (significance <= Min + (Max - Min))
					{
						scale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					scale = ((float(significance) - float(Min)) / float(Max)) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				case EScalingMethod::VE_MinMax:

					scale = ((float(significance) - float(Min)) / (float(Max) - float(Min))) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				default:
					break;
				}

				Edge->Significance = scale / 2;

				Edge->SetEdgeProperties();
			}
		}
	}

	retflag = false;
}

void AInputParser::CreateHorizontalGraph(TSharedPtr<FJsonObject> &JsonObject, bool &retflag)
{
	retflag = true;
	//Check if you can spawn objects
	if (!NodeActorBP) return;

	TSharedPtr<FJsonObject> FuzzyModelObject = JsonObject->GetObjectField("FuzzyModel");

	TArray<TSharedPtr<FJsonValue>> nodesArray = FuzzyModelObject->GetArrayField("nodes");
	TArray<TSharedPtr<FJsonValue>> edgesArray = FuzzyModelObject->GetArrayField("edges");

	// Define positioning of the nodes
	TArray<TArray<FString>> NodesPositioning = GetNodesPositioning(nodesArray, edgesArray);

	// Find min and max significance and duration in the nodes array
	int32 MaxSign = 0;
	int32 MinSign = 99999;
	double MaxTime = 0;
	double MinTime = nodesArray[1]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetIntegerField("MeanDuration");
	for (int32 index = 0; index < nodesArray.Num(); index++)
	{
		if (!nodesArray[index]->AsObject()->GetStringField("label").Equals("start_node") && !nodesArray[index]->AsObject()->GetStringField("label").Equals("end_node"))
		{
			if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") > MaxSign)
			{
				MaxSign = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
			}
			if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") < MinSign)
			{
				MinSign = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
			}
		}

		if (!nodesArray[index]->AsObject()->GetStringField("label").Equals("start"))
		{
			if (nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration") > MaxTime)
			{
				MaxTime = nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration");
			}
			if (nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration") < MinTime)
			{
				MinTime = nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration");
			}
		}
	}

	FVector* location = new FVector((NodesPositioning.Num() / 2) * NodesZDistance, 0, 0);

	for (int32 i = 0; i < NodesPositioning.Num(); i++)
	{
		location->Y = (NodesPositioning[i].Num() - 1)*(-NodesYDistance / 2);
		for (int32 j = 0; j < NodesPositioning[i].Num(); j++)
		{
			FString label = NodesPositioning[i][j];
			int32 significance = 0;
			double duration = 0;
			TMap<FString, FValuesToFrequencyMap> attributes = TMap<FString, FValuesToFrequencyMap>();
			for (int32 index = 0; index < nodesArray.Num(); index++)
			{
				if (nodesArray[index]->AsObject()->GetStringField("label").Equals(label))
				{
					significance = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");

					if (!nodesArray[index]->AsObject()->GetStringField("label").Equals("start"))
					{
						duration = nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration");
						GLog->Log("duration " + label + ": " + FString::SanitizeFloat(duration));
					}
					
					for (auto itr = nodesArray[index]->AsObject()->GetArrayField("attributes")[0]->AsObject()->Values.CreateConstIterator(); itr; ++itr)
					{
						FValuesToFrequencyMap currAttr = FValuesToFrequencyMap();
						for (auto attrItr = nodesArray[index]->AsObject()->GetArrayField("attributes")[0]->AsObject()->GetArrayField((*itr).Key)[0]->AsObject()->Values.CreateConstIterator(); attrItr; ++attrItr)
						{
							currAttr.AddToMap((*attrItr).Key, (*attrItr).Value->AsString());
						}
						currAttr.SortByValue();
						attributes.Add((*itr).Key, currAttr);
					}
				}
			}

			/*for (auto& fs : attributes)
			{
				GLog->Log(fs.Key + ": ");
				for (auto& ff : fs.Value.GetValuesToPercentage())
				{
					GLog->Log("    " + ff.Key + ": " + ff.Value);
				}
			}*/

			UWorld* const World = GetWorld();
			if (World)
			{
				//ANodeActor* Node = (ANodeActor*) World->SpawnActor(ANodeActor::StaticClass(), location);
				ANodeActor* Node = (ANodeActor*)World->SpawnActor(NodeActorBP, location);
#if WITH_EDITOR
				Node->SetFolderPath("Nodes");
#endif
				Node->Label = label;

				// set scale
				float scale;
				double timeScale = 0;

				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (significance < MinSign + (MaxSign - MinSign) / 5)
					{
						scale = 0.2f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 2 / 5)
					{
						scale = 0.4f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 3 / 5)
					{
						scale = 0.6f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 4 / 5)
					{
						scale = 0.8f;
					}
					else if (significance <= MinSign + (MaxSign - MinSign))
					{
						scale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					scale = ((float(significance) - float(MinSign)) / float(MaxSign)) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				case EScalingMethod::VE_MinMax:

					scale = ((float(significance) - float(MinSign)) / (float(MaxSign) - float(MinSign))) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				default:
					break;
				}

				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (duration < MinTime + (MaxTime - MinTime) / 5)
					{
						timeScale = 0.2f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 2 / 5)
					{
						timeScale = 0.4f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 3 / 5)
					{
						timeScale = 0.6f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 4 / 5)
					{
						timeScale = 0.8f;
					}
					else if (duration <= MinTime + (MaxTime - MinTime))
					{
						timeScale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					timeScale = ((float(duration) - float(MinTime)) / float(MaxTime));

					break;
				case EScalingMethod::VE_MinMax:

					timeScale = ((duration - MinTime) / (MaxTime - MinTime));

					break;
				default:
					break;
				}

				Node->SignificanceScale = scale;

				if (MinTime == 0 && MaxTime == 0)
				{
					Node->TimeScale = 0;
				}
				else
				{
					Node->TimeScale = timeScale;
				}

				GLog->Log(label + ": " + FString::SanitizeFloat(timeScale));

				Node->LabelText = FText::AsCultureInvariant(label);

				Node->Attributes = attributes;

				Node->SetNodeProperties();

			}
			location->Y += NodesYDistance;
		}
		location->Y = 0;
		location->X -= NodesZDistance;
	}

	FVector* graphLocation = new FVector(0, 0, 0);
	SetActorLocation(*graphLocation);



	// Find min and max significance and duration in the edges array
	MaxSign = 0;
	MinSign = 99999;
	MaxTime = 0;
	MinTime = edgesArray[1]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetIntegerField("MeanDuration");
	for (int32 index = 0; index < edgesArray.Num(); index++)
	{
		if (edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance") > MaxSign)
		{
			MaxSign = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}
		if (edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance") < MinSign)
		{
			MinSign = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}
		if (!edgesArray[index]->AsObject()->GetStringField("label").Contains("start ->"))
		{
			if (edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration") > MaxTime)
			{
				MaxTime = edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration");
			}
			if (edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration") < MinTime)
			{
				MinTime = edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration");
			}
		}
	}

	location = new FVector(0, 0, 0);
	for (int32 index = 0; index < edgesArray.Num(); index++)
	{

		FString label = edgesArray[index]->AsObject()->GetStringField("label");
		int32 significance = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		float duration = 0;
		if (!edgesArray[index]->AsObject()->GetStringField("label").Contains("start ->"))
		{
			duration = edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration");
		}
		FString fromNode = edgesArray[index]->AsObject()->GetStringField("fromNode");
		FString toNode = edgesArray[index]->AsObject()->GetStringField("toNode");

		UWorld* const World = GetWorld();
		if (World)
		{
			ANodeActor* FromNode = nullptr;
			ANodeActor* ToNode = nullptr;

			FVector startingLocation;
			FVector endingLocation;

			GLog->Log(label);
			for (TActorIterator<ANodeActor> ActorItr(World); ActorItr; ++ActorItr)
			{
				if (ActorItr->Label.Equals(fromNode.TrimEnd()))
				{
					FromNode = *ActorItr;
					startingLocation.X = ActorItr->GetActorLocation().X;
					startingLocation.Y = ActorItr->GetActorLocation().Y;
					startingLocation.Z = 0;
				}

				if (ActorItr->Label.Equals(toNode.TrimEnd()))
				{
					ToNode = *ActorItr;
					endingLocation.X = ActorItr->GetActorLocation().X;
					endingLocation.Y = ActorItr->GetActorLocation().Y;
					endingLocation.Z = 0;
				}
			}

			if (FromNode && ToNode)
			{
				AEdgeActor* Edge = (AEdgeActor*)World->SpawnActor(EdgeActorBP, location);
#if WITH_EDITOR
				Edge->SetFolderPath("Edges");
#endif
				Edge->Label = label;
				Edge->FromNode = FromNode;
				Edge->ToNode = ToNode;
				Edge->Spline->SetLocationAtSplinePoint(0, startingLocation, ESplineCoordinateSpace::Local);
				Edge->Spline->SetLocationAtSplinePoint(Edge->Spline->GetNumberOfSplinePoints() - 1, endingLocation, ESplineCoordinateSpace::Local);

				Edge->Spline->AddSplinePointAtIndex(ComputeMiddleSplinePointLocation(Edge->Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local), Edge->Spline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local)), 1, ESplineCoordinateSpace::Local);

				float signScale;
				double timeScale;
				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (significance < MinSign + (MaxSign - MinSign) / 5)
					{
						signScale = 0.2f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 2 / 5)
					{
						signScale = 0.4f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 3 / 5)
					{
						signScale = 0.6f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 4 / 5)
					{
						signScale = 0.8f;
					}
					else if (significance <= MinSign + (MaxSign - MinSign))
					{
						signScale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					signScale = ((float(significance) - float(MinSign)) / float(MaxSign)) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				case EScalingMethod::VE_MinMax:

					signScale = ((float(significance) - float(MinSign)) / (float(MaxSign) - float(MinSign))) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				default:
					break;
				}

				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (duration < MinTime + (MaxTime - MinTime) / 5)
					{
						timeScale = 0.2f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 2 / 5)
					{
						timeScale = 0.4f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 3 / 5)
					{
						timeScale = 0.6f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 4 / 5)
					{
						timeScale = 0.8f;
					}
					else if (duration <= MinTime + (MaxTime - MinTime))
					{
						timeScale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					timeScale = ((float(duration) - float(MinTime)) / float(MaxTime));

					break;
				case EScalingMethod::VE_MinMax:

					timeScale = ((float(duration) - float(MinTime)) / (float(MaxTime) - float(MinTime)));

					break;
				default:
					break;
				}

				Edge->Significance = signScale / 2;

				Edge->TimeScale = timeScale;

				Edge->SetEdgeProperties();
			}
		}
	}

	retflag = false;
}

void AInputParser::CreateHorizontalImprovedGraph(TSharedPtr<FJsonObject>& JsonObject, bool & retflag)
{
	retflag = true;
	//Check if you can spawn objects
	if (!NodeActorBP) return;

	GLog->Log("HORIZONTAL WITH IMPROVED NODES POSITIONING\n\n");

	TSharedPtr<FJsonObject> FuzzyModelObject = JsonObject->GetObjectField("FuzzyModel");

	TArray<TSharedPtr<FJsonValue>> nodesArray = FuzzyModelObject->GetArrayField("nodes");
	TArray<TSharedPtr<FJsonValue>> edgesArray = FuzzyModelObject->GetArrayField("edges");

	FString heigthVar = "";
	switch (HeightVariable)
	{
	case EHeightVariable::VE_TotalDuration:
		heigthVar = "TotalDuration";
		break;
	case EHeightVariable::VE_MeanDuration:
		heigthVar = "MeanDuration";
		break;
	case EHeightVariable::VE_MedianDuration:
		heigthVar = "MedianDuration";
		break;
	case EHeightVariable::VE_MinDuration:
		heigthVar = "MinDuration";
		break;
	case EHeightVariable::VE_MaxDuration:
		heigthVar = "MaxDuration";
		break;
	default:
		heigthVar = "MeanDuration";
		break;
	}

	// Define positioning of the nodes
	TPair<TArray<TArray<FString>>, TArray<GraphEdge>> NodesPositioning = GetNodesPositioningImproved(nodesArray, edgesArray);
	TArray<TArray<FString>> nodesLayers = NodesPositioning.Key;
	TArray<GraphEdge> newEdges = NodesPositioning.Value;

	// Find min and max significance and duration in the nodes array
	int32 MaxSign = 0;
	int32 MinSign = 99999;
	double MaxTime = 0;
	double MinTime = nodesArray[1]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetIntegerField(heigthVar);
	
	int32 totalNumberofCases = 0;
	int32 totalNumberOfEvent = 0;

	if (InputJSON.Equals("BPIChallenge2017"))
	{
		NodesZDistance = 1000;
	}

	for (int32 index = 0; index < nodesArray.Num(); index++)
	{
		if (!nodesArray[index]->AsObject()->GetStringField("label").Equals("start_node") && !nodesArray[index]->AsObject()->GetStringField("label").Equals("end_node"))
		{
			totalNumberOfEvent += nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
			if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") > MaxSign)
			{
				MaxSign = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
			}
			if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") < MinSign)
			{
				MinSign = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
			}
		}
		else if (nodesArray[index]->AsObject()->GetStringField("label").Equals("start_node"))
		{
			totalNumberofCases = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}

		if (!nodesArray[index]->AsObject()->GetStringField("label").Equals("start_node") && !nodesArray[index]->AsObject()->GetStringField("label").Equals("end_node"))
		{
			if (nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar) > MaxTime)
			{
				MaxTime = nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar);
			}
			if (nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar) < MinTime)
			{
				MinTime = nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar);
			}
		}
	}

	FVector* location = new FVector((nodesLayers.Num() / 2) * NodesZDistance, 0, 0);

	for (int32 i = 0; i < nodesLayers.Num(); i++)
	{
		location->Y = (nodesLayers[i].Num() - 1)*(-NodesYDistance / 2);
		for (int32 j = 0; j < nodesLayers[i].Num(); j++)
		{
			FString label = nodesLayers[i][j];
			int32 significance = 0;
			int32 caseFrequency = 0;
			double duration = 0;
			TMap<FString, FValuesToFrequencyMap> attributes = TMap<FString, FValuesToFrequencyMap>();
			TMap<FString, float> durations = TMap<FString, float>();
			for (int32 index = 0; index < nodesArray.Num(); index++)
			{
				if (nodesArray[index]->AsObject()->GetStringField("label").Equals(label))
				{
					significance = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
					caseFrequency = nodesArray[index]->AsObject()->GetIntegerField("caseFrequencySignificance");

					if (!nodesArray[index]->AsObject()->GetStringField("label").Equals("end_node"))
					{
						duration = nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar);
						durations.Add("TotalDuration", nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("TotalDuration"));
						durations.Add("MeanDuration", nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration"));
						durations.Add("MedianDuration", nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MedianDuration"));
						durations.Add("MinDuration", nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MinDuration"));
						durations.Add("MaxDuration", nodesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MaxDuration"));
					}

					for (auto itr = nodesArray[index]->AsObject()->GetArrayField("attributes")[0]->AsObject()->Values.CreateConstIterator(); itr; ++itr)
					{
						FValuesToFrequencyMap currAttr = FValuesToFrequencyMap();
						for (auto attrItr = nodesArray[index]->AsObject()->GetArrayField("attributes")[0]->AsObject()->GetArrayField((*itr).Key)[0]->AsObject()->Values.CreateConstIterator(); attrItr; ++attrItr)
						{
							currAttr.AddToMap((*attrItr).Key, (*attrItr).Value->AsString());
						}
						currAttr.SortByValue();
						attributes.Add((*itr).Key, currAttr);
					}
				}
			}

			UWorld* const World = GetWorld();
			if (World)
			{
				//ANodeActor* Node = (ANodeActor*) World->SpawnActor(ANodeActor::StaticClass(), location);
				ANodeActor* Node = (ANodeActor*)World->SpawnActor(NodeActorBP, location);
#if WITH_EDITOR
				Node->SetFolderPath("Nodes");
#endif
				Node->Label = label;

				// set scale
				float scale;
				double timeScale = 0;

				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (significance < MinSign + (MaxSign - MinSign) / 5)
					{
						scale = 0.2f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 2 / 5)
					{
						scale = 0.4f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 3 / 5)
					{
						scale = 0.6f;
					}
					else if (significance < MinSign + (MaxSign - MinSign) * 4 / 5)
					{
						scale = 0.8f;
					}
					else if (significance <= MinSign + (MaxSign - MinSign))
					{
						scale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					scale = ((float(significance) - float(MinSign)) / float(MaxSign)) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				case EScalingMethod::VE_MinMax:

					scale = ((float(significance) - float(MinSign)) / (float(MaxSign) - float(MinSign))) + 0.2f;
					//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

					break;
				default:
					break;
				}

				switch (ScalingMethod)
				{
				case EScalingMethod::VE_Discrete:

					if (duration < MinTime + (MaxTime - MinTime) / 5)
					{
						timeScale = 0.2f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 2 / 5)
					{
						timeScale = 0.4f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 3 / 5)
					{
						timeScale = 0.6f;
					}
					else if (duration < MinTime + (MaxTime - MinTime) * 4 / 5)
					{
						timeScale = 0.8f;
					}
					else if (duration <= MinTime + (MaxTime - MinTime))
					{
						timeScale = 1;
					}

					break;
				case EScalingMethod::VE_Continuous:

					timeScale = ((float(duration) - float(MinTime)) / float(MaxTime));

					break;
				case EScalingMethod::VE_MinMax:

					timeScale = ((duration - MinTime) / (MaxTime - MinTime));

					break;
				default:
					break;
				}

				Node->Significance = significance;

				Node->CaseFrequency = float(caseFrequency) * 100 / totalNumberofCases;

				Node->SignificanceScale = scale;

				Node->Duration = duration;

				Node->Durations = durations;

				if (MinTime == 0 && MaxTime == 0)
				{
					Node->TimeScale = 0;
				}
				else
				{
					Node->TimeScale = timeScale;
				}

				Node->LabelText = FText::AsCultureInvariant(label);

				Node->Attributes = attributes;

				Node->SetNodeProperties();

			}
			location->Y += NodesYDistance;
		}
		location->Y = 0;
		location->X -= NodesZDistance;
	}

	FVector* graphLocation = new FVector(0, 0, 0);
	SetActorLocation(*graphLocation);



	// Find min and max significance and duration in the edges array
	MaxSign = 0;
	MinSign = 99999;
	MaxTime = 0;
	MinTime = edgesArray[1]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetIntegerField(heigthVar);
	for (int32 index = 0; index < edgesArray.Num(); index++)
	{
		if (!edgesArray[index]->AsObject()->GetStringField("label").Contains("start_node ->") && !edgesArray[index]->AsObject()->GetStringField("label").Contains(" -> end_node"))
		{
			if (edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance") > MaxSign)
			{
				MaxSign = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
			}
			if (edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance") < MinSign)
			{
				MinSign = edgesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
			}
		}

		if (!edgesArray[index]->AsObject()->GetStringField("label").Contains("start_node ->") && !edgesArray[index]->AsObject()->GetStringField("label").Contains(" -> end_node"))
		{
			if (edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar) > MaxTime)
			{
				MaxTime = edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar);
			}
			if (edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar) < MinTime)
			{
				MinTime = edgesArray[index]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar);
			}
		}
	}

	location = new FVector(0, 0, 0);

	for (int32 index = 0; index < newEdges.Num(); index++)
	{
		if (!newEdges[index].GetFromNode().Contains("virtual"))
		{
			FString label = "";
			FString fromNode = "";
			FString toNode = "";
			if (!newEdges[index].IsToInvert())
			{
				label = newEdges[index].GetLabel();
				label.Split(" -> ", &fromNode, &toNode);
			}
			else
			{
				newEdges[index].GetLabel().Split(" -> ", &toNode, &fromNode);
				label = fromNode + " -> " + toNode;
			}
			int32 significance = 0;
			int32 caseFrequency = 0;
			float duration = 0;
			TMap<FString, float> durations = TMap<FString, float>();
			for (int32 j = 0; j < edgesArray.Num(); j++)
			{
				if (edgesArray[j]->AsObject()->GetStringField("label").Equals(label))
				{
					significance = edgesArray[j]->AsObject()->GetIntegerField("frequencySignificance");
					caseFrequency = edgesArray[j]->AsObject()->GetIntegerField("caseFrequencySignificance");
					duration = 0;
					if (!edgesArray[j]->AsObject()->GetStringField("label").Contains("start_node ->") && !edgesArray[j]->AsObject()->GetStringField("label").Contains(" -> end_node"))
					{
						duration = edgesArray[j]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField(heigthVar); 
						durations.Add("TotalDuration", edgesArray[j]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("TotalDuration"));
						durations.Add("MeanDuration", edgesArray[j]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MeanDuration"));
						durations.Add("MedianDuration", edgesArray[j]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MedianDuration"));
						durations.Add("MinDuration", edgesArray[j]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MinDuration"));
						durations.Add("MaxDuration", edgesArray[j]->AsObject()->GetArrayField("durations")[0]->AsObject()->GetNumberField("MaxDuration"));
					}
				}
			}

			UWorld* const World = GetWorld();
			if (World)
			{
				ANodeActor* FromNode = nullptr;
				ANodeActor* ToNode = nullptr;

				FVector startingLocation;
				FVector endingLocation;

				for (TActorIterator<ANodeActor> ActorItr(World); ActorItr; ++ActorItr)
				{
					if (ActorItr->Label.TrimEnd().Equals(fromNode.TrimEnd()))
					{
						FromNode = *ActorItr;
						startingLocation.X = ActorItr->GetActorLocation().X;
						startingLocation.Y = ActorItr->GetActorLocation().Y;
						startingLocation.Z = 0;
					}

					if (ActorItr->Label.TrimEnd().Equals(toNode.TrimEnd()))
					{
						ToNode = *ActorItr;
						endingLocation.X = ActorItr->GetActorLocation().X;
						endingLocation.Y = ActorItr->GetActorLocation().Y;
						endingLocation.Z = 0;
					}
				}

				if (FromNode && ToNode)
				{
					AEdgeActor* Edge = (AEdgeActor*)World->SpawnActor(EdgeActorBP, location);
#if WITH_EDITOR
					Edge->SetFolderPath("Edges");
#endif
					Edge->Label = label;
					Edge->FromNode = FromNode;
					Edge->ToNode = ToNode;
					Edge->Spline->SetLocationAtSplinePoint(0, startingLocation, ESplineCoordinateSpace::Local);

					GraphEdge currentEdge = newEdges[index];
					int32 splineIndex = 1;
					if (!newEdges[index].IsToInvert())
					{
						while (!currentEdge.GetToNode().Equals(toNode))
						{
							bool found = false;
							for (TActorIterator<ANodeActor> ActorItr(World); ActorItr; ++ActorItr)
							{
								if (ActorItr->Label.Equals(currentEdge.GetToNode().TrimEnd()))
								{
									found = true;
									Edge->Spline->AddSplinePointAtIndex(ActorItr->GetActorLocation(), splineIndex, ESplineCoordinateSpace::Local);
									splineIndex++;
									for (int32 x = 0; x < newEdges.Num(); x++)
									{
										if (newEdges[x].GetFromNode().Equals(currentEdge.GetToNode()) && newEdges[x].GetLabel().Equals(label))
										{
											currentEdge = newEdges[x];
											break;
										}
									}
								}
								if (found)
								{
									break;
								}
							}
						}
					}
					else
					{
						for (int32 y = 0; y < newEdges.Num(); y++)
						{
							if (newEdges[y].GetFromNode().Contains("virtual") && newEdges[y].GetToNode().Contains(fromNode) && newEdges[y].GetLabel().Equals(newEdges[index].GetLabel()))
							{
								currentEdge = newEdges[y];
							}
						}
						while (!currentEdge.GetFromNode().Equals(toNode))
						{
							bool found = false;
							for (TActorIterator<ANodeActor> ActorItr(World); ActorItr; ++ActorItr)
							{
								if (ActorItr->Label.Equals(currentEdge.GetFromNode().TrimEnd()))
								{
									found = true;
									Edge->Spline->AddSplinePointAtIndex(ActorItr->GetActorLocation(), splineIndex, ESplineCoordinateSpace::Local);
									splineIndex++;
									for (int32 x = 0; x < newEdges.Num(); x++)
									{
										if (newEdges[x].GetToNode().Equals(currentEdge.GetFromNode()) && newEdges[x].GetLabel().Equals(newEdges[index].GetLabel()))
										{
											currentEdge = newEdges[x];
											break;
										}
									}
								}
								if (found)
								{
									break;
								}
							}
						}
					}

					Edge->Spline->SetLocationAtSplinePoint(Edge->Spline->GetNumberOfSplinePoints() - 1, endingLocation, ESplineCoordinateSpace::Local);
					//if (splineIndex == 1)
					//{
					//	Edge->Spline->AddSplinePointAtIndex(ComputeMiddleSplinePointLocation(Edge->Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local), Edge->Spline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local)), 1, ESplineCoordinateSpace::Local);
					//}


					float signScale;
					double timeScale;
					switch (ScalingMethod)
					{
					case EScalingMethod::VE_Discrete:

						if (significance < MinSign + (MaxSign - MinSign) / 5)
						{
							signScale = 0.2f;
						}
						else if (significance < MinSign + (MaxSign - MinSign) * 2 / 5)
						{
							signScale = 0.4f;
						}
						else if (significance < MinSign + (MaxSign - MinSign) * 3 / 5)
						{
							signScale = 0.6f;
						}
						else if (significance < MinSign + (MaxSign - MinSign) * 4 / 5)
						{
							signScale = 0.8f;
						}
						else if (significance <= MinSign + (MaxSign - MinSign))
						{
							signScale = 1;
						}

						break;
					case EScalingMethod::VE_Continuous:

						signScale = ((float(significance) - float(MinSign)) / float(MaxSign)) + 0.2f;
						//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

						break;
					case EScalingMethod::VE_MinMax:

						signScale = ((float(significance) - float(MinSign)) / (float(MaxSign) - float(MinSign))) + 0.2f;
						//GLog->Log("(significance:" + FString::FromInt(significance) + " - Min:" + FString::FromInt(Min) + ") / Max:" + FString::FromInt(Max) + " = scale:" + FString::SanitizeFloat(scale));

						break;
					default:
						break;
					}

					switch (ScalingMethod)
					{
					case EScalingMethod::VE_Discrete:

						if (duration < MinTime + (MaxTime - MinTime) / 5)
						{
							timeScale = 0.2f;
						}
						else if (duration < MinTime + (MaxTime - MinTime) * 2 / 5)
						{
							timeScale = 0.4f;
						}
						else if (duration < MinTime + (MaxTime - MinTime) * 3 / 5)
						{
							timeScale = 0.6f;
						}
						else if (duration < MinTime + (MaxTime - MinTime) * 4 / 5)
						{
							timeScale = 0.8f;
						}
						else if (duration <= MinTime + (MaxTime - MinTime))
						{
							timeScale = 1;
						}

						break;
					case EScalingMethod::VE_Continuous:

						timeScale = ((float(duration) - float(MinTime)) / float(MaxTime));

						break;
					case EScalingMethod::VE_MinMax:

						timeScale = ((float(duration) - float(MinTime)) / (float(MaxTime) - float(MinTime)));

						break;
					default:
						break;
					}

					Edge->Significance = significance;

					Edge->CaseFrequency = float(caseFrequency) * 100 / totalNumberofCases;

					Edge->SignificanceScale = signScale / 2;

					Edge->Duration = duration;

					Edge->Durations = durations;

					if (MinTime == 0 && MaxTime == 0)
					{
						Edge->TimeScale = 0;
					}
					else
					{
						Edge->TimeScale = timeScale;
					}
				}
			}
		}
	}

	UWorld* const World = GetWorld();
	if (World)
	{
		for (TActorIterator<ANodeActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			if (ActorItr->Label.Contains("virtual"))
			{
				ActorItr->Destroy();
			}
			else
			{
				int32 outgoingEdgesSignificance = 0;
				TArray<AEdgeActor*> outgoingEdges = TArray<AEdgeActor*>();
				for (TActorIterator<AEdgeActor> ActorItrEdge(World); ActorItrEdge; ++ActorItrEdge)
				{
					if (ActorItrEdge->FromNode->Label.Equals(ActorItr->Label))
					{
						outgoingEdgesSignificance += ActorItrEdge->Significance;
						outgoingEdges.Add(*ActorItrEdge);
					}
				}
				for (TActorIterator<AEdgeActor> ActorItrEdge(World); ActorItrEdge; ++ActorItrEdge)
				{
					if (ActorItrEdge->FromNode->Label.Equals(ActorItr->Label))
					{
						ActorItrEdge->PercentageFrequencyFromParentNode = (ActorItrEdge->Significance * 100) / outgoingEdgesSignificance;
					}
				}
				ActorItr->PercentageFrequencyToTotal = (ActorItr->Significance * 100) / totalNumberOfEvent;
			}
		}

		for (TActorIterator<AEdgeActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			ActorItr->PercentageFrequencyToTotal = (ActorItr->Significance * 100) / totalNumberofCases;

			ActorItr->SetEdgeProperties();
		}
	}

	retflag = false;
}
