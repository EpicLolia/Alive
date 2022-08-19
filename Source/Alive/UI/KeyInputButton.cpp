// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/KeyInputButton.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "Components/ButtonSlot.h"


//#define LOCTEXT_NAMESPACE "UMG"

static FButtonStyle* DefaultButtonStyle = nullptr;

UKeyInputButton::UKeyInputButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (DefaultButtonStyle == nullptr)
	{
		// HACK: THIS SHOULD NOT COME FROM CORESTYLE AND SHOULD INSTEAD BE DEFINED BY ENGINE TEXTURES/PROJECT SETTINGS
		DefaultButtonStyle = new FButtonStyle(FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"));

		// Unlink UMG default colors from the editor settings colors.
		DefaultButtonStyle->UnlinkColors();
	}

	WidgetStyle = *DefaultButtonStyle;

	ColorAndOpacity = FLinearColor::White;
	BackgroundColor = FLinearColor::White;

	KeyName = FName("None");
	ControllerId = 0;
	IsFocusable = true;

#if WITH_EDITORONLY_DATA
	AccessibleBehavior = ESlateAccessibleBehavior::Summary;
	bCanChildrenBeAccessible = false;
#endif
}

void UKeyInputButton::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyButton.Reset();
}

TSharedRef<SWidget> UKeyInputButton::RebuildWidget()
{
	MyButton = SNew(SButton)
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
		.ButtonStyle(&WidgetStyle)
		.IsFocusable(IsFocusable)
		;

	if ( GetChildrenCount() > 0 )
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	}
	
	return MyButton.ToSharedRef();
}

void UKeyInputButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MyButton->SetColorAndOpacity( ColorAndOpacity );
	MyButton->SetBorderBackgroundColor( BackgroundColor );
}

UClass* UKeyInputButton::GetSlotClass() const
{
	return UButtonSlot::StaticClass();
}

void UKeyInputButton::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live slot if it already exists
	if ( MyButton.IsValid() )
	{
		CastChecked<UButtonSlot>(InSlot)->BuildSlot(MyButton.ToSharedRef());
	}
}

void UKeyInputButton::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if ( MyButton.IsValid() )
	{
		MyButton->SetContent(SNullWidget::NullWidget);
	}
}

void UKeyInputButton::SetStyle(const FButtonStyle& InStyle)
{
	WidgetStyle = InStyle;
	if ( MyButton.IsValid() )
	{
		MyButton->SetButtonStyle(&WidgetStyle);
	}
}

void UKeyInputButton::SetColorAndOpacity(FLinearColor InColorAndOpacity)
{
	ColorAndOpacity = InColorAndOpacity;
	if ( MyButton.IsValid() )
	{
		MyButton->SetColorAndOpacity(InColorAndOpacity);
	}
}

void UKeyInputButton::SetBackgroundColor(FLinearColor InBackgroundColor)
{
	BackgroundColor = InBackgroundColor;
	if ( MyButton.IsValid() )
	{
		MyButton->SetBorderBackgroundColor(InBackgroundColor);
	}
}

bool UKeyInputButton::IsPressed() const
{
	if ( MyButton.IsValid() )
	{
		return MyButton->IsPressed();
	}

	return false;
}

void UKeyInputButton::PostLoad()
{
	Super::PostLoad();

	if ( GetChildrenCount() > 0 )
	{
		//TODO UMG Pre-Release Upgrade, now buttons have slots of their own.  Convert existing slot to new slot.
		if ( UPanelSlot* PanelSlot = GetContentSlot() )
		{
			UButtonSlot* ButtonSlot = Cast<UButtonSlot>(PanelSlot);
			if ( ButtonSlot == NULL )
			{
				ButtonSlot = NewObject<UButtonSlot>(this);
				ButtonSlot->Content = GetContentSlot()->Content;
				ButtonSlot->Content->Slot = ButtonSlot;
				Slots[0] = ButtonSlot;
			}
		}
	}

	if( GetLinkerUE4Version() < VER_UE4_DEPRECATE_UMG_STYLE_ASSETS && Style_DEPRECATED != nullptr )
	{
		const FButtonStyle* StylePtr = Style_DEPRECATED->GetStyle<FButtonStyle>();
		if(StylePtr != nullptr)
		{
			WidgetStyle = *StylePtr;
		}

		Style_DEPRECATED = nullptr;
	}
}

void UKeyInputButton::SlateHandlePressed()
{
	if(GetOwningPlayer())
	{
		FKey key("Jump");
		GetOwningPlayer()->InputKey(key,EInputEvent::IE_Pressed,0,false);
	}
	/*
	UPlayerInput
	APlayerController::InputKey()
	uint8 JavaScriptKeyCode;
	bool IsRepeat;
	//Event.GetKeyDown(JavaScriptKeyCode, IsRepeat);
	const FKey* AgnosticKey = AgnosticKeys[JavaScriptKeyCode];
	const uint32* KeyCode;
	const uint32* CharacterCode;
	FInputKeyManager::Get().GetCodesFromKey(*AgnosticKey, KeyCode, CharacterCode);
	MessageHandler->OnKeyDown(KeyCode ? *KeyCode : 0, CharacterCode ? *CharacterCode : 0, IsRepeat);
	UE_LOG(LogTemp, Verbose, TEXT("KEY_DOWN: KeyCode = %d; CharacterCode = %d; IsRepeat = %s"), KeyCode, CharacterCode, IsRepeat ? TEXT("True") : TEXT("False"));
	*/
}

void UKeyInputButton::SlateHandleReleased()
{
	
}

#if WITH_ACCESSIBILITY
TSharedPtr<SWidget> UKeyInputButton::GetAccessibleWidget() const
{
	return MyButton;
}
#endif
/*
#if WITH_EDITOR

const FText UKeyInputButton::GetPaletteCategory()
{
	//return LOCTEXT("Common", "Common");
}

#endif
*/
/////////////////////////////////////////////////////

//#undef LOCTEXT_NAMESPACE
