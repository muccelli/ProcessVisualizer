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

FVector AInputParser::ComputeMiddleSplinePointLocation(FVector startSplinePointLocation, FVector endSplinePointLocation)
{

	FVector direction = (startSplinePointLocation - endSplinePointLocation);
	float distance = direction.Size();

	direction.Normalize();

	float halfDistance = distance * 0.5;

	FVector middleSplinePointLocation = startSplinePointLocation - (direction*halfDistance);

	middleSplinePointLocation.X += FMath::RandRange(-MiddleSplinePointDeviation, MiddleSplinePointDeviation);
	middleSplinePointLocation.Y += FMath::RandRange(-MiddleSplinePointDeviation, MiddleSplinePointDeviation);

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
				Node->SetFolderPath("Nodes");
				Node->SetActorLabel(label);

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

				Node->WidgetText = FText::AsCultureInvariant(label);

				Node->SetNodeLabelAndTransform();

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
				if (ActorItr->GetActorLabel() == fromNode)
				{
					FromNode = *ActorItr;
					startingLocation = ActorItr->GetActorLocation();
				}

				if (ActorItr->GetActorLabel() == toNode)
				{
					ToNode = *ActorItr;
					endingLocation = ActorItr->GetActorLocation();
				}
			}

			if (FromNode && ToNode)
			{
				AEdgeActor* Edge = (AEdgeActor*)World->SpawnActor(EdgeActorBP, location);
				Edge->SetFolderPath("Edges");
				Edge->SetActorLabel(label);
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
		if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") > MaxSign)
		{
			MaxSign = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
		}
		if (nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance") < MinSign)
		{
			MinSign = nodesArray[index]->AsObject()->GetIntegerField("frequencySignificance");
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

	FVector* location = new FVector(1000, 0, 0);

	for (int32 i = 0; i < NodesPositioning.Num(); i++)
	{
		location->Y = (NodesPositioning[i].Num() - 1)*(-NodesYDistance / 2);
		for (int32 j = 0; j < NodesPositioning[i].Num(); j++)
		{
			FString label = NodesPositioning[i][j];
			int32 significance = 0;
			double duration = 0;
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
					
				}
			}

			UWorld* const World = GetWorld();
			if (World)
			{
				//ANodeActor* Node = (ANodeActor*) World->SpawnActor(ANodeActor::StaticClass(), location);
				ANodeActor* Node = (ANodeActor*)World->SpawnActor(NodeActorBP, location);
				Node->SetFolderPath("Nodes");
				Node->SetActorLabel(label);

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
				Node->TimeScale = timeScale;
				GLog->Log(label + ": " + FString::SanitizeFloat(timeScale));

				Node->WidgetText = FText::AsCultureInvariant(label);

				Node->SetNodeLabelAndTransform();

			}
			location->Y += NodesYDistance;
		}
		location->Y = 0;
		location->X -= NodesZDistance;
	}

	FVector* graphLocation = new FVector(0, 0, ((location->Z + NodesZDistance) + 1000) / 2);
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
		GLog->Log("duration: " + FString::SanitizeFloat(duration));
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
				if (ActorItr->GetActorLabel() == fromNode)
				{
					FromNode = *ActorItr;
					startingLocation.X = ActorItr->GetActorLocation().X;
					startingLocation.Y = ActorItr->GetActorLocation().Y;
					startingLocation.Z = 0;
				}

				if (ActorItr->GetActorLabel() == toNode)
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
				Edge->SetFolderPath("Edges");
				Edge->SetActorLabel(label);
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
				GLog->Log(FString::SanitizeFloat(timeScale));

				Edge->SetEdgeProperties();
			}
		}
	}

	retflag = false;
}