#include "LiveLinkCore.h"

#include "LiveLinkProvider.h"
#include "LiveLinkTypes.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkTransformTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogCircleLiveLinkProviderCore, Log, All);

FLiveLinkProviderCoreInitArgs::FLiveLinkProviderCoreInitArgs(const int32 ArgC, TCHAR* ArgV[])
{
	const FString CmdLine = FCommandLine::BuildFromArgV(nullptr, ArgC, ArgV, nullptr);
	FCommandLine::Set(*CmdLine);

	if (FString Value; FParse::Value(*CmdLine, TEXT("-Framerate="), Value))
	{
		FParse::Value(*Value, TEXT("Numerator="), Framerate.Numerator);
		FParse::Value(*Value, TEXT("Denominator="), Framerate.Denominator);
	}

	FParse::Value(*CmdLine, TEXT("-SourceName="), SourceName);
}

LiveLinkCore::LiveLinkCore(const FLiveLinkProviderCoreInitArgs& InitArgs) :
	FrameTime(0.0), InitArgs(InitArgs)
{
}

int32 LiveLinkCore::Run()
{
	checkf(InitArgs.Framerate.AsInterval() > 0, TEXT("IdealFramerate must be greater than zero!"));
	checkf(!InitArgs.SourceName.IsEmpty(), TEXT("Source name cannot be empty!"));

	double DeltaTime = 0.0;
	FrameTime = FPlatformTime::Seconds();
	const float IdealFrameTime = InitArgs.Framerate.AsInterval();

	StartProvider();

	while (!IsEngineExitRequested())
	{
		Tick(DeltaTime);
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		FTSTicker::GetCoreTicker().Tick(DeltaTime);

		GFrameCounter++;

		IncrementalPurgeGarbage(true, FMath::Max<float>(0.002f, IdealFrameTime - (FPlatformTime::Seconds() - FrameTime)));
		FPlatformProcess::Sleep(FMath::Max<float>(0.0f, IdealFrameTime - (FPlatformTime::Seconds() - FrameTime)));

		const double CurrentTime = FPlatformTime::Seconds();
		DeltaTime = CurrentTime - FrameTime;
		FrameTime = CurrentTime;
	}

	StopProvider();
	UE_LOG(LogCircleLiveLinkProviderCore, Display, TEXT("%s Shutdown"), *InitArgs.SourceName);

	FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
	return 0;
}

LiveLinkCore::~LiveLinkCore()
{
}

void LiveLinkCore::StartProvider()
{
	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(InitArgs.SourceName);
	FLiveLinkStaticDataStruct StaticData = FLiveLinkStaticDataStruct(FLiveLinkTransformStaticData::StaticStruct());
	FLiveLinkTransformStaticData& TransformStaticData = *StaticData.Cast<FLiveLinkTransformStaticData>();
	
	TransformStaticData.PropertyNames.Add(TEXT("Cosine"));
	TransformStaticData.PropertyNames.Add(TEXT("Sinine"));

	LiveLinkProvider->UpdateSubjectStaticData(*InitArgs.SourceName, ULiveLinkTransformRole::StaticClass(), MoveTemp(StaticData));
}

void LiveLinkCore::Tick(float DeltaTime)
{
	FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct(FLiveLinkTransformFrameData::StaticStruct());
	FLiveLinkTransformFrameData& TransformFrameData = *FrameDataStruct.Cast<FLiveLinkTransformFrameData>();

	const float Radians = FMath::DegreesToRadians<float>(GFrameCounter % 360);
	const float CosValue = FMath::Cos(Radians);
	const float SinValue = FMath::Sin(Radians);
	const int ScaleFactor = 200;

	TransformFrameData.Transform.SetLocation(FVector(ScaleFactor * CosValue, ScaleFactor * SinValue, ScaleFactor));

	TransformFrameData.PropertyValues.Add(CosValue);
	TransformFrameData.PropertyValues.Add(SinValue);

	if (GFrameCounter % 100 == 0)
	{
		UE_LOG(LogCircleLiveLinkProviderCore, Display, TEXT("(%d) - Cosine: %f Sine: %f"), GFrameCounter, CosValue, SinValue);
	}


	TransformFrameData.WorldTime = FrameTime;
	const FTimecode EngineTimeCode = FTimecode(FrameTime, InitArgs.Framerate, true);
	TransformFrameData.MetaData.SceneTime = FQualifiedFrameTime(EngineTimeCode, InitArgs.Framerate);

	LiveLinkProvider->UpdateSubjectFrameData(*InitArgs.SourceName, MoveTemp(FrameDataStruct));
}

void LiveLinkCore::StopProvider() const
{
	LiveLinkProvider->RemoveSubject(*InitArgs.SourceName);
}

