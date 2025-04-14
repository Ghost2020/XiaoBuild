/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class FJsonObject;

template<typename ItemType>
class STreeView;

template<typename ItemType>
class SListView;

struct FRightItemDesc;


struct FLeftItemDesc
{
	explicit FLeftItemDesc(TArray<TSharedPtr<FRightItemDesc>>& ContentArray, const TSharedPtr<FJsonObject>& InItemObject, const uint32 InLevel);
	
	void ConstructItem(TArray<TSharedPtr<FRightItemDesc>>& ContentArray, const TSharedPtr<FJsonObject>& InItemObject);
	
	uint32 Level = 0;
	FText Text;
	TArray<TSharedPtr<FLeftItemDesc>> Children;

	TWeakPtr<FRightItemDesc> Content = nullptr;
};


struct FRightItemDesc
{
	explicit FRightItemDesc(const TSharedPtr<FJsonObject>& InContentObject);

	TWeakPtr<FJsonObject> ContentObject = nullptr;
};


class SDocumentWindow final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SDocumentWindow){}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
protected:
	void ConstructView();
	TSharedRef<class ITableRow> OnGenerateRow_LeftTree(TSharedPtr<FLeftItemDesc> InItem, const TSharedRef< class STableViewBase >& InTable) const;
	static void OnGetChildren_LeftTree(TSharedPtr<FLeftItemDesc> InParent, TArray<TSharedPtr<FLeftItemDesc>>& OutChildren);
	void OnDoubleClicked(const TSharedPtr<FLeftItemDesc> InItemNode) const;
	
private:
    void LoadJson();	
	
private:
	TSharedPtr<FJsonObject> RootObject = nullptr;
	
	TArray<TSharedPtr<FLeftItemDesc>> TreeArray;
	TSharedPtr<STreeView<TSharedPtr<FLeftItemDesc>>> TreeView = nullptr;

	TArray<TSharedPtr<FRightItemDesc>> ListArray;
	TSharedPtr<SListView<TSharedPtr<FRightItemDesc>>> ListView = nullptr;
};