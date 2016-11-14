// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
//#include "HUD/RMenuStyles.h"

////Custom implementation of the Default Game Module. 
//class FSlateUIGameModule : public FDefaultGameModuleImpl
//{
//	// Called whenever the module is starting up. In here, we unregister any style sets 
//	// (which may have been added by other modules) sharing our 
//	// style set's name, then initialize our style set. 
//	virtual void StartupModule() override
//	{
//		//Hot reload hack
//		FSlateStyleRegistry::UnRegisterSlateStyle(FRMenuStyles::GetStyleSetName());
//		FRMenuStyles::Initialize();
//	}
//
//	// Called whenever the module is shutting down. Here, we simply tell our MenuStyles to shut down.
//	virtual void ShutdownModule() override
//	{
//		FRMenuStyles::Shutdown();
//	}
//
//};
//
//IMPLEMENT_PRIMARY_GAME_MODULE(FSlateUIGameModule, RobCoG, "RobCoG" );


IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, RobCoG, "RobCoG" );

//General Log
DEFINE_LOG_CATEGORY(RobCoG);