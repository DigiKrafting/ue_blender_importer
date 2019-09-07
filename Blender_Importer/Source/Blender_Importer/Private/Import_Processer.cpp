// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Import_Processer.h"
#include "AssetRegistryModule.h"
#include "Editor/UnrealEd/Public/Editor.h"
#include "Misc/FeedbackContext.h"
#include "Logging/LogMacros.h"
#include "Logging/TokenizedMessage.h"
#include "MessageLogModule.h"
#include "Serialization/JsonTypes.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "ContentBrowserModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"

#define LOCTEXT_NAMESPACE "FBlender_ImporterModule"

bool FImport_Processer::Process_JSON_Data(const FString& Filename)
{

	FString JsonString; 
	FFileHelper::LoadFileToString(JsonString, *Filename);
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		const FString data_path = JsonObject->GetStringField(TEXT("path"));
		
		// Process Materials

		TArray<TSharedPtr<FJsonValue>> objArray = JsonObject->GetArrayField(TEXT("materials"));
		for (int32 i = 0; i < objArray.Num(); i++)
		{
			TSharedPtr<FJsonValue> value = objArray[i];
			TSharedPtr<FJsonObject> json = value->AsObject();
			
			const FString data_name = json->GetStringField(TEXT("name"));
			const FString data_base_color = json->GetStringField(TEXT("base_color"));
			const FString data_orm = json->GetStringField(TEXT("orm"));
			const FString data_normal = json->GetStringField(TEXT("normal"));
			const FString data_ambient_occlusion = json->GetStringField(TEXT("ambient_occlusion"));
			const FString data_metallic = json->GetStringField(TEXT("metallic"));
			const FString data_roughness = json->GetStringField(TEXT("roughness"));

			FString PackageName = TEXT("/Game/") + data_path + data_name;

			UPackage* Package = CreatePackage(NULL, *PackageName);

			auto MaterialFactory = NewObject<UMaterialFactoryNew>();
			UMaterial* UnrealMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(UMaterial::StaticClass(), Package, *data_name, RF_Standalone | RF_Public, NULL, GWarn);
			FAssetRegistryModule::AssetCreated(UnrealMaterial);
			Package->FullyLoad();
			Package->SetDirtyFlag(true);
			
			// Base Color

			if (data_base_color != "") {
			
				FStringAssetReference AssetPath_Base_Color(TEXT("/Game/") + data_path + data_base_color);
				UTexture* Texture_Base_Color = Cast<UTexture>(AssetPath_Base_Color.TryLoad());
				if (Texture_Base_Color)
				{
					UMaterialExpressionTextureSample* TextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);
					TextureExpression->Texture = Texture_Base_Color;
					TextureExpression->SamplerType = SAMPLERTYPE_Color;
					UnrealMaterial->Expressions.Add(TextureExpression);
					UnrealMaterial->BaseColor.Expression = TextureExpression;
				}
			
			}
			
			// ORM

			if (data_orm != "") {
			
				FStringAssetReference AssetPath_ORM(TEXT("/Game/") + data_path + data_orm);
				UTexture* Texture_ORM = Cast<UTexture>(AssetPath_ORM.TryLoad());
				if (Texture_ORM)
				{
					UMaterialExpressionTextureSample* TextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);
					TextureExpression->Texture = Texture_ORM;
					TextureExpression->SamplerType = SAMPLERTYPE_Color;
					UnrealMaterial->Expressions.Add(TextureExpression);

					UnrealMaterial->AmbientOcclusion.Connect(1, TextureExpression); // R
					UnrealMaterial->Roughness.Connect(2, TextureExpression); // G
					UnrealMaterial->Metallic.Connect(3, TextureExpression); // B

				}
			
			} else {

				// Ambient Occlusion

				if (data_ambient_occlusion != "") {

					FStringAssetReference AssetPath_Ambient_Occlusion(TEXT("/Game/") + data_path + data_ambient_occlusion);
					UTexture* Texture_Ambient_Occlusion = Cast<UTexture>(AssetPath_Ambient_Occlusion.TryLoad());
					if (Texture_Ambient_Occlusion)
					{
						UMaterialExpressionTextureSample* TextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);
						TextureExpression->Texture = Texture_Ambient_Occlusion;
						TextureExpression->SamplerType = SAMPLERTYPE_Color;
						UnrealMaterial->Expressions.Add(TextureExpression);
						UnrealMaterial->AmbientOcclusion.Expression = TextureExpression;
					}

				}
				
				// Metallic

				if (data_metallic != "") {

					FStringAssetReference AssetPath_Metallic(TEXT("/Game/") + data_path + data_metallic);
					UTexture* Texture_Metallic = Cast<UTexture>(AssetPath_Metallic.TryLoad());
					if (Texture_Metallic)
					{
						UMaterialExpressionTextureSample* TextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);
						TextureExpression->Texture = Texture_Metallic;
						TextureExpression->SamplerType = SAMPLERTYPE_Color;
						UnrealMaterial->Expressions.Add(TextureExpression);
						UnrealMaterial->Metallic.Expression = TextureExpression;
					}

				}
				
				// Roughness

				if (data_roughness != "") {

					FStringAssetReference AssetPath_Roughness(TEXT("/Game/") + data_path + data_roughness);
					UTexture* Texture_Roughness = Cast<UTexture>(AssetPath_Roughness.TryLoad());
					if (Texture_Roughness)
					{
						UMaterialExpressionTextureSample* TextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);
						TextureExpression->Texture = Texture_Roughness;
						TextureExpression->SamplerType = SAMPLERTYPE_Color;
						UnrealMaterial->Expressions.Add(TextureExpression);
						UnrealMaterial->Roughness.Expression = TextureExpression;
					}

				}

			}

			// Normal

			if (data_normal != "") {
				
				FStringAssetReference AssetPath_Normal(TEXT("/Game/") + data_path + data_normal);
				UTexture* Texture_Normal = Cast<UTexture>(AssetPath_Normal.TryLoad());
				if (Texture_Normal)
				{
					UMaterialExpressionTextureSample* TextureExpression = NewObject<UMaterialExpressionTextureSample>(UnrealMaterial);
					TextureExpression->Texture = Texture_Normal;
					TextureExpression->SamplerType = SAMPLERTYPE_Normal;
					UnrealMaterial->Expressions.Add(TextureExpression);
					UnrealMaterial->Normal.Expression = TextureExpression;
				}

			}
			
			UnrealMaterial->PreEditChange(NULL);
			UnrealMaterial->PostEditChange();

		}
		
	} else {

		FText DialogText = FText::Format(
			LOCTEXT("Error deserializing/processing ", "[{0}]"),
			FText::FromString(Filename)
		);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		
		return false;

	}

	return true;

}

#undef LOCTEXT_NAMESPACE
