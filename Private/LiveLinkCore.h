#pragma once

#include "CoreMinimal.h"
#include "Misc/FrameRate.h"

struct ILiveLinkProvider;

struct FLiveLinkProviderCoreInitArgs
{
	FLiveLinkProviderCoreInitArgs(int32 Argc, TCHAR* ArgV[]);

	FFrameRate Framerate = FFrameRate(60, 1);
	FString SourceName{ TEXT("CircleLiveLinkProvider" });
};

class CIRCLELIVELINKPROVIDER_API LiveLinkCore
{
public:
	explicit LiveLinkCore(const FLiveLinkProviderCoreInitArgs& InitArgs);
	int32 Run();
	~LiveLinkCore();

private:
	void StartProvider();
	void Tick(float DeltaTime);
	void StopProvider() const;

private:
	double FrameTime;
	FLiveLinkProviderCoreInitArgs InitArgs;
	TSharedPtr<ILiveLinkProvider> LiveLinkProvider;
};