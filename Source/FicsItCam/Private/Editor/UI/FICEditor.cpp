﻿#include "Editor/UI/FICEditor.h"

#include "FICSubsystem.h"
#include "Engine/World.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Editor/FICEditorContext.h"
#include "Editor/FICEditorSubsystem.h"
#include "Slate/SceneViewport.h"
#include "Widgets/Docking/SDockTab.h"
#include "Editor/UI/FICTimeline.h"
#include "Editor/UI/FICEditorSettings.h"
#include "Editor/UI/FICSceneObjectDetails.h"
#include "Editor/UI/FICSceneObjectOutliner.h"
#include "Editor/UI/FICSceneSettings.h"
#include "Slate/Private/Framework/Docking/SDockingArea.h"
#include "Slate/Private/Framework/Docking/SDockingTabStack.h"

#define LOCTEXT_NAMESPACE "FicsItCam.Editor"

FSlateColorBrush SFICEditor::Background = FSlateColorBrush(FColor::FromHex("030303"));

void SFICEditor::Construct(const FArguments& InArgs, UFICEditorContext* InContext, TSharedPtr<SWidget> InGameWidget, TSharedPtr<SViewport> InViewport) {
	Context = InContext;
	GameWidget = InGameWidget;
	GameViewport = InViewport;

	TSharedRef<SDockTab> MajorTab = SNew(SDockTab).TabRole(ETabRole::MajorTab);
	TabManager = FGlobalTabmanager::Get()->NewTabManager(MajorTab);
	TabManager->SetOnPersistLayout(FTabManager::FOnPersistLayout::CreateLambda([this](const TSharedRef<FTabManager::FLayout>& Layout) {
		AFICEditorSubsystem::GetFICEditorSubsystem(Context)->LastEditorLayout = Layout->ToString();
	}));

	RegisterTabs();

	DefaultLayout = FTabManager::NewLayout("Default")
	->AddArea(FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split(FTabManager::NewSplitter()
			->SetSizeCoefficient(0.7)
			->Split(FTabManager::NewSplitter()
				->SetSizeCoefficient(0.2)
				->SetOrientation(Orient_Vertical)
				->Split(FTabManager::NewStack()->AddTab("Scene Object Outliner", ETabState::OpenedTab))
				->Split(FTabManager::NewStack()->AddTab("Scene Object Details", ETabState::OpenedTab))
				->Split(FTabManager::NewStack()->AddTab("Scene Settings", ETabState::OpenedTab))
				->Split(FTabManager::NewStack()->AddTab("Editor Settings", ETabState::OpenedTab)))
			->Split(FTabManager::NewStack()->AddTab("Viewport", ETabState::OpenedTab)->SetSizeCoefficient(0.8)->SetHideTabWell(true)))
		->Split(FTabManager::NewStack()->AddTab("Timeline", ETabState::OpenedTab)->SetSizeCoefficient(0.3)->SetHideTabWell(true)));
	
	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(&Background)
		]
		+SOverlay::Slot()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().AutoHeight()[
				CreateMenuBar().MakeWidget()
			]
			+SVerticalBox::Slot().Expose(EditorSlot)
		]
	];

	LoadLayout(nullptr);
}

void SFICEditor::RegisterTabs() {
	TabManager->RegisterTabSpawner("Viewport", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		TSharedRef<SDockTab> ViewportTab = SNew(SDockTab)
			.Content()[
				GameWidget.ToSharedRef()
			].OnCanCloseTab(false)
			.Label(FText::FromString("Viewport"));
		TabManager->SetMainTab(ViewportTab);
		return ViewportTab;
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Scene Object Outliner", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICSceneObjectOutliner, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Scene Object Details", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICSceneObjectDetails, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Scene Settings", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICSceneSettings, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));

	TabManager->RegisterTabSpawner("Editor Settings", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICEditorSettings, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));
	
	TabManager->RegisterTabSpawner("Timeline", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) {
		return SNew(SDockTab)
		.Content()[
			SNew(SFICTimelinePanel, Context)
		];
	}), FCanSpawnTab::CreateLambda([](const FSpawnTabArgs& Args) {
		return true;
	}));
}

FMenuBarBuilder SFICEditor::CreateMenuBar() {
	FMenuBarBuilder MenuBarBuilder(nullptr);
	MenuBarBuilder.AddMenuEntry(LOCTEXT("Exit", "Exit"), LOCTEXT("ExitTT", "Closes and Exits the Editor"), FSlateIcon(), FExecuteAction::CreateLambda([this]() {
		TabManager->SavePersistentLayout();
		AFICEditorSubsystem::GetFICEditorSubsystem(Context->GetScene()->GetWorld())->CloseEditor();
	}));
	MenuBarBuilder.AddPullDownMenu(LOCTEXT("View", "View"), LOCTEXT("ViewTT", "Views, Panels & Windows"), FNewMenuDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder) {
		TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
	}));
	MenuBarBuilder.AddPullDownMenu(LOCTEXT("Layout", "Layout"), LOCTEXT("LayoutTT", "Editor Panel/View Layouts"), FNewMenuDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder) {
		MenuBuilder.AddMenuEntry(LOCTEXT("Default", "Default"), LOCTEXT("DefaultTT", "Default Editor Layout"), FSlateIcon(), FExecuteAction::CreateLambda([this]() {
			LoadLayout(nullptr);
		}));
		AFICEditorSubsystem* SubSys = AFICEditorSubsystem::GetFICEditorSubsystem(Context);
		if (SubSys->EditorLayouts.Num() > 0) {
			MenuBuilder.AddSeparator();
			for (const TPair<FString, FString> Layout : SubSys->EditorLayouts) {
				MenuBuilder.AddMenuEntry(FText::FromString(Layout.Key), TAttribute<FText>(), FSlateIcon(), FExecuteAction::CreateLambda([this, Layout]() {
					LoadLayout(FTabManager::FLayout::NewFromString(Layout.Value));
				}));
			}
		}
		MenuBuilder.AddSeparator();
		MenuBuilder.AddSubMenu(LOCTEXT("LayoutManager", "Manage Layouts"), LOCTEXT("LayoutManagerTT", "Opens a window that allows you to control your collection of layouts."), FNewMenuDelegate::CreateLambda([this, SubSys](FMenuBuilder& MenuBuilder) {
			MenuBuilder.AddWrapperSubMenu(LOCTEXT("NewLayout", "New Layout"), LOCTEXT("NewLayoutTT", "Stores your current layout for later reuse."), FOnGetContent::CreateLambda([this, SubSys]() {
				TSharedRef<SEditableTextBox> TextBox = SNew(SEditableTextBox).MinDesiredWidth(100);
				
				return SNew(SGridPanel)
				+SGridPanel::Slot(0, 0)
				.Padding(5)
				.VAlign(VAlign_Center)[
					SNew(STextBlock)
					.Text(LOCTEXT("NewLayout_LayoutName", "Layout Name: "))
				]
				+SGridPanel::Slot(1, 0)
				.Padding(5)[
					TextBox
				]
				+SGridPanel::Slot(0, 1)
				.ColumnSpan(2)
				.HAlign(HAlign_Left)
				.Padding(5)[
					SNew(SButton)
					.Text(LOCTEXT("NewLayout_StoreLayout", "Store Layout"))
					.OnClicked_Lambda([this, SubSys, TextBox]() {
						FString Text = TextBox->GetText().ToString().TrimStartAndEnd();
						if (Text.Len() > 0) {
							SubSys->EditorLayouts.Add(Text, TabManager->PersistLayout()->ToString());
							// TODO: Maybe add notification for successful save of layout
						}
						return FReply::Handled();
					})
				];
			}), FSlateIcon());
			if (SubSys->EditorLayouts.Num() > 0) {
				MenuBuilder.AddSeparator();
				for (const TPair<FString, FString> Layout : SubSys->EditorLayouts) {
					MenuBuilder.AddMenuEntry(FText::FromString(LOCTEXT("RemoveLayout", "Remove ").ToString() + Layout.Key), TAttribute<FText>(), FSlateIcon(), FExecuteAction::CreateLambda([this, Layout, SubSys]() {
						SubSys->EditorLayouts.Remove(Layout.Key);
					}));
				}
			}
		}));
	}));
	return MenuBarBuilder;
}

void SFICEditor::LoadLayout(TSharedPtr<FTabManager::FLayout> Layout) {
	FString LastLayout = AFICEditorSubsystem::GetFICEditorSubsystem(Context)->LastEditorLayout;
	if (LastLayout.Len() > 0) {
		Layout = FTabManager::FLayout::NewFromString(LastLayout);
	}
	if (!Layout) Layout = DefaultLayout.ToSharedRef();

	EditorSlot->DetachWidget();
	TabManager->CloseAllAreas();
	EditorSlot->AttachWidget(TabManager->RestoreFrom(Layout.ToSharedRef(), FSlateApplication::Get().FindWidgetWindow(SharedThis(this))).ToSharedRef());
}

void SFICEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	if (bIsLeft || bIsRight) {
		KeyPressTime += InDeltaTime;
		while (KeyPressTime > 0.5) {
			Context->SetCurrentFrame(Context->GetCurrentFrame() + (bIsLeft ? -1 : 1));
			KeyPressTime -= 0.2;
		}
	}

	FVector2D ViewportSize = GameWidget->GetCachedGeometry().GetAbsoluteSize();
	FVector2D GameSize = FVector2D(Context->GetScene()->ResolutionWidth, Context->GetScene()->ResolutionHeight);
	FVector2D Size = Context->GetScene()->GetWorld()->GetGameViewport()->GetGameViewport()->GetSizeXY();
	Size *= Scalability::GetResolutionScreenPercentage() / 100.0;
	float SecondaryPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SecondaryScreenPercentage.GameViewport"))->GetFloat();
	if (SecondaryPercentage) Size *= SecondaryPercentage / 100.0;
	Context->SensorWidthAdjust = GameSize.X / Size.X;
}

bool IsAction(UObject* Context, const FKeyEvent& InKeyEvent, const FName& ActionName) {
	TArray<FInputActionKeyMapping> Mappings = Context->GetWorld()->GetFirstPlayerController()->PlayerInput->GetKeysForAction(ActionName);
	if (Mappings.Num() > 0) {
		const FInputActionKeyMapping& Mapping = Mappings[0];
		return Mapping.Key == InKeyEvent.GetKey() &&
			Mapping.bAlt == InKeyEvent.GetModifierKeys().IsAltDown() &&
			Mapping.bCmd == InKeyEvent.GetModifierKeys().IsCommandDown() &&
			Mapping.bCtrl == InKeyEvent.GetModifierKeys().IsControlDown() &&
			Mapping.bShift == InKeyEvent.GetModifierKeys().IsShiftDown();
	}
	return false;
}

FReply SFICEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleCursor"))) {
		if (GameWidget->HasUserFocusedDescendants(InKeyEvent.GetUserIndex())) {
			FSlateApplication::Get().SetUserFocus(InKeyEvent.GetUserIndex(), SharedThis(this));
			APlayerController* Controller = Context->GetScene()->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(Controller);
		} else {
			FSlateApplication::Get().SetUserFocusToGameViewport(InKeyEvent.GetUserIndex());
			APlayerController* Controller = Context->GetScene()->GetWorld()->GetFirstPlayerController();
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(Controller);
		}
		return FReply::Handled();
	} else /*if (!GameWidget->HasAnyUserFocusOrFocusedDescendants())*/ {
		if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleAllKeyframes"))) {
			auto Change = MakeShared<FFICChange_Group>();
			Change->PushChange(MakeShared<FFICChange_ActiveFrame>(Context, TNumericLimits<int64>::Min(), Context->GetCurrentFrame()));
			BEGIN_ATTRIB_CHANGE(Context->GetAllAttributes()->GetAttribute())
			// TODO: Toggle All keyframes (dynamic check if all attributes have keyframe and if is different from current value
			END_ATTRIB_CHANGE(Change)
			Context->ChangeList.PushChange(Change);
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.PrevFrame"))) {
			int64 Rate = 1;
			if (InKeyEvent.GetModifierKeys().IsControlDown()) Rate = 10;
			Context->SetCurrentFrame(Context->GetCurrentFrame()-Rate);
			bIsLeft = true;
			KeyPressTime = 0;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextFrame"))) {
			int64 Rate = 1;
			if (InKeyEvent.GetModifierKeys().IsControlDown()) Rate = 10;
			Context->SetCurrentFrame(Context->GetCurrentFrame()+Rate);
			bIsRight= true;
			KeyPressTime = 0;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.PrevKeyframe"))) {
			int64 Time;
			TSharedPtr<FFICKeyframe> KF = Context->GetAllAttributes()->GetAttribute().GetPrevKeyframe(Context->GetCurrentFrame(), Time);
			if (KF) Context->SetCurrentFrame(Time);
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextKeyframe"))) {
			int64 Time;
			TSharedPtr<FFICKeyframe> KF = Context->GetAllAttributes()->GetAttribute().GetNextKeyframe(Context->GetCurrentFrame(), Time);
			if (KF) Context->SetCurrentFrame(Time);
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleAutoKeyframe"))) {
			Context->bAutoKeyframe = !Context->bAutoKeyframe;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleShowPath"))) {
			Context->bShowPath = !Context->bShowPath;
			return FReply::Handled();
		} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleLockCamera"))) {
			Context->bMoveCamera = !Context->bMoveCamera;
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::Z && InKeyEvent.IsControlDown()) {
			TSharedPtr<FFICChange> Change = Context->ChangeList.PopChange();
			if (Change) Change->UndoChange();
			return FReply::Handled();
		} else if (InKeyEvent.GetKey() == EKeys::Y && InKeyEvent.IsControlDown()) {
			TSharedPtr<FFICChange> Change = Context->ChangeList.PushChange();
			if (Change) Change->RedoChange();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SFICEditor::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.PrevFrame")) && bIsLeft) {
		bIsLeft = false;
		return FReply::Handled();
	} else if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.NextFrame")) && bIsRight) {
		bIsRight = false;
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
}

FReply SFICEditor::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetModifierKeys().IsControlDown()) {
		float Delta = MouseEvent.GetWheelDelta() * 3;
		int64 Range = Context->GetScene()->AnimationRange.Length();
		while (Range > 300) {
			Range /= 10;
			Delta *= 10;
		}
		Context->SetCurrentFrame(Context->GetCurrentFrame() + Delta);
		return FReply::Handled();
	} else if (MouseEvent.GetModifierKeys().IsShiftDown()) {
		float Delta = MouseEvent.GetWheelDelta();
		//Context->SetFlySpeed(Context->GetFlySpeed() + Delta);
		// TODO: Fly Speed!
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFICEditor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		if (GameViewport->HasAnyUserFocusOrFocusedDescendants()) {
			AFICEditorSubsystem::GetFICEditorSubsystem(Context)->OnLeftMouseDown();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SFICEditor::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		if (GameViewport->HasAnyUserFocusOrFocusedDescendants()) {
			AFICEditorSubsystem::GetFICEditorSubsystem(Context)->OnLeftMouseUp();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);
}

bool SFICEditor::SupportsKeyboardFocus() const {
	return true;
}

void SFICEditor::OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) {
	SCompoundWidget::OnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
	if (!PreviousFocusPath.ContainsWidget(GameWidget.ToSharedRef()) && NewWidgetPath.ContainsWidget(GameWidget.ToSharedRef())) {
		if (FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::RightMouseButton)) {
			Context->GetPlayerCharacter()->SetControlView(true, true);
		}
	}
}

FReply SFICEditor::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (IsAction(Context, InKeyEvent, TEXT("FicsItCam.ToggleCursor")) || IsAction(Context, InKeyEvent, TEXT("PauseGame"))) {
		Context->GetPlayerCharacter()->ControlViewToggle();
		return FReply::Handled();
	}
	return SCompoundWidget::OnPreviewKeyDown(MyGeometry, InKeyEvent);
}
