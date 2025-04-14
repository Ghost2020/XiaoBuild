/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "SSSLSettingsView.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"

#include "XiaoStyle.h"
#include "ShareWidget.h"
#include "SLoginView.h"
#include "SInstallProgressView.h"

#define LOCTEXT_NAMESPACE "SSSLSettingsView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSSLSettingsView::Construct(const FArguments& InArgs)
{
	static const FText SelectFile = LOCTEXT("SelectFile_Text", "浏览文件以选择证书");
	static const FString DeskPath = "";
	
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET
			
		+SVerticalBox::Slot().AutoHeight().FIR_PADDING
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(GInstallSettings.InstallType != 1 ? LOCTEXT("CoordinatorSSL_Text", "调度器SSL证书") : LOCTEXT("AgentSSL_Text", "代理SSL证书"))
			]
			+SVerticalBox::Slot().SEC_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SelectOneOf_Text", "选择下述的安装选项"))
			]
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]

		+ SVerticalBox::Slot().THR_PADDING
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(NotUseTrustedBox, SCheckBox)
							.IsChecked(true)
							.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
								{
									if (InState == ECheckBoxState::Checked)
									{
										HaveTrustedBox->SetIsChecked(false);
									}
									else
									{
										NotUseTrustedBox->SetIsChecked(true);
									}
									GInstallSettings.bHasSSL = HaveTrustedBox->GetCheckedState() == ECheckBoxState::Checked ? true : false;
								})
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("DoNotUse_Test", "不使用受信任的证书"))
					]
			]

			+ SVerticalBox::Slot().L_PADDING(20.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("ItIsNotRecom_Test", "不推荐使用这一选项"))
			]
		]

		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth()
				[
					SAssignNew(HaveTrustedBox, SCheckBox)
					.IsChecked(false)
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						if (InState == ECheckBoxState::Checked)
						{
							NotUseTrustedBox->SetIsChecked(false);
						}
						else
						{
							HaveTrustedBox->SetIsChecked(true);
						}
						GInstallSettings.bHasSSL = HaveTrustedBox->GetCheckedState() == ECheckBoxState::Checked ? true : false;
					})
				]
				+SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock).Text(LOCTEXT("HaveATrusted_Test", "我有可信任的证书"))
				]
			]

			+SVerticalBox::Slot().AutoHeight().L_PADDING(20.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("UserYourCerificate_Text", "使用你的证书来在代理之间进行网络传输"))
			]
			+SVerticalBox::Slot().AutoHeight().L_PADDING(20.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SAssignNew(YourCertificateTextBox, SEditableTextBox)
					.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
					{
						const FString CertPath = InText.ToString();
						GInstallSettings.CertFile = TEXT("");
						YourCertificateTextBox->SetError(FText::GetEmpty());
						if (!FPaths::FileExists(CertPath))
						{
							YourCertificateTextBox->SetError(LOCTEXT("FileNotExist_Text", "文件不存在"));
							return;
						}
						GInstallSettings.CertFile = CertPath;
					})
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("Browse_Test", "浏览"))
					.OnPressed_Lambda([this]() 
					{
						TArray<FString> OutFiles;
						const FText FileTypes = LOCTEXT("Cert_FileFilter", "构建监视文件(*.xzip*)|*.xzip*|All files (*.*)|*.*");
						if (OpenFileDialog(SelectFile.ToString(), DeskPath, *FileTypes.ToString(), OutFiles))
						{
							if (OutFiles.Num() > 0)
							{
								GInstallSettings.CertFile = OutFiles[0];
								YourCertificateTextBox->SetText(FText::FromString(GInstallSettings.CertFile));
							}
						}
					})
				]
			]
			+ SVerticalBox::Slot().AutoHeight().L_PADDING(20.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("PrivateKey_Text", "使用你的私有密钥来在代理之间进行网络传输"))
			]
			+ SVerticalBox::Slot().AutoHeight().L_PADDING(20.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SAssignNew(PrivateKeyTextBox, SEditableTextBox)
					.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
					{
						const FString KeyPath = InText.ToString();
						GInstallSettings.KeyFile = TEXT("");
						PrivateKeyTextBox->SetError(FText::GetEmpty());
						if (!FPaths::FileExists(KeyPath))
						{
							PrivateKeyTextBox->SetError(LOCTEXT("FileNotExist_Text", "文件不存在"));
							return;
						}
						GInstallSettings.KeyFile = KeyPath;
					})
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("Browse_Test", "浏览"))
					.OnPressed_Lambda([this]()
					{
						TArray<FString> OutFiles;
						const FText FileTypes = LOCTEXT("PrivateKey_FileFilter", "构建监视文件(*.xzip*)|*.xzip*|All files (*.*)|*.*");
						if (OpenFileDialog(SelectFile.ToString(), DeskPath, *FileTypes.ToString(), OutFiles))
						{
							if (OutFiles.Num() > 0)
							{
								GInstallSettings.KeyFile = OutFiles[0];
								PrivateKeyTextBox->SetText(FText::FromString(GInstallSettings.KeyFile));
							}
						}
					})
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SSSLSettingsView::GetNext()
{
	if(GInstallSettings.InstallType != 1)
	{
		if(!LoginView.IsValid())
		{
			LoginView = SNew(SLoginView);
		}
		return LoginView;
	}
	
	if(!ProgressView.IsValid())
	{
		ProgressView = SNew(SInstallProgressView);
	}
	return ProgressView;
}

bool SSSLSettingsView::OnCanNext()
{
	if(HaveTrustedBox->IsChecked())
	{
		if (GInstallSettings.bHasSSL)
		{
			return !GInstallSettings.CertFile.IsEmpty() && !GInstallSettings.KeyFile.IsEmpty();
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE