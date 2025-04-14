/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "SBaseView.h"

class SCertificateView : public SBaseView
{
public:
	SLATE_BEGIN_ARGS(SCertificateView){}
		SLATE_EVENT(FOnQueueNotification, OnQueueNotification)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	// ~ SWidget Override Begin 
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// ~ SWidget Override Finish

protected:
	FText GetLicenseType() const;
	FText GetLicenseText() const;

	FText GetLastUpdate() const;
	FText GetExpirationDate() const;

	FText GetInitiatorsNum() const;
	FText GetHelperCoreNum() const;

private:
	uint16 InitiatorNum = 10;
	uint16 HelperCoreNum = 40;
};
