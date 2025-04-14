/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include <map>
#include "system_settings.pb.h"

class SBaseView : public SCompoundWidget
{
public:
	struct FUserIp
	{
		FString LoginUser;
		FString IP;
	};

	DECLARE_DELEGATE_TwoParams(FOnQueueNotification, int8, const FText&);
	
	SLATE_BEGIN_ARGS(SBaseView){}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual void OnUpdate(const bool bInRebuild) const = 0;

protected:
	FOnQueueNotification OnQueueNotification;

	static TMap<FString, FUserIp> GID2User;
};
