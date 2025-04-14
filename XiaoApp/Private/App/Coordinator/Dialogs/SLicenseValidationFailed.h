// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

class SLicenseValidationFailed final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SLicenseValidationFailed)
	{}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
};
