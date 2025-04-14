/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

#ifdef USE_IMGUI
class SExpandableArea;
class FJsonObject;
class FJsonValue;

class SBuildDependencyView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBuildDependencyView){}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	bool LoadFromJsonObject(const TArray<TSharedPtr<FJsonValue>>*& InActionArray) const;
	void UpdateDependency(const FJsonObject* InObject);

private:
	TSharedPtr<class SViewport> Viewport = nullptr;
	TSharedPtr<class SDependencyWidget> DependencyWidget = nullptr;
};
#endif
