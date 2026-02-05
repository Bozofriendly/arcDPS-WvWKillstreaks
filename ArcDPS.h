///----------------------------------------------------------------------------------------------------
/// ArcDPS API structures for Nexus combat events
/// Based on official Raidcore RCGG-lib-arcdps-api
///----------------------------------------------------------------------------------------------------

#ifndef ARCDPS_H
#define ARCDPS_H

#include <stdint.h>

namespace ArcDPS
{
	/* Combat event structure - see evtc docs for details */
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
		char*     Name;           /* agent name. may be null. valid only at time of event. utf8 */
		uintptr_t ID;             /* agent unique identifier */
		uint32_t  Profession;     /* profession at time of event */
		uint32_t  Specialization; /* elite spec at time of event */
		uint32_t  IsSelf;         /* 1 if self, 0 if not */
		uint16_t  Team;           /* team id */
	};

	/* Combat state change types */
	enum ECombatStateChange
	{
		CBTS_NONE,             // not used - not this kind of event
		CBTS_ENTERCOMBAT,      // src_agent entered combat
		CBTS_EXITCOMBAT,       // src_agent left combat
		CBTS_CHANGEUP,         // src_agent is alive
		CBTS_CHANGEDEAD,       // src_agent is dead
		CBTS_CHANGEDOWN,       // src_agent is downed
		CBTS_SPAWN,            // src_agent entered tracking
		CBTS_DESPAWN,          // src_agent left tracking
		CBTS_HEALTHPCTUPDATE,  // src_agent health percent changed
		CBTS_SQCOMBATSTART,    // squad combat start (log start)
		CBTS_LOGEND,           // squad combat stop (log end)
		CBTS_WEAPSWAP,         // src_agent weapon set changed
		CBTS_MAXHEALTHUPDATE,  // src_agent max health changed
		CBTS_POINTOFVIEW,      // recording player
		CBTS_LANGUAGE,         // text language id
		CBTS_GWBUILD,          // game build number
		CBTS_SHARDID,          // server shard id
		CBTS_REWARD,           // wiggly box reward
		CBTS_BUFFINITIAL,      // buff application for existing buffs
		CBTS_POSITION,         // src_agent position changed
		CBTS_VELOCITY,         // src_agent velocity changed
		CBTS_FACING,           // src_agent facing changed
		CBTS_TEAMCHANGE,       // src_agent team changed
		CBTS_ATTACKTARGET,     // attacktarget to gadget association
		CBTS_TARGETABLE,       // src_agent targetable state
		CBTS_MAPID,            // map id (src_agent = map id)
		CBTS_REPLINFO,         // internal use
		CBTS_STACKACTIVE,      // buff instance active
		CBTS_STACKRESET,       // buff instance duration changed
		CBTS_GUILD,            // src_agent is member of guild
		CBTS_BUFFINFO,         // buff information
		CBTS_BUFFFORMULA,      // buff formula
		CBTS_SKILLINFO,        // skill information
		CBTS_SKILLTIMING,      // skill timing
		CBTS_BREAKBARSTATE,    // breakbar state
		CBTS_BREAKBARPERCENT,  // breakbar percent
		CBTS_INTEGRITY,        // internal use
		CBTS_MARKER,           // src_agent has marker
		CBTS_BARRIERPCTUPDATE, // barrier percent update
		CBTS_STATRESET,        // stat reset
		CBTS_EXTENSION,        // extension event
		CBTS_APIDELAYED,       // api delayed
		CBTS_INSTANCESTART,    // instance start
		CBTS_RATEHEALTH,       // rate health
		CBTS_LAST90BEFOREDOWN, // last 90% before down
		CBTS_EFFECT,           // effect
		CBTS_IDTOGUID,         // id to guid
		CBTS_LOGNPCUPDATE,     // log npc update
		CBTS_IDLEEVENT,        // idle event
		CBTS_EXTENSIONCOMBAT,  // extension combat
		CBTS_FRACTALSCALE,     // fractal scale
		CBTS_EFFECT2,          // effect v2
		CBTS_RULESET,          // ruleset
		CBTS_SQUADMARKER,      // squad marker
		CBTS_ARCBUILD,         // arc build
		CBTS_GLIDER,           // glider
		CBTS_STUNBREAK,        // stun break
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
		CBTR_NORMAL,       // 0: strike was neither crit or glance
		CBTR_CRIT,         // 1: strike was crit
		CBTR_GLANCE,       // 2: strike was glance
		CBTR_BLOCK,        // 3: strike was blocked
		CBTR_EVADE,        // 4: strike was evaded
		CBTR_INTERRUPT,    // 5: strike interrupted something
		CBTR_ABSORB,       // 6: strike was absorbed/invulned
		CBTR_BLIND,        // 7: strike missed
		CBTR_KILLINGBLOW,  // 8: not a damage strike, target was killed by skill
		CBTR_DOWNED,       // 9: not a damage strike, target was downed by skill
		CBTR_BREAKBAR,     // 10: breakbar damage dealt
		CBTR_ACTIVATION,   // 11: skill activation event
		CBTR_CROWDCONTROL, // 12: target was crowd controlled
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
