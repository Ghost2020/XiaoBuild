/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SInstallationFolderView.h"
#include "SlateOptMacros.h"
#include "Misc/EngineVersion.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "XiaoStyle.h"
#include "Misc/MessageDialog.h"
#include "SSSLSettingsView.h"
#include "SInstallProgressView.h"
#include "ShareWidget.h"
#include "Widgets/SWarningBox.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <winioctl.h>
#else
#include <fstream>
#endif

#define LOCTEXT_NAMESPACE "SInstallationFolderView"

static const FName SFolderIndex(TEXT("Id"));
static const FName SFolderType(TEXT("Type"));
static const FName SFolderPath(TEXT("Path"));
static const FName SFolderFlag(TEXT("Flag"));

static int SIndex = 0;

DECLARE_DELEGATE_TwoParams(FFolderChangedDelegate, const bool, const FString);

class SFolderListRow final : public SMultiColumnTableRow<TSharedPtr<FInstallFolder>>
{
public:
	SLATE_BEGIN_ARGS(SFolderListRow){}
		SLATE_ARGUMENT(TSharedPtr<FInstallFolder>, FolderDesc)
		SLATE_EVENT(FFolderChangedDelegate, OnSelectedChanged)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		check(InArgs._FolderDesc.IsValid());
		FolderDesc = InArgs._FolderDesc;
		OnSelectedChanged = InArgs._OnSelectedChanged;
	
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override
	{
		if(FolderDesc.IsValid())
		{
			if(const auto Folder = FolderDesc.Pin())
			{
				if (InColumnName == SFolderIndex)
				{
					return V_CENTER_WIGET(SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%ld"), ++SIndex))));
				}
				if (InColumnName == SFolderType)
				{
					return V_CENTER_WIGET(SNew(STextBlock).Text(Folder->Type ? LOCTEXT("Source_Text", "源码") : LOCTEXT("NotSource_Text", "公版")));
				}
				if(InColumnName == SFolderPath)
				{
					return SNew(STextBlock).Text(FText::FromString(Folder->Folder));
				}
				if(InColumnName == SFolderFlag)
				{
					return SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SCheckBox).IsChecked(Folder->bInstall ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
						.OnCheckStateChanged_Lambda([this] (ECheckBoxState InState)
						{
							if(FolderDesc.IsValid())
							{
								FolderDesc.Pin()->bInstall = true ? InState == ECheckBoxState::Checked : false;
								OnSelectedChanged.Execute(FolderDesc.Pin()->bInstall, FolderDesc.Pin()->Folder);
							}
						})
					];
				}
			}
		}
		return SNullWidget::NullWidget;
	}

private:
	TWeakPtr<FInstallFolder> FolderDesc = nullptr;
	FFolderChangedDelegate OnSelectedChanged;
};

#if PLATFORM_WINDOWS
static HANDLE GetVolumeHandleForFile(const wchar_t* filePath)
{
	wchar_t volume_path[MAX_PATH];
	if (!GetVolumePathName(filePath, volume_path, ARRAYSIZE(volume_path)))
		return nullptr;

	wchar_t volume_name[MAX_PATH];
	if (!GetVolumeNameForVolumeMountPoint(volume_path,
		volume_name, ARRAYSIZE(volume_name)))
		return nullptr;

	auto length = wcslen(volume_name);
	if (length && volume_name[length - 1] == L'\\')
		volume_name[length - 1] = L'\0';

	return CreateFile(volume_name, 0,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
}
#endif

static bool IsSSDDirver(const FString& InPath)
{
	bool bIsSSD{ false };
#if PLATFORM_WINDOWS
	HANDLE volume = GetVolumeHandleForFile(*InPath);
	if (volume == INVALID_HANDLE_VALUE)
	{
		return false; /*invalid path! throw?*/
	}

	STORAGE_PROPERTY_QUERY query{};
	query.PropertyId = StorageDeviceSeekPenaltyProperty;
	query.QueryType = PropertyStandardQuery;
	DWORD count;
	DEVICE_SEEK_PENALTY_DESCRIPTOR result{};
	if (DeviceIoControl(volume, IOCTL_STORAGE_QUERY_PROPERTY,
		&query, sizeof(query), &result, sizeof(result), &count, nullptr))
	{
		bIsSSD = !result.IncursSeekPenalty;
	}
	else { /*fails for network path, etc*/ }
	CloseHandle(volume);
	return bIsSSD;
#else
	const std::string Path = TCHAR_TO_UTF8(*InPath);
	char buffer[256];
	std::string devicePath = "/dev/";

	// 使用 stat 命令来获取路径所在的设备
	std::string command = "df --output=source " + Path + " | tail -n 1";
	FILE* fp = popen(command.c_str(), "r");
	if (fp == nullptr) {
		std::cerr << "Failed to run df command" << std::endl;
		return false;
	}
	if (fgets(buffer, sizeof(buffer), fp) != nullptr) 
	{
		devicePath += strtok(buffer, "\n");
	}
	fclose(fp);

	std::string rotationalFile = "/sys/block/" + devicePath + "/queue/rotational";

	std::ifstream file(rotationalFile);
	if (!file.is_open()) 
	{
		std::cerr << "Failed to open " << rotationalFile << std::endl;
		return false;
	}

	int rotational;
	file >> rotational;
	file.close();

	// 0 表示 SSD，1 表示 HDD
	return (rotational == 0);
#endif
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInstallationFolderView::Construct(const FArguments& InArgs, const bool InType)
{
	bType = InType;

	GetAllEngineFolder(FolderArray);
	FolderArray.RemoveAll([](const TSharedPtr<FInstallFolder> InFolder) 
	{
		return !InFolder->bSupport;
	});

	static FText SelectText = LOCTEXT("SelectInstallFolder_FileDesc", "选择安装目录...");
	
	ChildSlot
	[
		SNew(SScrollBox).Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			TOP_WIDGET

			V_ADD_CHECKBOX(AutoRunTrayCheckBox, GInstallSettings.bEnableAutoTray, LOCTEXT("AutoRunTray_Text", "启动时是否自动运行Tray"))
		
			+SVerticalBox::Slot().AutoHeight()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().FIR_PADDING
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AgentCacheDir_Text", "代理Cas目录"))
				]
				+SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SWarningBox)
					.Message(LOCTEXT("CacheWarning_Text", "存储目录会占用大量的存储空间，请选择容量大的SSD盘"))
				]
			]

			+SVerticalBox::Slot().THR_PADDING
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth()
				[
					SAssignNew(InstallFolderBox, SEditableTextBox).MinDesiredWidth(600.0f)
					.Text(FText::FromString(GInstallSettings.CacheFolder))
					.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
					{
						ValidationFolder(InText.ToString());
					})
				]
				+SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton).Text(LOCTEXT("Borwse_Text", "浏览"))
					.OnClicked_Lambda([this]()
					{
						if(IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
						{
							if(FString OutFolder; DesktopPlatform->OpenDirectoryDialog(
								FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
								SelectText.ToString(),
								GInstallSettings.CacheFolder,
								OutFolder
							))
							{
								ValidationFolder(OutFolder);
							}
						}
						return FReply::Handled();
					})
				]
			]

			+ SVerticalBox::Slot().AutoHeight().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DestinationFolder_Text", "UnrealEngine安装"))
				.Visibility_Raw(this, &SInstallationFolderView::CanInstallUnrealEngine)
			]

			+SVerticalBox::Slot().THR_PADDING.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Horizontal)
				.Visibility_Raw(this, &SInstallationFolderView::CanInstallUnrealEngine)
				+ SScrollBox::Slot()
				[
					SAssignNew(FolderListView, SListView<TSharedPtr<FInstallFolder>>)
					.ListItemsSource(&FolderArray)
					.Orientation(Orient_Vertical)
					.SelectionMode(ESelectionMode::Type::Multi)
					.EnableAnimatedScrolling(true)
					// .ItemHeight(50.0f)
					.AllowOverscroll(EAllowOverscroll::Yes)
					.OnGenerateRow_Raw(this, &SInstallationFolderView::OnGenerateRow)
					.HeaderRow(
						SNew(SHeaderRow)
						+SHeaderRow::Column(SFolderIndex)
						.FixedWidth(50.0f)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Id_Text", " 序号 "))

						+SHeaderRow::Column(SFolderType)
						.ManualWidth(100.0f)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Type_Text", "引擎类型"))
						.DefaultTooltip(LOCTEXT("Type_Tooltip", "引擎类型"))
						
						+SHeaderRow::Column(SFolderPath)
						.ManualWidth(500.0f)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("EngineRoot_Text", "引擎目录"))
						.DefaultTooltip(LOCTEXT("EngineRoot_Tooltip", "UE引擎目录"))
						
						+SHeaderRow::Column(SFolderFlag)
			    	    .ManualWidth(60.0f)
			    	    .VAlignHeader(VAlign_Center)
			    	    .DefaultLabel(LOCTEXT("Flag_Text", "是否安装"))
			    	    .DefaultTooltip(LOCTEXT("Flag_Tooltip", "..."))
					)
				]
			]
		]
	];

	DefualtCASDir = GInstallSettings.CacheFolder;
	// 创建默认存储目录
	if (!FPaths::DirectoryExists(DefualtCASDir))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*DefualtCASDir);
	}

	ValidationFolder(DefualtCASDir);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInstallationFolderView::OnResetState()
{
	if (InstallFolderBox)
	{
		InstallFolderBox->SetError(FText::GetEmpty());
	}
}

TWeakPtr<SWizardView> SInstallationFolderView::GetNext()
{
	if (GInstallSettings.InstallType & EComponentTye::CT_Coordinator)
	{
		if (!SSLSettingsView.IsValid())
		{
			SSLSettingsView = SNew(SSSLSettingsView);
		}
		return SSLSettingsView;
	}
	if (GInstallSettings.InstallType & EComponentTye::CT_Agent)
	{
		if (!InStallView.IsValid())
		{
			InStallView = SNew(SInstallProgressView);
		}
		return InStallView;
	}
	check(0)
	return nullptr;
}

bool SInstallationFolderView::OnCanNext()
{
	return bPass;
}

TSharedRef<ITableRow> SInstallationFolderView::OnGenerateRow(const TSharedPtr<FInstallFolder> InDesc,
	const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SFolderListRow, InTableView).FolderDesc(InDesc)
	.OnSelectedChanged_Lambda([](const bool InChecked, const FString InFolderPath)
	{
		if(InChecked)
		{
			GInstallSettings.EngineFolders.Add(InFolderPath);
			auto DeskPlatformModule = FDesktopPlatformModule::Get();
			GInstallSettings.EngineTypes.Add(DeskPlatformModule->IsSourceDistribution(InFolderPath) ? 1 : 0);
			FEngineVersion EngineVersion;
			DeskPlatformModule->TryGetEngineVersion(InFolderPath, EngineVersion);
			const FString VersionString = EngineVersion.ToString();
			GInstallSettings.EngineVersions.Add(VersionString);
		}
		else
		{
			const int Index = GInstallSettings.EngineFolders.Find(InFolderPath);
			GInstallSettings.EngineFolders.RemoveAt(Index);
			GInstallSettings.EngineTypes.RemoveAt(Index);
			GInstallSettings.EngineVersions.RemoveAt(Index);
		}
	});
}

EColumnSortMode::Type SInstallationFolderView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

void SInstallationFolderView::ValidationFolder(const FString& InFolder)
{
	const FString FullPath = FPaths::ConvertRelativePathToFull(InFolder);
	InstallFolderBox->SetError(FText::GetEmpty());
	bPass = false;
	if (!FPaths::DirectoryExists(FullPath))
	{
		InstallFolderBox->SetError(LOCTEXT("NotValidFolder_Text", "不是一个有效的目录"));
		return;
	}
	if (!IsSSDDirver(FullPath))
	{
		if (!FPaths::IsSamePath(FullPath, DefualtCASDir))
		{
			InstallFolderBox->SetError(LOCTEXT("NotSSD_Text", "非SSD类型硬盘，会极大影响程序性能"));
		}
	}

	uint64 TotalNumberOfBytes, NumberOfFreeBytes;
	if (FPlatformMisc::GetDiskTotalAndFreeSpace(InFolder, TotalNumberOfBytes, NumberOfFreeBytes))
	{
		const auto FreeBytes = static_cast<uint32>(NumberOfFreeBytes / static_cast<uint64>(1024 * 1024 * 1024));
		if (FreeBytes < 10)
		{
			InstallFolderBox->SetError(LOCTEXT("NotAvalibaleSpace_Text", "空余空间少于10GB,建议清理一下！"));
		}
	}

	if (FullPath.Len() > FPlatformMisc::GetMaxPathLength())
	{
		InstallFolderBox->SetError(LOCTEXT("PathToolLong_Text", "路径长度已经超过最大允许值"));
		return;
	}

	InstallFolderBox->SetText(FText::FromString(FullPath));
	GInstallSettings.CacheFolder = FullPath;
	bPass = true;
}

EVisibility SInstallationFolderView::CanInstallUnrealEngine() const
{
	return (GInstallSettings.InstallType & EComponentTye::CT_Agent) ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE