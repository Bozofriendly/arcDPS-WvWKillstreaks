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

/* DataLink identifiers */
#define DL_NEXUS_LINK "DL_NEXUS_LINK"
#ifndef DL_MUMBLE_LINK
#define DL_MUMBLE_LINK "DL_MUMBLE_LINK"
#endif
#ifndef DL_MUMBLE_LINK_IDENTITY
#define DL_MUMBLE_LINK_IDENTITY "DL_MUMBLE_LINK_IDENTITY"
#endif

/* Event identifiers */
#define EV_WINDOW_RESIZED          "EV_WINDOW_RESIZED"
#define EV_MUMBLE_IDENTITY_UPDATED "EV_MUMBLE_IDENTITY_UPDATED"
#define EV_ADDON_LOADED            "EV_ADDON_LOADED"
#define EV_ADDON_UNLOADED          "EV_ADDON_UNLOADED"

/* ArcDPS Event identifiers */
#define EV_ARCDPS_COMBATEVENT_LOCAL_RAW "EV_ARCDPS_COMBATEVENT_LOCAL_RAW"
#define EV_ARCDPS_COMBATEVENT_SQUAD_RAW "EV_ARCDPS_COMBATEVENT_SQUAD_RAW"

///----------------------------------------------------------------------------------------------------
/// ERenderType Enumeration
///----------------------------------------------------------------------------------------------------
typedef enum ERenderType
{
	ERenderType_PreRender,
	ERenderType_Render,
	ERenderType_PostRender,
	ERenderType_OptionsRender
} ERenderType;

///----------------------------------------------------------------------------------------------------
/// ELogLevel Enumeration
///----------------------------------------------------------------------------------------------------
typedef enum ELogLevel
{
	ELogLevel_CRITICAL = 1,
	ELogLevel_WARNING = 2,
	ELogLevel_INFO = 3,
	ELogLevel_DEBUG = 4,
	ELogLevel_TRACE = 5
} ELogLevel;

///----------------------------------------------------------------------------------------------------
/// EAddonFlags Enumeration
///----------------------------------------------------------------------------------------------------
typedef enum EAddonFlags
{
	EAddonFlags_None              = 0,
	EAddonFlags_IsVolatile        = 1 << 0,
	EAddonFlags_DisableHotloading = 1 << 1,
	EAddonFlags_LaunchOnly        = 1 << 2
} EAddonFlags;

///----------------------------------------------------------------------------------------------------
/// EUpdateProvider Enumeration
///----------------------------------------------------------------------------------------------------
typedef enum EUpdateProvider
{
	EUpdateProvider_None     = 0,
	EUpdateProvider_Raidcore = 1,
	EUpdateProvider_GitHub   = 2,
	EUpdateProvider_Direct   = 3,
	EUpdateProvider_Self     = 4
} EUpdateProvider;

typedef struct Keybind
{
	uint16_t Key;
	bool     Alt;
	bool     Ctrl;
	bool     Shift;
} Keybind;

typedef struct Texture
{
	uint32_t Width;
	uint32_t Height;
	void*    Resource;
} Texture;

typedef struct NexusLinkData
{
	uint32_t Width;
	uint32_t Height;
	float    Scaling;

	bool     IsMoving;
	bool     IsCameraMoving;
	bool     IsGameplay;

	void*    Font;
	void*    FontBig;
	void*    FontUI;
} NexusLinkData;

typedef struct AddonVersion
{
	uint16_t Major;
	uint16_t Minor;
	uint16_t Build;
	uint16_t Revision;
} AddonVersion;

struct AddonAPI;

typedef void        (*ADDON_LOAD)                       (AddonAPI* aAPI);
typedef void        (*ADDON_UNLOAD)                     ();

typedef void        (*GUI_RENDER)                       ();
typedef void        (*GUI_ADDRENDER)                    (ERenderType aRenderType, GUI_RENDER aRenderCallback);
typedef void        (*GUI_REMRENDER)                    (GUI_RENDER aRenderCallback);

typedef const char* (*PATHS_GETGAMEDIR)                 ();
typedef const char* (*PATHS_GETADDONDIR)                (const char* aName);
typedef const char* (*PATHS_GETCOMMONDIR)               ();

typedef void        (*LOGGER_LOG)                       (ELogLevel aLogLevel, const char* aChannel, const char* aStr);

typedef void        (*ALERTS_NOTIFY)                    (const char* aMessage);

typedef void        (*EVENT_CONSUME)                    (void* aEventArgs);
typedef void        (*EVENTS_RAISE)                     (const char* aIdentifier, void* aEventData);
typedef void        (*EVENTS_RAISENOTIFICATION)         (const char* aIdentifier);
typedef void        (*EVENTS_SUBSCRIBE)                 (const char* aIdentifier, EVENT_CONSUME aConsumeEventCallback);

typedef void*       (*DATALINK_GET)                     (const char* aIdentifier);
typedef void*       (*DATALINK_SHARE)                   (const char* aIdentifier, uint64_t aResourceSize);

typedef struct AddonDefinition
{
	/* required */
	uint32_t        Signature;
	uint32_t        APIVersion;
	const char*     Name;
	AddonVersion    Version;
	const char*     Author;
	const char*     Description;
	ADDON_LOAD      Load;
	ADDON_UNLOAD    Unload;
	EAddonFlags     Flags;

	/* update fallback */
	EUpdateProvider Provider;
	const char*     UpdateLink;
} AddonDefinition;

typedef struct AddonAPI
{
	/* Renderer */
	void* SwapChain;
	void* ImguiContext;
	void* ImguiMalloc;
	void* ImguiFree;

	struct {
		GUI_ADDRENDER Register;
		GUI_REMRENDER Deregister;
	} Renderer;

	/* Logging */
	LOGGER_LOG Log;

	/* Alerts */
	ALERTS_NOTIFY GUI_SendAlert;

	/* Paths */
	struct {
		PATHS_GETGAMEDIR  GetGameDirectory;
		PATHS_GETADDONDIR GetAddonDirectory;
		PATHS_GETCOMMONDIR GetCommonDirectory;
	} Paths;

	/* Events */
	struct {
		EVENTS_RAISE             Raise;
		EVENTS_RAISENOTIFICATION RaiseNotification;
		EVENTS_SUBSCRIBE         Subscribe;
		EVENTS_SUBSCRIBE         Unsubscribe;
	} Events;

	/* DataLink */
	struct {
		DATALINK_GET   Get;
		DATALINK_SHARE Share;
	} DataLink;
} AddonAPI;

#endif
