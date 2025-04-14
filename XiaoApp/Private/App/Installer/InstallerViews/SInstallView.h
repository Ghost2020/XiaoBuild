/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"
#include "Widgets/Views/STileView.h"

struct FInstallDesc
{
	int Type;
	FText Name;
	const FSlateBrush* Thumbnail;

	explicit FInstallDesc(const int InType, const FText InName, const FSlateBrush* InThumbnail)
		: Type(InType)
		, Name(InName)
		, Thumbnail(InThumbnail)
	{}
};

class SInstallView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SInstallView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
protected:
	void ConstructWidget();

	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;
	virtual bool OnCanBack() override;

private:
	TArray<TSharedPtr<FInstallDesc>> InstallArray;
	TSharedPtr<STileView<TSharedPtr<FInstallDesc>>> TitleView = nullptr;
	
	TSharedPtr<class SWidgetSwitcher> Switcher = nullptr;
	TSharedPtr<class SGridInstallView> GridView = nullptr;
	TSharedPtr<class SAgentInstallView> AgentView = nullptr;
	TSharedPtr<class SCustomInstallView> CustomView = nullptr;
};
