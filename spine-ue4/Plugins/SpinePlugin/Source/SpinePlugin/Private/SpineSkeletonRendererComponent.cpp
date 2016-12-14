#include "SpinePluginPrivatePCH.h"
#include "Engine.h"
#include "spine/spine.h"
#include <stdlib.h>

#define LOCTEXT_NAMESPACE "Spine"

USpineSkeletonRendererComponent::USpineSkeletonRendererComponent (const FObjectInitializer& ObjectInitializer) 
: URuntimeMeshComponent(ObjectInitializer) {
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterialRef(TEXT("/SpinePlugin/SpineUnlitNormalMaterial"));
	NormalBlendMaterial = NormalMaterialRef.Object;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterialRef(TEXT("/SpinePlugin/SpineUnlitAdditiveMaterial"));
	AdditiveBlendMaterial = AdditiveMaterialRef.Object;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterialRef(TEXT("/SpinePlugin/SpineUnlitMultiplyMaterial"));
	MultiplyBlendMaterial = MultiplyMaterialRef.Object;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ScreenMaterialRef(TEXT("/SpinePlugin/SpineUnlitScreenMaterial"));
	ScreenBlendMaterial = ScreenMaterialRef.Object;
	
	
	TextureParameterName = FName(TEXT("SpriteTexture"));
}

void USpineSkeletonRendererComponent::BeginPlay () {
	Super::BeginPlay();
}

void USpineSkeletonRendererComponent::TickComponent (float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	AActor* owner = GetOwner();
	if (owner) {
		UClass* skeletonClass = USpineSkeletonComponent::StaticClass();
		USpineSkeletonComponent* skeleton = Cast<USpineSkeletonComponent>(owner->GetComponentByClass(skeletonClass));
		
		if (skeleton && !skeleton->IsBeingDestroyed() && skeleton->GetSkeleton()) {
			if (atlasNormalBlendMaterials.Num() != skeleton->Atlas->atlasPages.Num()) {
				atlasNormalBlendMaterials.SetNum(0);
				pageToNormalBlendMaterial.Empty();
				atlasAdditiveBlendMaterials.SetNum(0);
				pageToAdditiveBlendMaterial.Empty();
				atlasMultiplyBlendMaterials.SetNum(0);
				pageToMultiplyBlendMaterial.Empty();
				atlasScreenBlendMaterials.SetNum(0);
				pageToScreenBlendMaterial.Empty();
				
				spAtlasPage* currPage = skeleton->Atlas->GetAtlas(false)->pages;
				for (int i = 0; i < skeleton->Atlas->atlasPages.Num(); i++) {
					
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(NormalBlendMaterial, owner);
					material->SetTextureParameterValue(TextureParameterName, skeleton->Atlas->atlasPages[i]);
					atlasNormalBlendMaterials.Add(material);
					pageToNormalBlendMaterial.Add(currPage, material);
					
					material = UMaterialInstanceDynamic::Create(AdditiveBlendMaterial, owner);
					material->SetTextureParameterValue(TextureParameterName, skeleton->Atlas->atlasPages[i]);
					atlasAdditiveBlendMaterials.Add(material);
					pageToAdditiveBlendMaterial.Add(currPage, material);
					
					material = UMaterialInstanceDynamic::Create(MultiplyBlendMaterial, owner);
					material->SetTextureParameterValue(TextureParameterName, skeleton->Atlas->atlasPages[i]);
					atlasMultiplyBlendMaterials.Add(material);
					pageToMultiplyBlendMaterial.Add(currPage, material);
					
					material = UMaterialInstanceDynamic::Create(ScreenBlendMaterial, owner);
					material->SetTextureParameterValue(TextureParameterName, skeleton->Atlas->atlasPages[i]);
					atlasScreenBlendMaterials.Add(material);
					pageToScreenBlendMaterial.Add(currPage, material);
					
					currPage = currPage->next;
				}
			} else {
				pageToNormalBlendMaterial.Empty();
				pageToAdditiveBlendMaterial.Empty();
				pageToMultiplyBlendMaterial.Empty();
				pageToScreenBlendMaterial.Empty();
				
				spAtlasPage* currPage = skeleton->Atlas->GetAtlas(false)->pages;
				for (int i = 0; i < skeleton->Atlas->atlasPages.Num(); i++) {
					UTexture2D* texture = skeleton->Atlas->atlasPages[i];
					UTexture* oldTexture = nullptr;
					
					UMaterialInstanceDynamic* current = atlasNormalBlendMaterials[i];
					if(!current->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture) {
						UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(NormalBlendMaterial, owner);
						material->SetTextureParameterValue(TextureParameterName, texture);
						atlasNormalBlendMaterials[i] = material;
					}
					pageToNormalBlendMaterial.Add(currPage, atlasNormalBlendMaterials[i]);
					
					current = atlasAdditiveBlendMaterials[i];
					if(!current->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture) {
						UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(AdditiveBlendMaterial, owner);
						material->SetTextureParameterValue(TextureParameterName, texture);
						atlasAdditiveBlendMaterials[i] = material;
					}
					pageToAdditiveBlendMaterial.Add(currPage, atlasAdditiveBlendMaterials[i]);
					
					current = atlasMultiplyBlendMaterials[i];
					if(!current->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture) {
						UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(MultiplyBlendMaterial, owner);
						material->SetTextureParameterValue(TextureParameterName, texture);
						atlasMultiplyBlendMaterials[i] = material;
					}
					pageToMultiplyBlendMaterial.Add(currPage, atlasMultiplyBlendMaterials[i]);
					
					current = atlasScreenBlendMaterials[i];
					if(!current->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture) {
						UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(ScreenBlendMaterial, owner);
						material->SetTextureParameterValue(TextureParameterName, texture);
						atlasScreenBlendMaterials[i] = material;
					}
					pageToScreenBlendMaterial.Add(currPage, atlasScreenBlendMaterials[i]);
					
					currPage = currPage->next;
				}
			}
			UpdateMesh(skeleton->GetSkeleton());
		} else {
			ClearAllMeshSections();
		}
	}
}

void USpineSkeletonRendererComponent::Flush (int &Idx, TArray<FVector> &Vertices, TArray<int32> &Indices, TArray<FVector2D> &Uvs, TArray<FColor> &Colors, UMaterialInstanceDynamic* Material) {
	if (Vertices.Num() == 0) return;
	SetMaterial(Idx, Material);
	CreateMeshSection(Idx, Vertices, Indices, TArray<FVector>(), Uvs, Colors, TArray<FRuntimeMeshTangent>(), false);
	Vertices.SetNum(0);
	Indices.SetNum(0);
	Uvs.SetNum(0);
	Colors.SetNum(0);
	Idx++;
}

void USpineSkeletonRendererComponent::UpdateMesh(spSkeleton* Skeleton) {
	TArray<FVector> vertices;
	TArray<int32> indices;
	TArray<FVector2D> uvs;
	TArray<FColor> colors;		

	TArray<float> worldVertices;
	worldVertices.SetNumUninitialized(2 * 1024);
	int idx = 0;
	int meshSection = 0;
	UMaterialInstanceDynamic* lastMaterial = nullptr;

	ClearAllMeshSections();

	float depthOffset = 0;

	for (int i = 0; i < Skeleton->slotsCount; ++i) {
		spSlot* slot = Skeleton->drawOrder[i];
		spAttachment* attachment = slot->attachment;
		if (!attachment) continue;						
		
		if (attachment->type == SP_ATTACHMENT_REGION) {
			spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;
			spAtlasRegion* region = (spAtlasRegion*)regionAttachment->rendererObject;
			
			UMaterialInstanceDynamic* material = nullptr;
			
			switch(slot->data->blendMode) {
				case SP_BLEND_MODE_NORMAL:
					material = pageToNormalBlendMaterial[region->page];
					break;
				case SP_BLEND_MODE_ADDITIVE:
					material = pageToAdditiveBlendMaterial[region->page];
					break;
				case SP_BLEND_MODE_MULTIPLY:
					material = pageToMultiplyBlendMaterial[region->page];
					break;
				case SP_BLEND_MODE_SCREEN:
				material = pageToScreenBlendMaterial[region->page];
					break;
				default:
					material = pageToNormalBlendMaterial[region->page];
			}

			if (lastMaterial != material) {
				Flush(meshSection, vertices, indices, uvs, colors, lastMaterial);				
				lastMaterial = material;
				idx = 0;
			}

			spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, worldVertices.GetData());

			uint8 r = static_cast<uint8>(Skeleton->r * slot->r * 255);
			uint8 g = static_cast<uint8>(Skeleton->g * slot->g * 255);
			uint8 b = static_cast<uint8>(Skeleton->b * slot->b * 255);
			uint8 a = static_cast<uint8>(Skeleton->a * slot->a * 255);
			
			colors.Add(FColor(r, g, b, a));
			vertices.Add(FVector(worldVertices[0], depthOffset, worldVertices[1]));
			uvs.Add(FVector2D(regionAttachment->uvs[0], regionAttachment->uvs[1]));

			colors.Add(FColor(r, g, b, a));
			vertices.Add(FVector(worldVertices[2], depthOffset, worldVertices[3]));
			uvs.Add(FVector2D(regionAttachment->uvs[2], regionAttachment->uvs[3]));

			colors.Add(FColor(r, g, b, a));
			vertices.Add(FVector(worldVertices[4], depthOffset, worldVertices[5]));
			uvs.Add(FVector2D(regionAttachment->uvs[4], regionAttachment->uvs[5]));

			colors.Add(FColor(r, g, b, a));
			vertices.Add(FVector(worldVertices[6], depthOffset, worldVertices[7]));
			uvs.Add(FVector2D(regionAttachment->uvs[6], regionAttachment->uvs[7]));

			indices.Add(idx + 0);
			indices.Add(idx + 1);
			indices.Add(idx + 2);
			indices.Add(idx + 0);
			indices.Add(idx + 2);
			indices.Add(idx + 3);
			idx += 4;
			depthOffset += this->DepthOffset;
		} else if (attachment->type == SP_ATTACHMENT_MESH) {
			spMeshAttachment* mesh = (spMeshAttachment*)attachment;
			spAtlasRegion* region = (spAtlasRegion*)mesh->rendererObject;
			UMaterialInstanceDynamic* material = pageToNormalBlendMaterial[region->page];

			if (lastMaterial != material) {
				Flush(meshSection, vertices, indices, uvs, colors, lastMaterial);
				lastMaterial = material;
				idx = 0;
			}

			if (mesh->super.worldVerticesLength> worldVertices.Num()) {
				worldVertices.SetNum(mesh->super.worldVerticesLength);
			}
			spMeshAttachment_computeWorldVertices(mesh, slot, worldVertices.GetData());

			uint8 r = static_cast<uint8>(Skeleton->r * slot->r * 255);
			uint8 g = static_cast<uint8>(Skeleton->g * slot->g * 255);
			uint8 b = static_cast<uint8>(Skeleton->b * slot->b * 255);
			uint8 a = static_cast<uint8>(Skeleton->a * slot->a * 255);
			
			for (int j = 0; j < mesh->super.worldVerticesLength; j += 2) {
				colors.Add(FColor(r, g, b, a));
				vertices.Add(FVector(worldVertices[j], depthOffset, worldVertices[j + 1]));
				uvs.Add(FVector2D(mesh->uvs[j], mesh->uvs[j + 1]));
			}

			for (int j = 0; j < mesh->trianglesCount; j++) {
				indices.Add(idx + mesh->triangles[j]);
			}
			idx += mesh->super.worldVerticesLength >> 1;
			depthOffset += this->DepthOffset;
			SetMaterial(meshSection, material);
		}
	}
	
	Flush(meshSection, vertices, indices, uvs, colors, lastMaterial);
}

#undef LOCTEXT_NAMESPACE