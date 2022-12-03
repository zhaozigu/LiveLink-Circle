// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CircleLiveLinkProvider : ModuleRules
{
	public CircleLiveLinkProvider(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add("Runtime/Launch/Public");

		PrivateIncludePaths.Add("Runtime/Launch/Private");      // For LaunchEngineLoop.cpp include

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Projects",
			"LiveLinkMessageBusFramework",
			"LiveLinkInterface",
			"Messaging",
			"UdpMessaging",
		});
	}
}
