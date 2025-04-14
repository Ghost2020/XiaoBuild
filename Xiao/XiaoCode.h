/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#ifdef DROGON_EXPORT
#include "drogon/HttpTypes.h"
static const drogon::HttpStatusCode Error_ParamNotFound = static_cast<drogon::HttpStatusCode>(550);
static const drogon::HttpStatusCode Error_SqlExecute = static_cast<drogon::HttpStatusCode>(551);
static const drogon::HttpStatusCode Error_IDAreadyExist = static_cast<drogon::HttpStatusCode>(552);
static const drogon::HttpStatusCode Error_BytesToJson = static_cast<drogon::HttpStatusCode>(553);
static const drogon::HttpStatusCode Error_JsonUpdateSettings = static_cast<drogon::HttpStatusCode>(554);
static const drogon::HttpStatusCode Error_NoWhichParam = static_cast<drogon::HttpStatusCode>(555);
static const drogon::HttpStatusCode Error_NoFindWhich = static_cast<drogon::HttpStatusCode>(556);
static const drogon::HttpStatusCode Error_Delete = static_cast<drogon::HttpStatusCode>(557);
static const drogon::HttpStatusCode Error_IDNotExist = static_cast<drogon::HttpStatusCode>(558);
static const drogon::HttpStatusCode Error_Decypt = static_cast<drogon::HttpStatusCode>(559);
static const drogon::HttpStatusCode Error_HeaderNotFound = static_cast<drogon::HttpStatusCode>(560);
static const drogon::HttpStatusCode Error_FromJson = static_cast<drogon::HttpStatusCode>(561);
static const drogon::HttpStatusCode Error_ProtobufParseFromString = static_cast<drogon::HttpStatusCode>(562);
static const drogon::HttpStatusCode Error_AreadyExist = static_cast<drogon::HttpStatusCode>(563);
#endif


enum EExitCode : uint16
{
    Success = 0,                                //正常返回
    Error_Unknown = 1,                          //未知错误
    Error_Arguments = 2,                        //参数错误
    Error_UnknownCommand = 3,                   //不支持的command
    Error_Singleton = 4,                        //单例冲突
	Error_BuildFailure = 5,                     //构建失败
    Error_SDKNotFound = 10,                     //SDK没找到
    Error_ProvisionNotFound = 11,               //条款未找到
    Error_CertificateNotFound = 12,             //证书未找到
    Error_ProvisionAndCertificateNotFound = 13,
    Error_InfoPListNotFound = 14,
    Error_KeyNotFoundInPList = 15,             
    Error_ProvisionExpired = 16,                //条款过期
    Error_CertificateExpired = 17,              //证书过期
    Error_CertificateProvisionMismatch = 18,    //证书条款不匹配
    Error_CodeUnsupported = 19,                 //代码不支持
    Error_PluginsUnsupported = 20,              //插件不支持
    Error_UnknownDeployFailure = 26,
    Error_UnknownBuildFailure = 27,
    Error_UnknownPackageFailure = 28,
    Error_UnknownLaunchFailure = 29,
    Error_StageMissingFile = 30,
    Error_FailedToCreateIPA = 31,
    Error_FailedToCodeSign = 32,
    Error_DeviceBackupFailed = 33,
    Error_AppUninstallFailed = 34,              //App卸载失败
    Error_AppInstallFailed = 35,                //App安装失败
    Error_AppNotFound = 36,                     //App未找到
    Error_StubNotSignedCorrectly = 37,
    Error_IPAMissingInfoPList = 38,
    Error_DeleteFile = 39,                      //删除文件失败
    Error_DeleteDirectory = 40,                 //删除文件夹失败
    Error_CreateDirectory = 41,                 //创建文件夹失败
    Error_CopyFile = 42,                        //拷贝文件失败
    Error_OnlyOneObbFileSupported = 50,
    Error_FailureGettingPackageInfo = 51,
    Error_OnlyOneTargetConfigSupported = 52,
    Error_ObbNotFound = 53,
    Error_NoApkSuitableForArchitecture = 55,
    Error_FilesInstallFailed = 56,
    Error_RemoteCertificatesNotFound = 57,      // 远程验证失败
    Error_LauncherFailed = 100,                 // 登陆器失败
    Error_UATLaunchFailure = 101,
    Error_FailedToDeleteStagingDirectory = 102,
    Error_MissingExecutable = 103,              // 丢失可执行程序
    Error_DeviceNotSetupForDevelopment = 150,
    Error_DeviceOSNewerThanSDK = 151,
	Error_TestFailure = 152,                    //测试失败
	Error_SymbolizedSONotFound = 153,
	Error_LicenseNotAccepted = 154,             // 证书不接受
	Error_AndroidOBBError = 155,
	Error_SDKInstallFailed = 200,               //SDK安装失败
	Error_DeviceUpdateFailed = 201,             // 设备更新失败

	Error_JsonCantDeserialize = 301,
	Error_CantGetFormatVersion = 302,
	Error_CantGetEnvironments = 303,
	Error_CantGetTools = 304,
	Error_NameInValid = 305,
	Error_BoolInValid = 306,
	Error_CantAllowIntercept = 307,
	Error_CantParams = 308,
	Error_CantPathField = 309,
	Error_CantSkipIfProjectFailed = 311,
	Error_CantAutoReserveMemory = 313,
	Error_CantOutputFileMasks = 314,
	Error_CantGetProject = 315,
	Error_CantGetTasks = 316,
	Error_CantCreateSystemFolder = 317,
	Error_CantSaveMonFile = 318,
	Error_CantCompress = 319,

	Error_CantOpenChannel = 401,
	Error_CantDeclareExchange = 402,
	Error_CantDeclareConsumer = 403,

	Error_CantCreateIPCMemory = 501,
	Error_CantCreateMappedRegion = 502,
	Error_CantCreateMQ = 503,
};