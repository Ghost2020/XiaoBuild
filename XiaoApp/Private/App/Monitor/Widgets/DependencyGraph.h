#pragma once

#include "CoreMinimal.h"
#include <string>

#ifdef USE_IMGUI
struct ImNodesContext;

namespace XiaoGraph
{
	class FBaseElement
	{
		friend class FGraph;
	public:
		explicit FBaseElement(const int32 InId)
			: ID(InId)
		{}
		
		virtual ~FBaseElement() = default;

	protected:
		virtual void OnDraw();
		virtual void OnBeginDraw();
		virtual void OnFinishDraw();
		
		int32 ID = 0;
	};

	class FBaseNode : public FBaseElement
	{
		friend class FGraph;
	public:
		enum ENodeType
		{
			Type_Unknown = 0,
			Type_Dll,
			Type_Exe
		};

		enum ENodeStatus
		{
			Status_Unknown = 0,
			Status_Sleep,
			Status_Ready,
			Status_Working,
			Status_Success,
			Status_Failed
		};

		explicit FBaseNode(FGraph* InGraph, const int32 InId, const FString& InTitle,
		                   const TArray<int32>& InFromIds = {}, const TArray<int32>& InToIds = {},
		                   const ENodeType InType = Type_Unknown
		)
			: FBaseElement(InId)
			, Graph(InGraph)
			, Type(InType)
			, Title(TCHAR_TO_UTF8(*InTitle))
			, FromIds(InFromIds)
			, ToIds(InToIds)
		{
			check(InId >= 0 && InId <= 8192);
			InputPinId = (InId) * 10000;
			OutputPinId = InputPinId + 1;
		}

		virtual void OnDraw() override;
		virtual void OnBeginDraw() override;
		virtual void OnFinishDraw() override;

		virtual void AddFrom(const int32 InFromId);
		virtual void AddTo(const int32 InToId);

		void SetLocation(const FVector2D& InLocation) const;
		FVector2D GetLocation() const;
		FVector2D GetNodeSize() const;

	protected:
		virtual void Update();

	protected:
		FGraph* Graph = nullptr;
		ENodeType Type = Type_Unknown;
		ENodeStatus Status = Status_Sleep;
		std::string Title;
		int32 InputPinId;
		int32 OutputPinId;
		TArray<int32> FromIds;
		TArray<int32> ToIds;
		int32 Depth = 0;
	};

	class FBaseEdge : public FBaseElement
	{
		friend class FGraph;
	public:
		explicit FBaseEdge(const int32 InId, const int32 InFrom, const int32 InTo)
			: FBaseElement(InId)
			, From(InFrom)
			, To(InTo)
		{}
		
		virtual void OnDraw() override;

	protected:
		int32 From = 0;
		int32 To = 0;
	};

	class FGraph final : public FBaseElement
	{
	public:
		explicit FGraph();

		virtual ~FGraph() override;

		virtual void OnDraw() override;

		TSharedPtr<FBaseNode> AddNode(const int32 InNodeId, const FString& InNodeTitle,
			const TArray<int32>& InFromIds = {}, const TArray<int32>& InToIds = {});
		TSharedPtr<FBaseNode> GetNode(const int32 InNodeId);
		TSharedPtr<FBaseEdge> AddEdge(const int32 InEdgeId, const int32 InFromId, const int32 InToId);
		TSharedPtr<FBaseEdge> GetEdge(const int32 InEdgeId);

		void RebuildGraph();
		void Reset();
		
		void SetCanvasSize(const FVector2D InCanvasSize) { CanvasSize = InCanvasSize; };
		void SetDirtyFlag() { bDirty = true;  };
		
	private:
		int32 NewEdgeId() const;

	protected:
		virtual void OnBeginDraw() override;
		virtual void OnFinishDraw() override;
	
	private:
		ImNodesContext* Context = nullptr;
		std::string GraphName = "Graph";
		FVector2D CanvasSize = FVector2D::ZeroVector;

		TMap<int32, TSharedPtr<FBaseNode>> Nodes;
		TMap<int32, TSharedPtr<FBaseEdge>> Edges;

		bool bDirty = true;
	};
}
#endif