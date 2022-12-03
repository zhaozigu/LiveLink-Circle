// Copyright Epic Games, Inc. All Rights Reserved.


#include "CircleLiveLinkProvider.h"

#include "RequiredProgramMainCPPInclude.h"


DEFINE_LOG_CATEGORY_STATIC(LogCircleLiveLinkProvider, Log, All);

IMPLEMENT_APPLICATION(CircleLiveLinkProvider, "CircleLiveLinkProvider");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	int32 Result = GEngineLoop.PreInit(ArgC, ArgV, TEXT(" -messaging"));
	check(Result == 0);
	check(GConfig && GConfig->IsReadyForUse());

	ProcessNewlyLoadedUObjects();

	FModuleManager::Get().StartProcessingNewlyLoadedObjects();
	FModuleManager::Get().LoadModuleChecked(TEXT("UdpMessaging"));

	FPlatformMisc::SetGracefulTerminationHandler();

	FLiveLinkProviderCoreInitArgs InitArgs(ArgC, ArgV);

	LiveLinkCore(InitArgs).Run();
	
	FEngineLoop::AppPreExit();
	FModuleManager::Get().UnloadModulesAtShutdown();
	FEngineLoop::AppExit();
	return Result;
}
