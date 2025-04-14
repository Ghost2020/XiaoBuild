#include "DependencyGraph.h"
#ifdef USE_IMGUI
#include "imnodes.h"
#include "HAL/PlatformApplicationMisc.h"

namespace XiaoGraph
{
	void FBaseElement::OnDraw()
	{
		OnBeginDraw();
		OnFinishDraw();
	}

	void FBaseElement::OnBeginDraw()
	{
		check(0);
	}

	void FBaseElement::OnFinishDraw()
	{
		check(0);
	}

	void FBaseNode::OnDraw()
	{
		OnBeginDraw();

		// Title
		ImNodes::BeginNodeTitleBar();
		const std::string TestTitle = TCHAR_TO_UTF8(*FString::Printf(TEXT("%d-%s"), ID, UTF8_TO_TCHAR(Title.c_str())));
		ImGui::TextUnformatted(TestTitle.c_str());
		ImNodes::EndNodeTitleBar();

		// 输入
		if (FromIds.Num() > 0)
		{
			ImNodes::BeginInputAttribute(InputPinId);
			ImNodes::EndInputAttribute();
		}

		// 类型
		/*switch (Type)
		{
		default:

		}*/

		// 状态
		/*switch (Status)
		{
		default:

		}*/

		// 输出
		if (ToIds.Num() > 0)
		{
			ImNodes::BeginOutputAttribute(OutputPinId);
			ImNodes::EndOutputAttribute();
		}
		
		OnFinishDraw();
	}

	void FBaseNode::OnBeginDraw()
	{
		ImNodes::BeginNode(ID);
	}

	void FBaseNode::OnFinishDraw()
	{
		ImNodes::EndNode();
	}

	void FBaseNode::AddFrom(const int32 InFromId)
	{
		FromIds.Add(InFromId);
	}

	void FBaseNode::AddTo(const int32 InToId)
	{
		ToIds.Add(InToId);
	}

	void FBaseNode::SetLocation(const FVector2D& InLocation) const
	{
		ImNodes::SetNodeScreenSpacePos(ID, ImVec2(InLocation.X-100.0f, InLocation.Y-150.0f));
	}

	FVector2D FBaseNode::GetLocation() const
	{
		const ImVec2 Pos = ImNodes::GetNodeEditorSpacePos(ID);
		return FVector2D(Pos.x, Pos.y);
	}

	void FBaseNode::Update()
	{
		if(Depth != 0)
		{
			return;
		}
		
		Depth = 1;
		for (const auto& FromId : FromIds)
		{
			auto FromNode = Graph->GetNode(FromId);
			check(FromNode.IsValid());
			if(FromNode->Depth == 0)
			{
				FromNode->Update();
			}
			if(FromNode->Depth >= Depth)
			{
				Depth = FromNode->Depth + 1;
			}
		}
	}

	FVector2D FBaseNode::GetNodeSize() const
	{
		if (ImNodes::NodeExist(ID))
		{
			const auto Size = ImNodes::GetNodeDimensions(ID);
			return FVector2D(Size.x, Size.y);
		}
		return FVector2D::ZeroVector;
	}

	void FBaseEdge::OnDraw()
	{
		ImNodes::Link(ID, From, To);
	}

	FGraph::FGraph()
		: FBaseElement(0)
	{
		GraphName = "DependencyGraph";

		Context = ImNodes::CreateContext();
		auto& Style = ImNodes::GetStyle();
		Style.Flags = ImNodesStyleFlags_None;
		Style.LinkThickness = 1.0f;
	}

	FGraph::~FGraph()
	{
		if (Context)
		{
			ImNodes::DestroyContext(Context);
		}
	}

	void FGraph::RebuildGraph()
	{
		if (!bDirty)
		{
			return;
		}

		Edges.Empty();
		for (const auto& Node : Nodes)
		{
			int StartedNodeId = Node.Key;
			int StartedOutputPinId = Node.Value->OutputPinId;

			// 生成Edge
			for (const auto ToId : Node.Value->ToIds)
			{
				if (!Nodes.Contains(ToId))
				{
					// TODO 是否需要提示
					continue;
				}
				const auto TargetNode = Nodes[ToId];
				int EndedNodeId = TargetNode->ID;
				int EndedInputPinId = TargetNode->InputPinId;
				bool bCreate = false;
				if (!ImNodes::IsLinkCreated(&StartedNodeId, &StartedOutputPinId, &EndedNodeId, &EndedInputPinId, &bCreate))
				{
					AddEdge(NewEdgeId(), StartedOutputPinId, EndedInputPinId);
				}
			}

			// 计算深度
			Node.Value->Update();
		}

		// 先进行排序
		Nodes.ValueStableSort([](const TSharedPtr<FBaseNode> L, const TSharedPtr<FBaseNode> R) 
		{
			return (L->Depth == R->Depth) ? (L->ToIds.Num() == R->ToIds.Num() ? L->ID < R->ID : L->ToIds.Num() > R->ToIds.Num()) : (L->Depth < R->Depth);
		});

		// 设置位置 Y轴直接按当前层次进行排序
		int Index = 0;
		TSharedPtr<FBaseNode> LastNode = nullptr;
		for(const auto Iter : Nodes)
		{
			const auto CurNode = Iter.Value;

			float X = 200.f;
			float Y = 200.f;
			if(!CurNode->FromIds.IsEmpty())
			{
				// 确定X轴
				const auto LastDepthNode = Nodes[CurNode->FromIds[0]];
				if (LastDepthNode->GetNodeSize().X <=0.1f)
				{
					return;
				}
				X += LastDepthNode->GetLocation().X + LastDepthNode->GetNodeSize().X + 250.0f;
			}

			// 确定Y轴
			if (LastNode.IsValid())
			{
				if (LastNode->Depth == CurNode->Depth)
				{
					Y = LastNode->GetLocation().Y + 200.0f;
				}
			}

			CurNode->SetLocation(FVector2D(X, Y));
			Index++;
			LastNode = CurNode;
		}

		bDirty = false;
	}

	void FGraph::Reset()
	{
		Nodes.Empty();
		Edges.Empty();
		bDirty = true;
	}

	int32 FGraph::NewEdgeId() const
	{
		int NewId = -1;
		TArray<int32> Ids, EdgeIds;
		Edges.GetKeys(EdgeIds);
		Ids.Append(EdgeIds);
		if (!Ids.Contains(NewId))
		{
			return NewId;	
		}
		for (const auto Id : Ids)
		{
			if (Id != NewId)
			{
				return NewId;
			}
			--NewId;
		}
		return NewId;
	}

	void FGraph::OnDraw()
	{
		OnBeginDraw();

		for(const auto& Node : Nodes)
		{
			Node.Value->OnDraw();
		}

		for(const auto& Edge : Edges)
		{
			Edge.Value->OnDraw();
		}
		
		OnFinishDraw();
	}

	void FGraph::OnBeginDraw()
	{
		ImGui::Begin(GraphName.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetWindowSize(ImVec2(CanvasSize.X, CanvasSize.Y));
		ImNodes::BeginNodeEditor();
	}

	void FGraph::OnFinishDraw()
	{
		ImNodes::MiniMap(0.15f, ImNodesMiniMapLocation_TopLeft);
		ImNodes::EndNodeEditor();
		ImGui::End();
	}

	TSharedPtr<FBaseNode> FGraph::AddNode(const int32 InNodeId, const FString& InNodeTitle, const TArray<int32>& InFromIds, const TArray<int32>& InToIds)
	{
		check(!Nodes.Contains(InNodeId));
		TSharedPtr<FBaseNode> NewNode = MakeShared<FBaseNode>(this, InNodeId, InNodeTitle, InFromIds, InToIds);
		Nodes.Add(InNodeId, NewNode);
		return NewNode;
	}

	TSharedPtr<FBaseNode> FGraph::GetNode(const int32 InNodeId)
	{
		if(Nodes.Contains(InNodeId))
		{
			return Nodes[InNodeId];
		}
		return nullptr;
	}

	TSharedPtr<FBaseEdge> FGraph::AddEdge(const int32 InEdgeId, const int32 InFromId, const int32 InToId)
	{
		check(!Edges.Contains(InEdgeId));
		TSharedPtr<FBaseEdge> NewEdge = MakeShared<FBaseEdge>(InEdgeId, InFromId, InToId);
		Edges.Add(InEdgeId, NewEdge);
		return NewEdge;
	}

	TSharedPtr<FBaseEdge> FGraph::GetEdge(const int32 InEdgeId)
	{
		if(Edges.Contains(InEdgeId))
		{
			return Edges[InEdgeId];
		}
		return nullptr;
	}
}
#endif