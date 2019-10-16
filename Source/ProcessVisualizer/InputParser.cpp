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
				int32 significance;
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

					Node->SphereVisual->SetWorldScale3D(FVector(scale*1.5, scale*1.5, scale*1.5));

					Node->WidgetText = FText::AsCultureInvariant(label);

					Node->SetNodeLabelAndTransform();

				}
				location->X += FMath::RandRange(-XNodeDeviation, XNodeDeviation);
				location->Y += NodesYDistance;
			}
			location->Y = 0;
			location->Z -= NodesZDistance;
		}

		

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
				AEdgeActor* Edge = (AEdgeActor*)World->SpawnActor(EdgeActorBP, location);
				Edge->SetActorLabel(label);

				for (TActorIterator<ANodeActor> ActorItr(World); ActorItr; ++ActorItr)
				{
					if (ActorItr->GetActorLabel() == fromNode)
					{
						Edge->FromNode = *ActorItr;
						Edge->Spline->SetLocationAtSplinePoint(0, ActorItr->GetActorLocation(), ESplineCoordinateSpace::Local);
					}
					
					if (ActorItr->GetActorLabel() == toNode)
					{
						Edge->ToNode = *ActorItr;
						Edge->Spline->SetLocationAtSplinePoint(Edge->Spline->GetNumberOfSplinePoints() - 1, ActorItr->GetActorLocation(), ESplineCoordinateSpace::Local);
					}
				}

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
	middleSplinePointLocation.Z += FMath::RandRange(-MiddleSplinePointDeviation, MiddleSplinePointDeviation);

	return middleSplinePointLocation;
}
