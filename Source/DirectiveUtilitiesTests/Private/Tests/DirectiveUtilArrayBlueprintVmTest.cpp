// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallArrayFunction.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

namespace DirectiveUtilArrayBlueprintVmTest
{
	template <typename NodeType>
	NodeType* AddNode(UEdGraph& Graph)
	{
		NodeType* Node = NewObject<NodeType>(&Graph);
		Graph.AddNode(Node);
		Node->CreateNewGuid();
		Node->PostPlacedNewNode();
		return Node;
	}

	UK2Node_VariableGet* AddVariableGet(UEdGraph& Graph, const FName PropertyName)
	{
		UK2Node_VariableGet* Node = AddNode<UK2Node_VariableGet>(Graph);
		Node->VariableReference.SetSelfMember(PropertyName);
		Node->AllocateDefaultPins();
		return Node;
	}

	UK2Node_VariableSet* AddVariableSet(UEdGraph& Graph, const FName PropertyName)
	{
		UK2Node_VariableSet* Node = AddNode<UK2Node_VariableSet>(Graph);
		Node->VariableReference.SetSelfMember(PropertyName);
		Node->AllocateDefaultPins();
		return Node;
	}

	UK2Node_CallArrayFunction* AddArrayCall(UEdGraph& Graph, const FName FunctionName)
	{
		UK2Node_CallArrayFunction* Node = AddNode<UK2Node_CallArrayFunction>(Graph);
		Node->FunctionReference.SetExternalMember(FunctionName, UDirectiveUtilArrayFunctionLibrary::StaticClass());
		Node->AllocateDefaultPins();
		return Node;
	}

	bool Connect(UEdGraph& Graph, UEdGraphNode& FromNode, const FName FromPinName,
		UEdGraphNode& ToNode, const FName ToPinName)
	{
		UEdGraphPin* FromPin = FromNode.FindPin(FromPinName);
		UEdGraphPin* ToPin = ToNode.FindPin(ToPinName);
		const UEdGraphSchema_K2* Schema = CastChecked<UEdGraphSchema_K2>(Graph.GetSchema());
		return FromPin && ToPin && Schema->TryCreateConnection(FromPin, ToPin);
	}

	bool ConnectVariable(UEdGraph& Graph, const FName PropertyName,
		UEdGraphNode& ToNode, const FName ToPinName)
	{
		UK2Node_VariableGet* VariableGet = AddVariableGet(Graph, PropertyName);
		return Connect(Graph, *VariableGet, PropertyName, ToNode, ToPinName);
	}

	UBlueprint* BuildScenarioBlueprint(FAutomationTestBase& Test, const FName TargetProperty,
		const FName SourceProperty, const FName ItemProperty)
	{
		const FName BlueprintName(*FString::Printf(
			TEXT("BP_ArrayThunkVm_%s_%s"),
			*TargetProperty.ToString(),
			*FGuid::NewGuid().ToString(EGuidFormats::Digits)));
		UPackage* Package = CreatePackage(
			*FString::Printf(TEXT("/Temp/DirectiveUtilitiesTests/%s"), *BlueprintName.ToString()));
		Package->SetFlags(RF_Transient);
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
			UDirectiveUtilTestObject::StaticClass(),
			Package,
			BlueprintName,
			BPTYPE_Normal,
			TEXT("DirectiveUtilities.ArrayBlueprintVmTests"));
		if (!Test.TestNotNull(TEXT("The array VM test Blueprint should be created"), Blueprint))
		{
			return nullptr;
		}

		UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		if (!Test.TestNotNull(TEXT("The array VM test Blueprint should have an event graph"), Graph))
		{
			return nullptr;
		}

		UK2Node_Event* Event = AddNode<UK2Node_Event>(*Graph);
		Event->EventReference.SetExternalMember(
			GET_FUNCTION_NAME_CHECKED(UDirectiveUtilTestObject, RunArrayThunkScenario),
			UDirectiveUtilTestObject::StaticClass());
		Event->bOverrideFunction = true;
		Event->AllocateDefaultPins();

		UK2Node_CallArrayFunction* Append = AddArrayCall(
			*Graph,
			GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_AppendOptimized));
		UK2Node_CallArrayFunction* Insert = AddArrayCall(
			*Graph,
			GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_InsertOptimized));
		UK2Node_VariableSet* SetInsertResult = AddVariableSet(*Graph, TEXT("TestInsertResult"));
		UK2Node_CallArrayFunction* RemoveIndices = AddArrayCall(
			*Graph,
			GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_RemoveAtIndices));
		UK2Node_VariableSet* SetRemovedCount = AddVariableSet(*Graph, TEXT("TestRemovedCount"));
		UK2Node_CallArrayFunction* RemoveAll = AddArrayCall(
			*Graph,
			GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_RemoveAllOccurrences));
		UK2Node_VariableSet* SetRemoveAllResult = AddVariableSet(*Graph, TEXT("TestRemoveAllResult"));

		bool bConnected = true;
		bConnected &= Connect(*Graph, *Event, UEdGraphSchema_K2::PN_Then, *Append, UEdGraphSchema_K2::PN_Execute);
		bConnected &= Connect(*Graph, *Append, UEdGraphSchema_K2::PN_Then, *Insert, UEdGraphSchema_K2::PN_Execute);
		bConnected &= Connect(*Graph, *Insert, UEdGraphSchema_K2::PN_Then, *SetInsertResult, UEdGraphSchema_K2::PN_Execute);
		bConnected &= Connect(*Graph, *Insert, UEdGraphSchema_K2::PN_ReturnValue, *SetInsertResult, TEXT("TestInsertResult"));
		bConnected &= Connect(*Graph, *SetInsertResult, UEdGraphSchema_K2::PN_Then, *RemoveIndices, UEdGraphSchema_K2::PN_Execute);
		bConnected &= Connect(*Graph, *RemoveIndices, UEdGraphSchema_K2::PN_Then, *SetRemovedCount, UEdGraphSchema_K2::PN_Execute);
		bConnected &= Connect(*Graph, *RemoveIndices, UEdGraphSchema_K2::PN_ReturnValue, *SetRemovedCount, TEXT("TestRemovedCount"));
		bConnected &= Connect(*Graph, *SetRemovedCount, UEdGraphSchema_K2::PN_Then, *RemoveAll, UEdGraphSchema_K2::PN_Execute);
		bConnected &= Connect(*Graph, *RemoveAll, UEdGraphSchema_K2::PN_Then, *SetRemoveAllResult, UEdGraphSchema_K2::PN_Execute);
		bConnected &= Connect(*Graph, *RemoveAll, UEdGraphSchema_K2::PN_ReturnValue, *SetRemoveAllResult, TEXT("TestRemoveAllResult"));

		for (UK2Node_CallArrayFunction* Call : {Append, Insert, RemoveIndices, RemoveAll})
		{
			bConnected &= ConnectVariable(*Graph, TargetProperty, *Call, TEXT("TargetArray"));
		}
		bConnected &= ConnectVariable(*Graph, SourceProperty, *Append, TEXT("SourceArray"));
		bConnected &= ConnectVariable(*Graph, SourceProperty, *Insert, TEXT("SourceArray"));
		bConnected &= ConnectVariable(*Graph, TEXT("TestIndices"), *RemoveIndices, TEXT("Indices"));
		bConnected &= ConnectVariable(*Graph, ItemProperty, *RemoveAll, TEXT("Item"));
		Insert->FindPinChecked(TEXT("Index"))->DefaultValue = TEXT("1");

		if (!Test.TestTrue(TEXT("The array VM test graph should connect every pin"), bConnected))
		{
			return nullptr;
		}

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);
		if (!Test.TestTrue(
			TEXT("The array VM test Blueprint should compile"),
			Blueprint->Status == BS_UpToDate || Blueprint->Status == BS_UpToDateWithWarnings))
		{
			return nullptr;
		}
		return Blueprint;
	}

	UDirectiveUtilTestObject* CreateScenarioInstance(FAutomationTestBase& Test, UBlueprint& Blueprint)
	{
		UDirectiveUtilTestObject* Instance = NewObject<UDirectiveUtilTestObject>(
			GetTransientPackage(), Blueprint.GeneratedClass);
		if (!Test.TestNotNull(TEXT("The compiled array VM test instance should be created"), Instance))
		{
			return nullptr;
		}
		Instance->TestIndices = {0, 4};
		return Instance;
	}

	bool RunScenario(
		FAutomationTestBase& Test,
		UDirectiveUtilTestObject& Instance,
		const bool bExpectedInsertResult = true,
		const int32 ExpectedRemovedCount = 2,
		const bool bExpectedRemoveAllResult = true)
	{
		UFunction* Function = Instance.FindFunction(
			GET_FUNCTION_NAME_CHECKED(UDirectiveUtilTestObject, RunArrayThunkScenario));
		if (!Test.TestNotNull(TEXT("The compiled array VM event should exist"), Function))
		{
			return false;
		}
		Instance.ProcessEvent(Function, nullptr);
		Test.TestEqual(
			TEXT("Insert Array Optimized should marshal its Blueprint result"),
			Instance.TestInsertResult,
			bExpectedInsertResult);
		Test.TestEqual(
			TEXT("Remove At Indices should marshal its Blueprint result"),
			Instance.TestRemovedCount,
			ExpectedRemovedCount);
		Test.TestEqual(
			TEXT("Remove All Occurrences should marshal its Blueprint result"),
			Instance.TestRemoveAllResult,
			bExpectedRemoveAllResult);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArrayBlueprintVmTest,
	"DirectiveUtilities.ArrayBlueprintVmTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayBlueprintVmTest::RunTest(const FString& Parameters)
{
	using namespace DirectiveUtilArrayBlueprintVmTest;

	if (UBlueprint* Blueprint = BuildScenarioBlueprint(
		*this, TEXT("TestBoolArray"), TEXT("TestBoolSourceArray"), TEXT("TestBoolItem")))
	{
		if (UDirectiveUtilTestObject* Instance = CreateScenarioInstance(*this, *Blueprint))
		{
			Instance->TestBoolArray = {true, false, true};
			Instance->TestBoolSourceArray = {false, true};
			Instance->TestBoolItem = false;
			if (RunScenario(*this, *Instance))
			{
				TestEqual(TEXT("Boolean wildcard values should survive Blueprint VM execution"),
					Instance->TestBoolArray, TArray<bool>({true, true}));
			}
		}
	}

	if (UBlueprint* Blueprint = BuildScenarioBlueprint(
		*this, TEXT("TestBoolArray"), TEXT("TestBoolArray"), TEXT("TestBoolItem")))
	{
		if (UDirectiveUtilTestObject* Instance = CreateScenarioInstance(*this, *Blueprint))
		{
			Instance->TestBoolArray = {true, false};
			Instance->TestBoolItem = true;
			Instance->TestIndices = {-1, 99};
			if (RunScenario(*this, *Instance, true, 0, true))
			{
				TestEqual(
					TEXT("Self-aliasing wildcard arrays should survive Blueprint VM execution"),
					Instance->TestBoolArray,
					TArray<bool>({false, false, false, false}));
			}
		}
	}

	if (UBlueprint* Blueprint = BuildScenarioBlueprint(
		*this, TEXT("TestBoolArray"), TEXT("TestBoolSourceArray"), TEXT("TestBoolItem")))
	{
		if (UDirectiveUtilTestObject* Instance = CreateScenarioInstance(*this, *Blueprint))
		{
			Instance->TestBoolArray = {true};
			Instance->TestBoolSourceArray.Reset();
			Instance->TestBoolItem = false;
			Instance->TestIndices = {-1, 99};
			if (RunScenario(*this, *Instance, false, 0, false))
			{
				TestEqual(
					TEXT("No-op wildcard calls should preserve the target through Blueprint VM execution"),
					Instance->TestBoolArray,
					TArray<bool>({true}));
			}
		}
	}

	if (UBlueprint* Blueprint = BuildScenarioBlueprint(
		*this, TEXT("TestStringArray"), TEXT("TestStringSourceArray"), TEXT("TestStringItem")))
	{
		if (UDirectiveUtilTestObject* Instance = CreateScenarioInstance(*this, *Blueprint))
		{
			Instance->TestStringArray = {TEXT("A"), TEXT("B"), TEXT("A")};
			Instance->TestStringSourceArray = {TEXT("C"), TEXT("A")};
			Instance->TestStringItem = TEXT("A");
			if (RunScenario(*this, *Instance))
			{
				TestEqual(TEXT("String wildcard values should survive Blueprint VM execution"),
					Instance->TestStringArray, TArray<FString>({TEXT("C"), TEXT("B"), TEXT("C")}));
			}
		}
	}

	if (UBlueprint* Blueprint = BuildScenarioBlueprint(
		*this, TEXT("TestCollisionArray"), TEXT("TestCollisionSourceArray"), TEXT("TestCollisionItem")))
	{
		if (UDirectiveUtilTestObject* Instance = CreateScenarioInstance(*this, *Blueprint))
		{
			Instance->TestCollisionArray = {{1}, {2}, {1}};
			Instance->TestCollisionSourceArray = {{3}, {1}};
			Instance->TestCollisionItem = {1};
			if (RunScenario(*this, *Instance))
			{
				TestEqual(TEXT("Struct wildcard values should survive Blueprint VM execution"),
					Instance->TestCollisionArray, TArray<FDirectiveUtilCollisionValue>({{3}, {2}, {3}}));
			}
		}
	}

	if (UBlueprint* Blueprint = BuildScenarioBlueprint(
		*this, TEXT("TestObjectArray"), TEXT("TestObjectSourceArray"), TEXT("TestObjectItem")))
	{
		if (UDirectiveUtilTestObject* Instance = CreateScenarioInstance(*this, *Blueprint))
		{
			UObject* A = NewObject<UDirectiveUtilTestObject>(Instance);
			UObject* B = NewObject<UDirectiveUtilTestObject>(Instance);
			UObject* C = NewObject<UDirectiveUtilTestObject>(Instance);
			Instance->TestObjectArray = {A, B, A};
			Instance->TestObjectSourceArray = {C, A};
			Instance->TestObjectItem = A;
			if (RunScenario(*this, *Instance))
			{
				TestEqual(TEXT("Object wildcard arrays should retain three values"), Instance->TestObjectArray.Num(), 3);
				if (Instance->TestObjectArray.Num() == 3)
				{
					TestEqual(TEXT("Object wildcard arrays should retain the first source object"), Instance->TestObjectArray[0].Get(), C);
					TestEqual(TEXT("Object wildcard arrays should retain the target object"), Instance->TestObjectArray[1].Get(), B);
					TestEqual(TEXT("Object wildcard arrays should retain the second source object"), Instance->TestObjectArray[2].Get(), C);
				}
			}
		}
	}

	return true;
}
