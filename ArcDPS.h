///----------------------------------------------------------------------------------------------------
/// ArcDPS API structures for Nexus combat events
///----------------------------------------------------------------------------------------------------

#ifndef ARCDPS_H
#define ARCDPS_H

#include <stdint.h>

namespace ArcDPS
{
	/* Combat event structure */
	struct CombatEvent
	{
		uint64_t Time;
		uint64_t SourceAgent;
		uint64_t DestinationAgent;
		int32_t  Value;
		int32_t  BuffDamage;
		uint32_t OverstackValue;
		uint32_t SkillID;
		uint16_t SourceInstanceID;
		uint16_t DestinationInstanceID;
		uint16_t SrcMasterInstanceID;
		uint16_t DestinationMasterInstanceID;
		uint8_t  IFF;
		uint8_t  Buff;
		uint8_t  Result;
		uint8_t  IsActivation;
		uint8_t  IsBuffRemove;
		uint8_t  IsNinety;
		uint8_t  IsFifty;
		uint8_t  IsMoving;
		uint8_t  IsStatechange;
		uint8_t  IsFlanking;
		uint8_t  IsShields;
		uint8_t  IsOffcycle;
		uint8_t  PAD61;
		uint8_t  PAD62;
		uint8_t  PAD63;
		uint8_t  PAD64;
	};

	/* Agent short structure */
	struct AgentShort
	{
		char*     Name;
		uintptr_t ID;
		uint32_t  Profession;
		uint32_t  Specialization;
		uint32_t  IsSelf;
		uint16_t  Team;
	};

	/* Combat state change types */
	enum ECombatStateChange
	{
		CBTS_NONE,
		CBTS_ENTERCOMBAT,
		CBTS_EXITCOMBAT,
		CBTS_CHANGEUP,
		CBTS_CHANGEDEAD,
		CBTS_CHANGEDOWN,
		CBTS_SPAWN,
		CBTS_DESPAWN,
		CBTS_HEALTHPCTUPDATE,
		CBTS_SQCOMBATSTART,
		CBTS_LOGEND,
		CBTS_WEAPSWAP,
		CBTS_MAXHEALTHUPDATE,
		CBTS_POINTOFVIEW,
		CBTS_LANGUAGE,
		CBTS_GWBUILD,
		CBTS_SHARDID,
		CBTS_REWARD,
		CBTS_BUFFINITIAL,
		CBTS_POSITION,
		CBTS_VELOCITY,
		CBTS_FACING,
		CBTS_TEAMCHANGE,
		CBTS_ATTACKTARGET,
		CBTS_TARGETABLE,
		CBTS_MAPID,
		CBTS_REPLINFO,
		CBTS_STACKACTIVE,
		CBTS_STACKRESET,
		CBTS_GUILD,
		CBTS_BUFFINFO,
		CBTS_BUFFFORMULA,
		CBTS_SKILLINFO,
		CBTS_SKILLTIMING,
		CBTS_BREAKBARSTATE,
		CBTS_BREAKBARPERCENT,
		CBTS_INTEGRITY,
		CBTS_MARKER,
		CBTS_BARRIERPCTUPDATE,
		CBTS_STATRESET,
		CBTS_EXTENSION,
		CBTS_APIDELAYED,
		CBTS_INSTANCESTART,
		CBTS_RATEHEALTH,
		CBTS_LAST90BEFOREDOWN,
		CBTS_EFFECT,
		CBTS_IDTOGUID,
		CBTS_LOGNPCUPDATE,
		CBTS_IDLEEVENT,
		CBTS_EXTENSIONCOMBAT,
		CBTS_FRACTALSCALE,
		CBTS_EFFECT2,
		CBTS_RULESET,
		CBTS_SQUADMARKER,
		CBTS_ARCBUILD,
		CBTS_GLIDER,
		CBTS_STUNBREAK,
		CBTS_UNKNOWN
	};

	/* Is Friend/Foe */
	enum EIsFriendFoe
	{
		IFF_FRIEND,
		IFF_FOE,
		IFF_UNKNOWN
	};

	/* Combat result types */
	enum ECombatResult
	{
		CBTR_NORMAL,
		CBTR_CRIT,
		CBTR_GLANCE,
		CBTR_BLOCK,
		CBTR_EVADE,
		CBTR_INTERRUPT,
		CBTR_ABSORB,
		CBTR_BLIND,
		CBTR_KILLINGBLOW,
		CBTR_DOWNED,
		CBTR_BREAKBAR,
		CBTR_ACTIVATION,
		CBTR_CROWDCONTROL,
		CBTR_UNKNOWN
	};
}

/* Nexus combat event payload structure */
struct EvCombatData
{
	ArcDPS::CombatEvent* ev;
	ArcDPS::AgentShort* src;
	ArcDPS::AgentShort* dst;
	char* skillname;
	uint64_t id;
	uint64_t revision;
};

#endif
