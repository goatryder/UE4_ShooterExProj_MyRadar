// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

/**
 *
 */
class SRadarWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRadarWidget) {}

	SLATE_ARGUMENT(TWeakObjectPtr<class AShooterHUD>, OwningHUD)

	SLATE_END_ARGS()

	/** every widget needs a construction function */
	void Construct(const FArguments& InArgs);

	FReply OnQuitClicked() const;

	/** The HUD that created this widget */
	TWeakObjectPtr<class AShooterHUD> OwningHUD;
};
