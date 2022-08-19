// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/ContentWidget.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SWidget.h"

#include "KeyInputButton.generated.h"


class SButton;
class USlateWidgetStyleAsset;

/**
 * This Button is used to bind key input in the mobile
 */
UCLASS()
class ALIVE_API UKeyInputButton: public UContentWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** The template style asset, used to seed the mutable instance of the style. */
	UPROPERTY()
	USlateWidgetStyleAsset* Style_DEPRECATED;

	/** The button style used at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=( DisplayName="Style" ))
	FButtonStyle WidgetStyle;
	
	/** The color multiplier for the button content */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance", meta=( sRGB="true" ))
	FLinearColor ColorAndOpacity;
	
	/** The color multiplier for the button background */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance", meta=( sRGB="true" ))
	FLinearColor BackgroundColor;

	/** Sometimes a button should only be mouse-clickable and never keyboard focusable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool IsFocusable;

	/** The input key name you want to bind */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="KeyInput")
	FName KeyName;

	/** The local controller who binds this button. Useful when the game has more than one local player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="KeyInput")
	int32 ControllerId;

public:
	/** Sets the color multiplier for the button background */
	UFUNCTION(BlueprintCallable, Category="Button|Appearance")
	void SetStyle(const FButtonStyle& InStyle);

	/** Sets the color multiplier for the button content */
	UFUNCTION(BlueprintCallable, Category="Button|Appearance")
	void SetColorAndOpacity(FLinearColor InColorAndOpacity);

	/** Sets the color multiplier for the button background */
	UFUNCTION(BlueprintCallable, Category="Button|Appearance")
	void SetBackgroundColor(FLinearColor InBackgroundColor);

	/**
	 * Returns true if the user is actively pressing the button.  Do not use this for detecting 'Clicks', use the OnClicked event instead.
	 *
	 * @return true if the user is actively pressing the button otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category="Button")
	bool IsPressed() const;

public:

	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	//~ End UWidget Interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface
/*
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
*/
protected:

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

protected:
	/** Handle the actual click event from slate and forward it on */
	void SlateHandlePressed();
	void SlateHandleReleased();

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
#if WITH_EDITOR
	virtual TSharedRef<SWidget> RebuildDesignWidget(TSharedRef<SWidget> Content) override { return Content; }
#endif
	//~ End UWidget Interface

#if WITH_ACCESSIBILITY
	virtual TSharedPtr<SWidget> GetAccessibleWidget() const override;
#endif

protected:
	/** Cached pointer to the underlying slate button owned by this UWidget */
	TSharedPtr<SButton> MyButton;
};
