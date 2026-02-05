///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - Licensed under the MIT license.
///
/// Name         :  Nexus.h
/// Description  :  C/C++ Definitions header for Nexus API.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef NEXUS_H
#define NEXUS_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#include <stdbool.h>
#endif
#include <windows.h>

#define NEXUS_API_VERSION 6

/* Event identifiers */
#define EV_ARCDPS_COMBATEVENT_LOCAL_RAW "EV_ARCDPS_COMBATEVENT_LOCAL_RAW"
#define EV_ARCDPS_COMBATEVENT_SQUAD_RAW "EV_ARCDPS_COMBATEVENT_SQUAD_RAW"

typedef enum ERenderType
{
	RT_PreRender,
	RT_Render,
	RT_PostRender,
	RT_OptionsRender
} ERenderType;

typedef enum EMHStatus
{
	MH_UNKNOWN = -1,
	MH_OK = 0,
	MH_ERROR_ALREADY_INITIALIZED,
	MH_ERROR_NOT_INITIALIZED,
	MH_ERROR_ALREADY_CREATED,
	MH_ERROR_NOT_CREATED,
	MH_ERROR_ENABLED,
	MH_ERROR_DISABLED,
	MH_ERROR_NOT_EXECUTABLE,
	MH_ERROR_UNSUPPORTED_FUNCTION,
	MH_ERROR_MEMORY_ALLOC,
	MH_ERROR_MEMORY_PROTECT,
	MH_ERROR_MODULE_NOT_FOUND,
	MH_ERROR_FUNCTION_NOT_FOUND
} EMHStatus;

typedef enum ELogLevel
{
	ELogLevel_CRITICAL = 1,
	ELogLevel_WARNING = 2,
	ELogLevel_INFO = 3,
	ELogLevel_DEBUG = 4,
	ELogLevel_TRACE = 5
} ELogLevel;

typedef enum EAddonFlags
{
	EAddonFlags_None              = 0,
	EAddonFlags_IsVolatile        = 1 << 0,
	EAddonFlags_DisableHotloading = 1 << 1
} EAddonFlags;

typedef enum EUpdateProvider
{
	EUpdateProvider_None     = 0,
	EUpdateProvider_Raidcore = 1,
	EUpdateProvider_GitHub   = 2,
	EUpdateProvider_Direct   = 3,
	EUpdateProvider_Self     = 4
} EUpdateProvider;

typedef struct AddonVersion
{
	uint16_t Major;
	uint16_t Minor;
	uint16_t Build;
	uint16_t Revision;
} AddonVersion;

struct AddonAPI;

typedef void        (*ADDON_LOAD)(AddonAPI* aAPI);
typedef void        (*ADDON_UNLOAD)();
typedef void        (*GUI_RENDER)();
typedef void        (*GUI_ADDRENDER)(ERenderType aRenderType, GUI_RENDER aRenderCallback);
typedef void        (*GUI_REMRENDER)(GUI_RENDER aRenderCallback);
typedef void        (*GUI_REGISTERCLOSEONESCAPE)(const char* aWindowName, bool* aIsVisible);
typedef void        (*GUI_DEREGISTERCLOSEONESCAPE)(const char* aWindowName);
typedef void        (*UPDATER_REQUESTUPDATE)(uint32_t aSignature, const char* aUpdateURL);
typedef const char* (*PATHS_GETGAMEDIR)();
typedef const char* (*PATHS_GETADDONDIR)(const char* aName);
typedef const char* (*PATHS_GETCOMMONDIR)();
typedef EMHStatus   (__stdcall* MINHOOK_CREATE)(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal);
typedef EMHStatus   (__stdcall* MINHOOK_REMOVE)(LPVOID pTarget);
typedef EMHStatus   (__stdcall* MINHOOK_ENABLE)(LPVOID pTarget);
typedef EMHStatus   (__stdcall* MINHOOK_DISABLE)(LPVOID pTarget);
typedef void        (*LOGGER_LOG)(ELogLevel aLogLevel, const char* aChannel, const char* aStr);
typedef void        (*ALERTS_NOTIFY)(const char* aMessage);
typedef void        (*EVENT_CONSUME)(void* aEventArgs);
typedef void        (*EVENTS_RAISE)(const char* aIdentifier, void* aEventData);
typedef void        (*EVENTS_RAISENOTIFICATION)(const char* aIdentifier);
typedef void        (*EVENTS_RAISE_TARGETED)(uint32_t aSignature, const char* aIdentifier, void* aEventData);
typedef void        (*EVENTS_RAISENOTIFICATION_TARGETED)(uint32_t aSignature, const char* aIdentifier);
typedef void        (*EVENTS_SUBSCRIBE)(const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

typedef struct AddonDefinition
{
	uint32_t        Signature;
	uint32_t        APIVersion;
	const char*     Name;
	AddonVersion    Version;
	const char*     Author;
	const char*     Description;
	ADDON_LOAD      Load;
	ADDON_UNLOAD    Unload;
	EAddonFlags     Flags;
	EUpdateProvider Provider;
	const char*     UpdateLink;
} AddonDefinition;

typedef struct AddonAPI
{
	void* SwapChain;
	void* ImguiContext;
	void* ImguiMalloc;
	void* ImguiFree;

	GUI_ADDRENDER                     GUI_Register;
	GUI_REMRENDER                     GUI_Deregister;
	UPDATER_REQUESTUPDATE             RequestUpdate;
	LOGGER_LOG                        Log;
	ALERTS_NOTIFY                     GUI_SendAlert;
	GUI_REGISTERCLOSEONESCAPE         GUI_RegisterCloseOnEscape;
	GUI_DEREGISTERCLOSEONESCAPE       GUI_DeregisterCloseOnEscape;
	PATHS_GETGAMEDIR                  Paths_GetGameDirectory;
	PATHS_GETADDONDIR                 Paths_GetAddonDirectory;
	PATHS_GETCOMMONDIR                Paths_GetCommonDirectory;
	MINHOOK_CREATE                    MinHook_Create;
	MINHOOK_REMOVE                    MinHook_Remove;
	MINHOOK_ENABLE                    MinHook_Enable;
	MINHOOK_DISABLE                   MinHook_Disable;
	EVENTS_RAISE                      Events_Raise;
	EVENTS_RAISENOTIFICATION          Events_RaiseNotification;
	EVENTS_RAISE_TARGETED             Events_RaiseTargeted;
	EVENTS_RAISENOTIFICATION_TARGETED Events_RaiseNotificationTargeted;
	EVENTS_SUBSCRIBE                  Events_Subscribe;
	EVENTS_SUBSCRIBE                  Events_Unsubscribe;
	// Additional fields not needed for this addon
} AddonAPI;

#endif
