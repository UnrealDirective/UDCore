// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "Subsystems/DirectiveUtilEditorActorSubsystem.h"
#include "Types/DirectiveUtilEditorTypes.h"
#include "Misc/AutomationTest.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Texture2D.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorActorSubsystemTest, "DirectiveUtilities.EditorActorSubsystemTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorActorSubsystemTest::RunTest(const FString& Parameters)
{
	UDirectiveUtilEditorActorSubsystem::FocusActorsInViewport({nullptr});

	// IsActorWithinBoxBounds should not crash and should return false with null Actor
	TestFalse("IsActorWithinBoxBounds should return false with null Actor",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinBoxBounds(nullptr, nullptr));

	// IsActorWithinSphereBounds should not crash and should return false with null Actor
	TestFalse("IsActorWithinSphereBounds should return false with null Actor",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinSphereBounds(nullptr, nullptr));

	// IsActorWithinCapsuleBounds should not crash and should return false with null Actor
	TestFalse("IsActorWithinCapsuleBounds should return false with null CapsuleComponent",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinCapsuleBounds(nullptr, nullptr));

	// FilterEmptyActors should not crash with an empty array
	TArray<AActor*> EmptyActors;
	TArray<AActor*> FilteredActors;
	UDirectiveUtilEditorActorSubsystem::FilterEmptyActors(EmptyActors, FilteredActors, Include);
	TestEqual("FilterEmptyActors with empty input should produce empty output", FilteredActors.Num(), 0);

	// FilterActorsByMaterialName should not crash with an empty array
	TArray<AActor*> MaterialFiltered;
	UDirectiveUtilEditorActorSubsystem::FilterActorsByMaterialName(EmptyActors, MaterialFiltered, TEXT("TestMaterial"), OverrideOnly, Include);
	TestEqual("FilterActorsByMaterialName with empty input should produce empty output", MaterialFiltered.Num(), 0);

	// FilterActorsByVertCount should not crash with an empty array
	TArray<AActor*> VertFiltered;
	UDirectiveUtilEditorActorSubsystem::FilterActorsByVertCount(EmptyActors, VertFiltered, 0, 1000, Include);
	TestEqual("FilterActorsByVertCount with empty input should produce empty output", VertFiltered.Num(), 0);

	// FilterActorsByBounds should not crash with an empty array
	TArray<AActor*> BoundsFiltered;
	UDirectiveUtilEditorActorSubsystem::FilterActorsByBounds(EmptyActors, BoundsFiltered, FVector::ZeroVector, FVector::OneVector, Include);
	TestEqual("FilterActorsByBounds with empty input should produce empty output", BoundsFiltered.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorActorSubsystemFilterTest, "DirectiveUtilities.EditorActorSubsystemFilterTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorActorSubsystemFilterTest::RunTest(const FString& Parameters)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (!Cube || !Sphere)
	{
		AddError(TEXT("Engine basic shapes unavailable for the include/exclude behaviour test."));
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!World)
	{
		AddError(TEXT("Failed to create a transient world for the filter test."));
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	auto SpawnWithMeshes = [World](const TArray<UStaticMesh*>& Meshes) -> AActor*
	{
		AActor* Actor = World->SpawnActor<AActor>();
		USceneComponent* Root = NewObject<USceneComponent>(Actor);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		for (UStaticMesh* Mesh : Meshes)
		{
			UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(Actor);
			MeshComponent->SetupAttachment(Root);
			MeshComponent->RegisterComponent();
			MeshComponent->SetStaticMesh(Mesh);
			Actor->AddInstanceComponent(MeshComponent);
		}
		return Actor;
	};

	AActor* ActorCubeAndSphere = SpawnWithMeshes({ Cube, Sphere });
	AActor* ActorCubeOnly = SpawnWithMeshes({ Cube });
	AActor* ActorNoMesh = SpawnWithMeshes({});
	const TArray<AActor*> Source = { ActorCubeAndSphere, ActorCubeOnly, ActorNoMesh };

	// Include: actors that contain the cube mesh.
	TArray<AActor*> Included;
	UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMesh(Source, Included, Cube, Include);
	TestTrue("Include: multi-mesh actor containing the cube is included", Included.Contains(ActorCubeAndSphere));
	TestTrue("Include: cube-only actor is included", Included.Contains(ActorCubeOnly));
	TestFalse("Include: actor with no mesh is excluded", Included.Contains(ActorNoMesh));

	// Exclude: actors that do NOT contain the cube. A multi-mesh actor that uses the cube in one slot
	// must still be excluded even though another slot uses a different mesh (the aggregation fix).
	TArray<AActor*> Excluded;
	UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMesh(Source, Excluded, Cube, Exclude);
	TestFalse("Exclude: multi-mesh actor containing the cube is not mistakenly included", Excluded.Contains(ActorCubeAndSphere));
	TestFalse("Exclude: cube-only actor is excluded", Excluded.Contains(ActorCubeOnly));
	TestTrue("Exclude: actor with no mesh is included", Excluded.Contains(ActorNoMesh));

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorActorSubsystemFilterCoverageTest, "DirectiveUtilities.EditorActorSubsystemFilterCoverageTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorActorSubsystemFilterCoverageTest::RunTest(const FString& Parameters)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* MatA = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInterface* MatB = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"));
	if (!Cube || !Sphere || !MatA || !MatB)
	{
		AddError(TEXT("Engine basic shapes/materials unavailable for the filter coverage test."));
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!World)
	{
		AddError(TEXT("Failed to create a transient world for the filter coverage test."));
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	auto SpawnMesh = [World](UStaticMesh* Mesh, EComponentMobility::Type Mobility) -> UStaticMeshComponent*
	{
		AActor* Actor = World->SpawnActor<AActor>();
		USceneComponent* Root = NewObject<USceneComponent>(Actor);
		Root->SetMobility(Mobility);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(Actor);
		MeshComponent->SetMobility(Mobility);
		MeshComponent->SetupAttachment(Root);
		if (Mesh) { MeshComponent->SetStaticMesh(Mesh); }
		MeshComponent->RegisterComponent();
		Actor->AddInstanceComponent(MeshComponent);
		return MeshComponent;
	};

	// Actor A: cube, MatA override, Static mobility, BlockAll collision, tag Alpha, at origin.
	UStaticMeshComponent* CompA = SpawnMesh(Cube, EComponentMobility::Static);
	AActor* ActorA = CompA->GetOwner();
	CompA->SetMaterial(0, MatA);
	CompA->SetCollisionProfileName(TEXT("BlockAll"));
	ActorA->Tags.Add(FName("Alpha"));
	ActorA->SetActorLocation(FVector::ZeroVector);

	// Actor B: sphere, MatB override, Movable mobility, far away, no tag.
	UStaticMeshComponent* CompB = SpawnMesh(Sphere, EComponentMobility::Movable);
	AActor* ActorB = CompB->GetOwner();
	CompB->SetMaterial(0, MatB);
	ActorB->SetActorLocation(FVector(100000.0f, 0.0f, 0.0f));

	// Actor C: no static mesh component at all.
	AActor* ActorC = World->SpawnActor<AActor>();
	USceneComponent* RootC = NewObject<USceneComponent>(ActorC);
	ActorC->SetRootComponent(RootC);
	RootC->RegisterComponent();

	const TArray<AActor*> Src = { ActorA, ActorB, ActorC };
	auto RunFilter = [&Src](TFunctionRef<void(const TArray<AActor*>&, TArray<AActor*>&)> Fn) -> TArray<AActor*>
	{
		TArray<AActor*> Out;
		Fn(Src, Out);
		return Out;
	};

	// Class: everything is an AActor.
	TestEqual("ByClass(AActor) includes all", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByClass(S, O, AActor::StaticClass(), Include); }).Num(), 3);

	// Tag (Include + Exclude complement).
	{
		const TArray<AActor*> Inc = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByTag(S, O, FName("Alpha"), Include); });
		const TArray<AActor*> Exc = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByTag(S, O, FName("Alpha"), Exclude); });
		TestTrue("ByTag Include => A only", Inc.Contains(ActorA) && !Inc.Contains(ActorB) && !Inc.Contains(ActorC));
		TestTrue("ByTag Exclude => B and C", !Exc.Contains(ActorA) && Exc.Contains(ActorB) && Exc.Contains(ActorC));
	}

	// Static mesh by reference + by name.
	{
		const TArray<AActor*> Inc = RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMesh(S, O, Cube, Include); });
		TestTrue("ByStaticMesh(Cube) => A only", Inc.Contains(ActorA) && !Inc.Contains(ActorB) && !Inc.Contains(ActorC));
		const TArray<AActor*> ByName = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMeshName(S, O, TEXT("Cube"), Include); });
		TestTrue("ByStaticMeshName(Cube) => A", ByName.Contains(ActorA) && !ByName.Contains(ActorB));
	}

	// Material by reference + by name (override slot).
	{
		const TArray<AActor*> Inc = RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByMaterial(S, O, MatA, OverrideOnly, Include); });
		const TArray<AActor*> Exc = RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByMaterial(S, O, MatA, OverrideOnly, Exclude); });
		TestTrue("ByMaterial(MatA) Include => A only", Inc.Contains(ActorA) && !Inc.Contains(ActorB));
		TestTrue("ByMaterial(MatA) Exclude => B and C, not A", !Exc.Contains(ActorA) && Exc.Contains(ActorB) && Exc.Contains(ActorC));
		const TArray<AActor*> ByName = RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByMaterialName(S, O, MatA->GetName(), OverrideOnly, Include); });
		TestTrue("ByMaterialName(MatA) => A", ByName.Contains(ActorA) && !ByName.Contains(ActorB));
	}

	// Vert / tri count: query the cube's actual counts, then assert in-range includes and out-of-range excludes.
	{
		const int32 Verts = Cube->GetNumVertices(0);
		TestTrue("ByVertCount [V,V] => A", RunFilter([Verts](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByVertCount(S, O, Verts, Verts, Include); }).Contains(ActorA));
		TestFalse("ByVertCount out-of-range => not A", RunFilter([Verts](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByVertCount(S, O, Verts + 100000, Verts + 200000, Include); }).Contains(ActorA));
		const int32 Tris = Cube->GetNumTriangles(0);
		TestTrue("ByTriCount [T,T] => A", RunFilter([Tris](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByTriCount(S, O, Tris, Tris, Include); }).Contains(ActorA));
	}

	// Mobility.
	{
		TestTrue("ByMobility(Static) => A, not B", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByMobility(S, O, EComponentMobility::Static, Include); }).Contains(ActorA));
		TestTrue("ByMobility(Movable) => B, not A", RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByMobility(S, O, EComponentMobility::Movable, Include); }).Contains(ActorB));
	}

	// Collision (BlockAll on A implies WorldStatic object type, QueryAndPhysics, blocking responses).
	{
		TestTrue("ByCollisionProfile(BlockAll) => A", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionProfile(S, O, FName("BlockAll"), Include); }).Contains(ActorA));
		TestTrue("ByCollisionChannel(WorldStatic) => A", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionChannel(S, O, ECC_WorldStatic, Include); }).Contains(ActorA));
		TestTrue("ByCollisionEnabled(QueryAndPhysics) => A", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionEnabled(S, O, ECollisionEnabled::QueryAndPhysics, Include); }).Contains(ActorA));
		TestTrue("ByCollisionResponse(WorldDynamic, Block) => A", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByCollisionResponse(S, O, ECC_WorldDynamic, ECR_Block, Include); }).Contains(ActorA));
	}

	// Nanite: assert the filter partitions correctly (A matched by exactly one of true/false) without
	// reading the deprecated member directly.
	{
		const bool bInFalse = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByNaniteState(S, O, false, Include); }).Contains(ActorA);
		const bool bInTrue = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByNaniteState(S, O, true, Include); }).Contains(ActorA);
		TestTrue("ByNaniteState partitions A into exactly one of true/false", bInFalse != bInTrue);
	}

	// LOD count: query the cube's LOD count and assert in-range includes A.
	{
		const int32 LODs = Cube->GetNumLODs();
		TestTrue("ByLODCount [n,n] => A", RunFilter([LODs](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByLODCount(S, O, LODs, LODs, Include); }).Contains(ActorA));
	}

	// Actor bounds: query A's own size and assert a range around it includes A.
	{
		FVector Origin, Extent;
		ActorA->GetActorBounds(false, Origin, Extent);
		const FVector Size = Extent * 2.0f;
		TestTrue("ByBounds around A's size => A", RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByBounds(S, O, Size - FVector(1.0f), Size + FVector(1.0f), Include); }).Contains(ActorA));
	}

	// Static-mesh bounds use the source mesh dimensions, independently of actor transform.
	{
		const FVector MeshSize = Cube->GetBounds().BoxExtent * 2.0f;
		const TArray<AActor*> Match = RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O)
		{
			UDirectiveUtilEditorActorSubsystem::FilterActorsByStaticMeshBounds(
				S, O, MeshSize - FVector(1.0f), MeshSize + FVector(1.0f), Include);
		});
		TestTrue("ByStaticMeshBounds includes the cube actor", Match.Contains(ActorA));
		TestFalse("ByStaticMeshBounds excludes the actor without a mesh", Match.Contains(ActorC));
	}

	// Override lightmap resolution is a distinct search lane from the mesh default.
	{
		CompA->bOverrideLightMapRes = true;
		CompA->OverriddenLightMapRes = 128;
		const TArray<AActor*> Match = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O)
		{
			UDirectiveUtilEditorActorSubsystem::FilterActorsByLightmapResolution(
				S, O, 128, 128, OverrideOnly, Include);
		});
		TestTrue("ByLightmapResolution finds the exact override", Match.Contains(ActorA));
		TestFalse("ByLightmapResolution rejects a different override", Match.Contains(ActorB));
	}

	// World location: A at origin, B far away.
	{
		const TArray<AActor*> Near = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByWorldLocation(S, O, FVector::ZeroVector, 50.0f, Include); });
		TestTrue("ByWorldLocation near origin => A, not the far B", Near.Contains(ActorA) && !Near.Contains(ActorB));
	}

	// Texture by name: a name no material uses -> Include matches none, Exclude matches all (exercises traversal).
	{
		TestEqual("ByTextureName(absent) Include => none", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByTextureName(S, O, TEXT("__udcore_absent_texture__"), BaseAndOverride, Include); }).Num(), 0);
		TestEqual("ByTextureName(absent) Exclude => all", RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O){ UDirectiveUtilEditorActorSubsystem::FilterActorsByTextureName(S, O, TEXT("__udcore_absent_texture__"), BaseAndOverride, Exclude); }).Num(), 3);
	}

	// Texture-reference filtering must follow the same include/exclude contract.
	{
		UTexture2D* AbsentTexture = NewObject<UTexture2D>(GetTransientPackage());
		const TArray<AActor*> Inc = RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O)
		{
			UDirectiveUtilEditorActorSubsystem::FilterActorsByTexture(S, O, AbsentTexture, BaseAndOverride, Include);
		});
		const TArray<AActor*> Exc = RunFilter([&](const TArray<AActor*>& S, TArray<AActor*>& O)
		{
			UDirectiveUtilEditorActorSubsystem::FilterActorsByTexture(S, O, AbsentTexture, BaseAndOverride, Exclude);
		});
		TestEqual("ByTexture(absent) Include returns none", Inc.Num(), 0);
		TestEqual("ByTexture(absent) Exclude returns every valid source actor", Exc.Num(), Src.Num());
	}

	// Missing-resource convenience filters are semantic aliases, not merely spawn-tested nodes.
	{
		UStaticMesh* MissingMaterialMesh = DuplicateObject<UStaticMesh>(Cube, GetTransientPackage());
		MissingMaterialMesh->SetMaterial(0, nullptr);
		UStaticMeshComponent* MissingMaterialComp = SpawnMesh(MissingMaterialMesh, EComponentMobility::Static);
		UStaticMeshComponent* MissingMeshComp = SpawnMesh(nullptr, EComponentMobility::Static);
		const TArray<AActor*> MissingSource = { MissingMaterialComp->GetOwner(), MissingMeshComp->GetOwner(), ActorA };
		TArray<AActor*> MissingMaterials;
		UDirectiveUtilEditorActorSubsystem::FilterActorsByMissingMaterials(
			MissingSource, MissingMaterials, BaseOnly, Include);
		TestTrue("MissingMaterials finds the null source material", MissingMaterials.Contains(MissingMaterialComp->GetOwner()));
		TestFalse("MissingMaterials rejects the valid cube material", MissingMaterials.Contains(ActorA));
		TArray<AActor*> MissingMeshes;
		UDirectiveUtilEditorActorSubsystem::FilterActorsByMissingStaticMeshes(MissingSource, MissingMeshes, Include);
		TestTrue("MissingStaticMeshes finds a mesh component with no mesh", MissingMeshes.Contains(MissingMeshComp->GetOwner()));
		TestFalse("MissingStaticMeshes rejects a valid mesh", MissingMeshes.Contains(ActorA));
	}

	{
		UMaterial* MissingTextureMaterial = NewObject<UMaterial>(GetTransientPackage());
		UMaterialExpressionTextureSample* MissingTextureExpression = NewObject<UMaterialExpressionTextureSample>(MissingTextureMaterial);
		MissingTextureMaterial->GetExpressionCollection().AddExpression(MissingTextureExpression);
		CompA->SetMaterial(0, MissingTextureMaterial);
		const TArray<AActor*> MissingTextures = RunFilter([](const TArray<AActor*>& S, TArray<AActor*>& O)
		{
			UDirectiveUtilEditorActorSubsystem::FilterActorsByMissingTextures(S, O, OverrideOnly, Include);
		});
		TestTrue("Missing texture filter should include an actor with an unset texture expression", MissingTextures.Contains(ActorA));
		TestFalse("Missing texture filter should exclude actors without unset texture expressions", MissingTextures.Contains(ActorB));
		CompA->SetMaterial(0, MatA);
	}

	{
		AActor* DestroyedActor = World->SpawnActor<AActor>();
		World->DestroyActor(DestroyedActor);
		TestFalse("Destroyed actor should be invalid", IsValid(DestroyedActor));
		TArray<AActor*> InvalidActors = {DestroyedActor};
		TArray<AActor*> FilteredInvalidActors;
		UDirectiveUtilEditorActorSubsystem::FilterActorsByTag(
			InvalidActors,
			FilteredInvalidActors,
			TEXT("Absent"),
			Exclude);
		TestEqual("Actor filters should skip invalid actors", FilteredInvalidActors.Num(), 0);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorActorSubsystemEmptyActorsTest, "DirectiveUtilities.EditorActorSubsystemEmptyActorsTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorActorSubsystemEmptyActorsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!World)
	{
		AddError(TEXT("Failed to create a transient world for the empty actors test."));
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	// A bare actor whose only component is a childless scene root: the "Empty Actor" shape.
	AActor* BareActor = World->SpawnActor<AActor>();
	USceneComponent* BareRoot = NewObject<USceneComponent>(BareActor);
	BareActor->SetRootComponent(BareRoot);
	BareRoot->RegisterComponent();

	AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();

	const TArray<AActor*> Source = { BareActor, MeshActor };

	TArray<AActor*> Included;
	UDirectiveUtilEditorActorSubsystem::FilterEmptyActors(Source, Included, Include);
	TestTrue("Include: bare actor with only a scene root is empty", Included.Contains(BareActor));
	TestFalse("Include: static mesh actor is not empty", Included.Contains(MeshActor));
	TestEqual("Include: only the bare actor is returned", Included.Num(), 1);

	TArray<AActor*> Excluded;
	UDirectiveUtilEditorActorSubsystem::FilterEmptyActors(Source, Excluded, Exclude);
	TestFalse("Exclude: bare actor is not returned", Excluded.Contains(BareActor));
	TestTrue("Exclude: static mesh actor is returned", Excluded.Contains(MeshActor));
	TestEqual("Exclude: only the static mesh actor is returned", Excluded.Num(), 1);

	// Aliasing: filtering an array into itself rebuilds it in place.
	TArray<AActor*> Aliased = { BareActor, MeshActor };
	UDirectiveUtilEditorActorSubsystem::FilterEmptyActors(Aliased, Aliased, Include);
	TestEqual("Aliased: array is rebuilt in place with one entry", Aliased.Num(), 1);
	TestTrue("Aliased: only the bare actor remains", Aliased.Contains(BareActor));

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorActorSubsystemQueryAlignmentTest, "DirectiveUtilities.EditorActorSubsystemQueryAlignmentTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorActorSubsystemQueryAlignmentTest::RunTest(const FString& Parameters)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* BasicMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Cube || !BasicMaterial)
	{
		AddError(TEXT("Engine basic shapes or materials unavailable for the query alignment test."));
		return false;
	}

	// The GetActorsBy* queries read from the editor world rather than a passed array.
	UWorld* EditorWorld = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor && Context.World())
		{
			EditorWorld = Context.World();
			break;
		}
	}
	if (!EditorWorld)
	{
		AddError(TEXT("No editor world available for the query alignment test."));
		return false;
	}

	AActor* MeshActor = EditorWorld->SpawnActor<AActor>();
	USceneComponent* Root = NewObject<USceneComponent>(MeshActor);
	Root->SetMobility(EComponentMobility::Movable);
	MeshActor->SetRootComponent(Root);
	Root->RegisterComponent();
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(MeshActor);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetStaticMesh(Cube);
	MeshComponent->SetMaterial(0, BasicMaterial);
	MeshComponent->RegisterComponent();
	MeshActor->AddInstanceComponent(MeshComponent);
	MeshActor->SetActorLocation(FVector::ZeroVector);

	// The query methods keep no instance state, so a transient instance is enough headless.
	UDirectiveUtilEditorActorSubsystem* Subsystem = NewObject<UDirectiveUtilEditorActorSubsystem>();

	TArray<AActor*> ByClass;
	Subsystem->GetActorsByClass(ByClass, AActor::StaticClass(), World, Include);
	TestTrue("Class: AActor query finds the fixture actor", ByClass.Contains(MeshActor));

	// Bounding box: an enclosing box finds the actor, a disjoint one does not.
	TArray<AActor*> InBox;
	Subsystem->GetActorsByBoundingBox(InBox, FVector(-100000.0f), FVector(100000.0f), World, Include);
	TestTrue("BoundingBox: enclosing box finds the actor", InBox.Contains(MeshActor));
	TArray<AActor*> OutOfBox;
	Subsystem->GetActorsByBoundingBox(OutOfBox, FVector(900000.0f), FVector(900100.0f), World, Include);
	TestFalse("BoundingBox: disjoint box does not find the actor", OutOfBox.Contains(MeshActor));

	// Mesh name: a lowercase substring matches case-insensitively, as in the Filter variant.
	TArray<AActor*> ByName;
	Subsystem->GetActorsByStaticMeshName(ByName, TEXT("cub"), World, Include);
	TestTrue("StaticMeshName: lowercase substring finds the actor", ByName.Contains(MeshActor));

	TArray<AActor*> ByMaterial;
	Subsystem->GetActorsByMaterial(ByMaterial, BasicMaterial, OverrideOnly, World, Include);
	TestTrue("Material: generic actor with a static mesh component is included", ByMaterial.Contains(MeshActor));
	TArray<AActor*> ByMaterialSoftReference;
	Subsystem->GetActorsByMaterialSoftReference(
		ByMaterialSoftReference, BasicMaterial, OverrideOnly, World, Include);
	TestTrue("Material soft reference: generic actor with a static mesh component is included",
		ByMaterialSoftReference.Contains(MeshActor));
	TArray<AActor*> ByMaterialName;
	Subsystem->GetActorsByMaterialName(
		ByMaterialName, BasicMaterial->GetName(), OverrideOnly, World, Include);
	TestTrue("Material name: generic actor with a static mesh component is included",
		ByMaterialName.Contains(MeshActor));
	TArray<AActor*> WithoutMaterial;
	Subsystem->GetActorsByMaterial(WithoutMaterial, BasicMaterial, OverrideOnly, World, Exclude);
	TestFalse("Material exclude: matching generic actor is excluded", WithoutMaterial.Contains(MeshActor));

	const int32 VertexCount = Cube->GetNumVertices(0);
	TArray<AActor*> ByVertexCount;
	Subsystem->GetActorsByVertexCount(ByVertexCount, VertexCount, VertexCount, World, Include);
	TestTrue("VertexCount: exact range finds the fixture actor", ByVertexCount.Contains(MeshActor));
	const int32 TriangleCount = Cube->GetNumTriangles(0);
	TArray<AActor*> ByTriangleCount;
	Subsystem->GetActorsByTriCount(ByTriangleCount, TriangleCount, TriangleCount, World, Include);
	TestTrue("TriangleCount: exact range finds the fixture actor", ByTriangleCount.Contains(MeshActor));

	const float MeshSize = Cube->GetBoundingBox().GetSize().Size();
	TArray<AActor*> ByMeshSize;
	Subsystem->GetActorsByMeshSize(ByMeshSize, MeshSize - 1.0f, MeshSize + 1.0f, World, Include);
	TestTrue("MeshSize: enclosing range finds the fixture actor", ByMeshSize.Contains(MeshActor));
	TArray<AActor*> ByWorldLocation;
	Subsystem->GetActorsByWorldLocation(ByWorldLocation, FVector::ZeroVector, 100.0f, World, Include);
	TestTrue("WorldLocation: nearby query finds the fixture actor", ByWorldLocation.Contains(MeshActor));

	const int32 LODCount = Cube->GetNumLODs();
	TArray<AActor*> ByLODCount;
	Subsystem->GetActorsByLODCount(ByLODCount, LODCount, LODCount, World, Include);
	TestTrue("LODCount: exact range finds the fixture actor", ByLODCount.Contains(MeshActor));
	TArray<AActor*> NaniteOff;
	TArray<AActor*> NaniteOn;
	Subsystem->GetActorsByNaniteEnabled(NaniteOff, false, World, Include);
	Subsystem->GetActorsByNaniteEnabled(NaniteOn, true, World, Include);
	TestTrue("Nanite: fixture belongs to exactly one state", NaniteOff.Contains(MeshActor) != NaniteOn.Contains(MeshActor));

	const int32 SourceLightmapResolution = Cube->GetLightMapResolution();
	TArray<AActor*> ByLightmapResolution;
	Subsystem->GetActorsByLightmapResolution(
		ByLightmapResolution, SourceLightmapResolution, SourceLightmapResolution, World, Include);
	TestTrue("LightmapResolution: exact source resolution finds the fixture actor", ByLightmapResolution.Contains(MeshActor));
	TArray<AActor*> ByMobility;
	Subsystem->GetActorsByMobility(ByMobility, EComponentMobility::Movable, World, Include);
	TestTrue("Mobility query finds the movable fixture actor", ByMobility.Contains(MeshActor));

	TArray<AActor*> ByStaticMesh;
	Subsystem->GetActorsByStaticMesh(ByStaticMesh, Cube, World, Include);
	TestTrue("StaticMesh reference query finds the fixture actor", ByStaticMesh.Contains(MeshActor));
	TArray<AActor*> ByStaticMeshSoft;
	Subsystem->GetActorsByStaticMeshSoftReference(ByStaticMeshSoft, Cube, World, Include);
	TestTrue("StaticMesh soft-reference query finds the fixture actor", ByStaticMeshSoft.Contains(MeshActor));

	UTexture2D* AbsentTexture = NewObject<UTexture2D>(GetTransientPackage());
	TArray<AActor*> ByTexture;
	Subsystem->GetActorsByTexture(ByTexture, AbsentTexture, World, Include);
	TestFalse("Texture reference query rejects an absent texture", ByTexture.Contains(MeshActor));
	TArray<AActor*> ByTextureSoft;
	Subsystem->GetActorsByTextureSoftReference(ByTextureSoft, AbsentTexture, World, Include);
	TestFalse("Texture soft-reference query rejects an absent texture", ByTextureSoft.Contains(MeshActor));
	TArray<AActor*> ByTextureName;
	Subsystem->GetActorsByTextureName(ByTextureName, TEXT("__udcore_absent_texture__"), World, Include);
	TestFalse("Texture-name query rejects an absent texture", ByTextureName.Contains(MeshActor));

	TArray<AActor*> InvalidActors = { MeshActor };
	Subsystem->GetInvalidActors(InvalidActors);
	TestEqual("GetInvalidActors resets its deprecated output", InvalidActors.Num(), 0);

	// Mobility: the root component's mobility is what counts.
	const TArray<AActor*> Source = { MeshActor };
	TArray<AActor*> MovableActors;
	UDirectiveUtilEditorActorSubsystem::FilterActorsByMobility(Source, MovableActors, EComponentMobility::Movable, Include);
	TestTrue("Mobility: movable root is matched", MovableActors.Contains(MeshActor));
	TArray<AActor*> StaticActors;
	UDirectiveUtilEditorActorSubsystem::FilterActorsByMobility(Source, StaticActors, EComponentMobility::Static, Include);
	TestFalse("Mobility: static does not match a movable root", StaticActors.Contains(MeshActor));

	EditorWorld->DestroyActor(MeshActor);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilPushOverrideMaterialsTest, "DirectiveUtilities.PushOverrideMaterialsTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilPushOverrideMaterialsTest::RunTest(const FString& Parameters)
{
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInterface* OverrideMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"));
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!BaseMaterial || !OverrideMaterial || !Cube)
	{
		AddError(TEXT("Engine materials unavailable for the override material test."));
		return false;
	}

	UStaticMesh* StaticMesh = DuplicateObject<UStaticMesh>(Cube, GetTransientPackage());
	StaticMesh->GetStaticMaterials()[0].MaterialInterface = BaseMaterial;
	UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());
	StaticMeshComponent->SetStaticMesh(StaticMesh);
	StaticMeshComponent->SetMaterial(0, OverrideMaterial);

	UDirectiveUtilEditorActorSubsystem::PushOverrideMaterialsToSource(StaticMeshComponent);
	TestEqual("Override material is copied to its source slot", StaticMesh->GetMaterial(0), OverrideMaterial);

	UStaticMesh* MeshWithoutOverride = DuplicateObject<UStaticMesh>(Cube, GetTransientPackage());
	MeshWithoutOverride->GetStaticMaterials()[0].MaterialInterface = BaseMaterial;
	UStaticMeshComponent* ComponentWithoutOverride = NewObject<UStaticMeshComponent>(GetTransientPackage());
	ComponentWithoutOverride->SetStaticMesh(MeshWithoutOverride);
	UDirectiveUtilEditorActorSubsystem::PushOverrideMaterialsToSource(ComponentWithoutOverride);
	TestEqual("A source slot is unchanged when the component has no override",
		MeshWithoutOverride->GetMaterial(0), BaseMaterial);

	AddExpectedError(TEXT("Static Mesh Component is invalid."), EAutomationExpectedErrorFlags::Exact, 1);
	UDirectiveUtilEditorActorSubsystem::PushOverrideMaterialsToSource(nullptr);
	UStaticMeshComponent* ComponentWithoutMesh = NewObject<UStaticMeshComponent>(GetTransientPackage());
	AddExpectedError(TEXT("Static Mesh Component has no valid static mesh."), EAutomationExpectedErrorFlags::Exact, 1);
	UDirectiveUtilEditorActorSubsystem::PushOverrideMaterialsToSource(ComponentWithoutMesh);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorActorSubsystemBoundsTest, "DirectiveUtilities.EditorActorSubsystemBoundsTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorActorSubsystemBoundsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!World)
	{
		AddError(TEXT("Failed to create a transient world for the bounds test."));
		return false;
	}
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Editor);
	WorldContext.SetCurrentWorld(World);

	auto SpawnPoint = [World](const FVector& Location, const FVector& Scale) -> AActor*
	{
		AActor* Actor = World->SpawnActor<AActor>();
		USceneComponent* Root = NewObject<USceneComponent>(Actor);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		Actor->SetActorScale3D(Scale);
		Actor->SetActorLocation(Location);
		return Actor;
	};

	// Axis-aligned box of extent 100 at the origin.
	AActor* BoxActor = World->SpawnActor<AActor>();
	USceneComponent* BoxRoot = NewObject<USceneComponent>(BoxActor);
	BoxActor->SetRootComponent(BoxRoot);
	BoxRoot->RegisterComponent();
	UBoxComponent* Box = NewObject<UBoxComponent>(BoxActor);
	Box->SetupAttachment(BoxRoot);
	Box->RegisterComponent();
	Box->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));

	// Point well inside the box, on an actor scaled to 0.1. The previous code multiplied the box
	// extent by the queried actor's scale and would wrongly report this as outside.
	TestTrue("Box: point inside is detected regardless of the queried actor's scale",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinBoxBounds(SpawnPoint(FVector(50.0f, 50.0f, 50.0f), FVector(0.1f)), Box));
	TestFalse("Box: point beyond the extent is rejected",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinBoxBounds(SpawnPoint(FVector(250.0f, 0.0f, 0.0f), FVector::OneVector), Box));

	// Capsule of radius 50 and half-height 100 at the origin.
	AActor* CapsuleActor = World->SpawnActor<AActor>();
	USceneComponent* CapsuleRoot = NewObject<USceneComponent>(CapsuleActor);
	CapsuleActor->SetRootComponent(CapsuleRoot);
	CapsuleRoot->RegisterComponent();
	UCapsuleComponent* Capsule = NewObject<UCapsuleComponent>(CapsuleActor);
	Capsule->SetupAttachment(CapsuleRoot);
	Capsule->RegisterComponent();
	Capsule->SetCapsuleSize(50.0f, 100.0f);

	TestTrue("Capsule: point within the upper cap is detected",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinCapsuleBounds(SpawnPoint(FVector(0.0f, 0.0f, 90.0f), FVector::OneVector), Capsule));
	TestTrue("Capsule: point within the radius is detected",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinCapsuleBounds(SpawnPoint(FVector(40.0f, 0.0f, 0.0f), FVector::OneVector), Capsule));
	TestFalse("Capsule: point beyond the radius is rejected (not a broad sphere)",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinCapsuleBounds(SpawnPoint(FVector(60.0f, 0.0f, 0.0f), FVector::OneVector), Capsule));
	TestFalse("Capsule: point beyond the end cap is rejected",
		UDirectiveUtilEditorActorSubsystem::IsActorWithinCapsuleBounds(SpawnPoint(FVector(0.0f, 0.0f, 200.0f), FVector::OneVector), Capsule));

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	return true;
}

#endif
