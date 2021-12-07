﻿#include "FICDetails.h"

#include "FICEditorContext.h"
#include "FICNumericType.h"
#include "FICVectorEditor.h"
#include "FICKeyframeControl.h"
#include "FicsItCam/FICUtils.h"
#include "Widgets/Input/SNumericEntryBox.h"

FSlateColorBrush SFICDetails::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));

TSharedRef<SWidget> ScalarAttribute(UFICEditorContext* Context, TFICEditorAttribute<FFICFloatAttribute>& Attr, const FString& Label) {
	return
	SNew(SHorizontalBox)
	+SHorizontalBox::Slot().Padding(5).AutoWidth()[
		SNew(STextBlock)
		.Text(FText::FromString(Label))
	]
	+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
		SNew(SNumericEntryBox<float>)
		.Value_Lambda([&Attr]() -> TOptional<float> {
			return Attr.GetValue();
		})
		.SupportDynamicSliderMaxValue(true)
		.SupportDynamicSliderMinValue(true)
		.SliderExponent(1)
		.Delta(1)
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.LinearDeltaSensitivity(10)
		.AllowSpin(true)
		.OnValueChanged_Lambda([&Attr](float Val) {
			Attr.SetValue(Val);
		})
		.OnValueCommitted_Lambda([&Attr](float Val, auto) {
			Attr.SetValue(Val);
		})
		.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
	]
	+SHorizontalBox::Slot().Padding(5).AutoWidth()[
		SNew(SFICKeyframeControl)
		.Attribute_Lambda([&Attr]() -> FFICEditorAttributeBase* {
			return &Attr;
		})
		.Frame_Lambda([Context]() {
			return Context->GetCurrentFrame();
		})
	];
}

void SFICDetails::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;
	BackgroundBrush = InArgs._Background;

	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(BackgroundBrush)
		]
		+SOverlay::Slot()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(STextBlock)
				.Text(FText::FromString("Details:"))
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("All Keyframe:")))
					.ToolTipText(FText::FromString(FString::Printf(TEXT("Allows you to manipulate the keyframes of all attributes at the current frame.\n\nToggle All Keyframes: %s\nGo-To Next Keyframe: %s\nGo-To Previous Keyframe: %s"), *UFICUtils::KeymappingToString("FicsItCam.ToggleAllKeyframes"), *UFICUtils::KeymappingToString("FicsItCam.NextKeyframe"), *UFICUtils::KeymappingToString("FicsItCam.PrevKeyframe"))))
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SFICKeyframeControl)
					.Attribute_Lambda([this]() -> FFICEditorAttributeBase* {
						return &Context->All;
					})
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Position"))
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SFICKeyframeControl)
					.Attribute_Lambda([this]() -> FFICEditorAttributeBase* {
						return &Context->Pos;
					})
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString(":"))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SFICVectorEditor)
					.ShowKeyframeControls(true)
					.XAttr_Lambda([this]() {
						return &Context->PosX;
					})
					.YAttr_Lambda([this]() {
						return &Context->PosY;
					})
					.ZAttr_Lambda([this]() {
						return &Context->PosZ;
					})
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
					.AutoKeyframe_Lambda([this]() {
						return Context->bAutoKeyframe;
					})
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Rotation"))
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SFICKeyframeControl)
					.Attribute_Lambda([this]() -> FFICEditorAttributeBase* {
						return &Context->Rot;
					})
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString(":"))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SFICVectorEditor)
					.ShowKeyframeControls(true)
					.XAttr_Lambda([this]() {
						return &Context->RotPitch;
					})
					.YAttr_Lambda([this]() {
						return &Context->RotYaw;
					})
					.ZAttr_Lambda([this]() {
						return &Context->RotRoll;
					})
					.AutoKeyframe_Lambda([this]() {
						return Context->bAutoKeyframe;
					})
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("FOV:"))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SNumericEntryBox<float>)
					.Value_Lambda([this]() {
						return Context->FOV.GetValue();
					})
					.SupportDynamicSliderMaxValue(true)
					.SupportDynamicSliderMinValue(true)
					.SliderExponent(1)
					.Delta(1)
					.MinValue(TOptional<float>())
					.MaxValue(TOptional<float>())
					.MinSliderValue(TOptional<float>())
					.MaxSliderValue(TOptional<float>())
					.LinearDeltaSensitivity(10)
					.AllowSpin(true)
					.OnValueChanged_Lambda([this](float Val) {
						Context->FOV.SetValue(Val);
					})
					.OnValueCommitted_Lambda([this](float Val, auto) {
						Context->FOV.SetValue(Val);
					})
					.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SFICKeyframeControl)
					.Attribute_Lambda([this]() {
						return &Context->FOV;
					})
					.Frame_Lambda([this]() {
						return Context->GetCurrentFrame();
					})
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				ScalarAttribute(Context, Context->Aperture, TEXT("Aperture:"))
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				ScalarAttribute(Context, Context->FocusDistance, TEXT("Focus Distance:"))
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Settings: "))
				]
				+SHorizontalBox::Slot().Padding(5).FillWidth(1)[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Use Cinematic Camera"))]
						.IsChecked_Lambda([this]() {
							return Context->GetAnimation()->bUseCinematic ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->GetAnimation()->bUseCinematic = State == ECheckBoxState::Checked;
						})
						.ToolTipText(FText::FromString(TEXT("If enabled, tries to use a more fancy camera which f.e. can do Depth-Of-Field,\ntho it will require more performance hence using it in combination with the play command is not reccomended.")))
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Bullet Time"))]
						.IsChecked_Lambda([this]() {
							return Context->GetAnimation()->bBulletTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->GetAnimation()->bBulletTime = State == ECheckBoxState::Checked;
						})
						.ToolTipText(FText::FromString(TEXT("If enabled, game simulation will pause allowing you to have a bullet time effect.")))
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight().HAlign(HAlign_Fill)[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock).Text(FText::FromString("FPS: "))
						]
						+SHorizontalBox::Slot().FillWidth(1)[
							SNew(SNumericEntryBox<int>)
							.Value_Lambda([this]() {
								return Context->GetAnimation()->FPS;
							})
							.SupportDynamicSliderMaxValue(true)
							.SliderExponent(1)
							.Delta(1)
							.MinValue(1)
							.LinearDeltaSensitivity(10)
							.AllowSpin(false)
							.OnValueCommitted_Lambda([this](int Val, auto) {
								Context->GetAnimation()->FPS = FMath::Max(1, Val);
							})
							.TypeInterface(MakeShared<TDefaultNumericTypeInterface<int>>())
						]
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight().HAlign(HAlign_Fill)[
						SNew(SHorizontalBox)
						.ToolTipText(FText::FromString(TEXT("The resolution setting will be used to determine the aspect ratio and image size for rendering the animation.")))
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock).Text(FText::FromString("Resolution: "))
						]
						+SHorizontalBox::Slot().FillWidth(1)[
							SNew(SNumericEntryBox<int>)
							.Value_Lambda([this]() {
								return Context->GetAnimation()->ResolutionWidth;
							})
							.SupportDynamicSliderMaxValue(true)
							.SliderExponent(1)
							.Delta(1)
							.MinValue(1)
							.LinearDeltaSensitivity(10)
							.AllowSpin(false)
							.OnValueCommitted_Lambda([this](int Val, auto) {
								Context->GetAnimation()->ResolutionWidth = FMath::Max(1, Val);
							})
							.TypeInterface(MakeShared<TDefaultNumericTypeInterface<int>>())
							.ToolTipText(FText::FromString(TEXT("Resolution Width")))
						]
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock).Text(FText::FromString(" x "))
						]
						+SHorizontalBox::Slot().FillWidth(1)[
							SNew(SNumericEntryBox<int>)
							.Value_Lambda([this]() {
								return Context->GetAnimation()->ResolutionHeight;
							})
							.SupportDynamicSliderMaxValue(true)
							.SliderExponent(1)
							.Delta(1)
							.MinValue(1)
							.LinearDeltaSensitivity(10)
							.AllowSpin(false)
							.OnValueCommitted_Lambda([this](int Val, auto) {
								Context->GetAnimation()->ResolutionHeight = FMath::Max(1, Val);
							})
							.TypeInterface(MakeShared<TDefaultNumericTypeInterface<int>>())
							.ToolTipText(FText::FromString(TEXT("Resolution Height")))
						]
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight().HAlign(HAlign_Fill)[
						SNew(SHorizontalBox)
						.ToolTipText(FText::FromString(TEXT("The sensor size used to adjust the DOF and aspect ration. (only functional with cinematic camera)")))
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock).Text(FText::FromString("Sensor Size: "))
						]
						+SHorizontalBox::Slot().FillWidth(1)[
							SNew(SNumericEntryBox<float>)
							.Value_Lambda([this]() {
								return Context->GetAnimation()->SensorWidth;
							})
							.SupportDynamicSliderMaxValue(true)
							.SliderExponent(1)
							.Delta(0.1)
							.MinValue(0)
							.LinearDeltaSensitivity(10)
							.AllowSpin(false)
							.OnValueCommitted_Lambda([this](float Val, auto) {
								Context->GetAnimation()->SensorWidth = FMath::Max(0.0f, Val);
							})
							.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
							.ToolTipText(FText::FromString(TEXT("Sensor Width")))
						]
						+SHorizontalBox::Slot().AutoWidth()[
							SNew(STextBlock).Text(FText::FromString(" x "))
						]
						+SHorizontalBox::Slot().FillWidth(1)[
							SNew(SNumericEntryBox<float>)
							.Value_Lambda([this]() {
								return Context->GetAnimation()->SensorHeight;
							})
							.SupportDynamicSliderMaxValue(true)
							.SliderExponent(1)
							.Delta(0.1)
							.MinValue(0)
							.LinearDeltaSensitivity(10)
							.AllowSpin(false)
							.OnValueCommitted_Lambda([this](float Val, auto) {
								Context->GetAnimation()->SensorHeight = FMath::Max(0.0f, Val);
							})
							.TypeInterface(MakeShared<TDefaultNumericTypeInterface<float>>())
							.ToolTipText(FText::FromString(TEXT("Sensor Height")))
						]
					]
				]
			]
			+SVerticalBox::Slot().Padding(5).AutoHeight()[
			SNew(SHorizontalBox)
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(STextBlock)
					.Text(FText::FromString("Editor:"))
				]
				+SHorizontalBox::Slot().Padding(5).AutoWidth()[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Lock Viewport Camera"))]
						.IsChecked_Lambda([this]() {
							return Context->bMoveCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bMoveCamera = State == ECheckBoxState::Checked;
						})
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, the viewport camera will be locked to the virtual camera for the animation,\nthis allows (if disabled) to move the camera on path without changing the viewport camera view/orientation.\n\n%s"), *UFICUtils::KeymappingToString("FicsItCam.ToggleLockCamera"))))
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Show Camera Path"))]
						.IsChecked_Lambda([this]() {
							return Context->bShowPath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bShowPath = State == ECheckBoxState::Checked;
						})
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, a camera path will be drawn into the world that shows how the camera moves through space,\nit additionally shows the camera orientation at the current frame.\n\n%s"), *UFICUtils::KeymappingToString("FicsItCam.ToggleShowPath"))))
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Auto Keyframe"))]
						.IsChecked_Lambda([this]() {
							return Context->bAutoKeyframe ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bAutoKeyframe = State == ECheckBoxState::Checked;
						})
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, a change of value of a attribute will directly cause it to set/create a keyframe for that attribute at the current frame.\n\n%s"), *UFICUtils::KeymappingToString("FicsItCam.ToggleAutoKeyframe"))))
					]
					+SVerticalBox::Slot().Padding(5).AutoHeight()[
						SNew(SCheckBox)
						.Content()[SNew(STextBlock).Text(FText::FromString("Force Resolution"))]
						.IsChecked_Lambda([this]() {
							return Context->bForceResolution ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
							Context->bForceResolution = State == ECheckBoxState::Checked;
						})
						.ToolTipText(FText::FromString(FString::Printf(TEXT("If enabled, viewport will be forced to use the aspect ratio of the resolution of the animation, causing black bars to appear."))))
					]
				]
			]
			+SVerticalBox::Slot().FillHeight(1)[
				SNew(SSpacer)
			]
		]
	];
}
