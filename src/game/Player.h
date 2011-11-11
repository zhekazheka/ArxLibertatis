/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_GAME_PLAYER_H
#define ARX_GAME_PLAYER_H

#include <string>
#include <vector>

#include "game/Spells.h"
#include "graphics/data/Mesh.h"
#include "math/MathFwd.h"
#include "platform/Flags.h"

struct EERIE_3DOBJ;
class TextureContainer;

struct ARX_NECKLACE {
	EERIE_3DOBJ * lacet;
	EERIE_3DOBJ * runes[RUNE_COUNT];
	TextureContainer * pTexTab[RUNE_COUNT];
};

struct ARX_INTERFACE_MEMORIZE_SPELL {
	bool bSpell;
	unsigned long lTimeCreation;
	unsigned long lDuration;
	Rune iSpellSymbols[6];
	float fPosX;
	float fPosY;
};

enum PlayerMovementFlag {
	PLAYER_MOVE_WALK_FORWARD  = (1<<0),
	PLAYER_MOVE_WALK_BACKWARD = (1<<1),
	PLAYER_MOVE_STRAFE_LEFT   = (1<<2),
	PLAYER_MOVE_STRAFE_RIGHT  = (1<<3),
	PLAYER_MOVE_JUMP          = (1<<4),
	PLAYER_MOVE_STEALTH       = (1<<5),
	PLAYER_ROTATE             = (1<<6),
	PLAYER_CROUCH             = (1<<7),
	PLAYER_LEAN_LEFT          = (1<<8),
	PLAYER_LEAN_RIGHT         = (1<<9)
};
DECLARE_FLAGS(PlayerMovementFlag, PlayerMovement)
DECLARE_FLAGS_OPERATORS(PlayerMovement)

enum PlayerFlag {
	PLAYERFLAGS_NO_MANA_DRAIN   = (1<<0),
	PLAYERFLAGS_INVULNERABILITY = (1<<1)
};
DECLARE_FLAGS(PlayerFlag, PlayerFlags)
DECLARE_FLAGS_OPERATORS(PlayerFlags)

enum RuneFlag {
	FLAG_AAM         = (1<<(RUNE_AAM)),
	FLAG_CETRIUS     = (1<<(RUNE_CETRIUS)),
	FLAG_COMUNICATUM = (1<<(RUNE_COMUNICATUM)),
	FLAG_COSUM       = (1<<(RUNE_COSUM)),
	FLAG_FOLGORA     = (1<<(RUNE_FOLGORA)),
	FLAG_FRIDD       = (1<<(RUNE_FRIDD)),
	FLAG_KAOM        = (1<<(RUNE_KAOM)),
	FLAG_MEGA        = (1<<(RUNE_MEGA)),
	FLAG_MORTE       = (1<<(RUNE_MORTE)),
	FLAG_MOVIS       = (1<<(RUNE_MOVIS)),
	FLAG_NHI         = (1<<(RUNE_NHI)),
	FLAG_RHAA        = (1<<(RUNE_RHAA)),
	FLAG_SPACIUM     = (1<<(RUNE_SPACIUM)),
	FLAG_STREGUM     = (1<<(RUNE_STREGUM)),
	FLAG_TAAR        = (1<<(RUNE_TAAR)),
	FLAG_TEMPUS      = (1<<(RUNE_TEMPUS)),
	FLAG_TERA        = (1<<(RUNE_TERA)),
	FLAG_VISTA       = (1<<(RUNE_VISTA)),
	FLAG_VITAE       = (1<<(RUNE_VITAE)),
	FLAG_YOK         = (1<<(RUNE_YOK))
};
DECLARE_FLAGS(RuneFlag, RuneFlags)
DECLARE_FLAGS_OPERATORS(RuneFlags)

namespace arx
{
	class character 
	{
	public:

		struct attributes
		{
			void operator+=(const float &v)
			{
				strength += v;
				dexterity += v;
				constitution += v;
				mind += v;
			}

			void operator=(const float &v)
			{
				strength = v;
				dexterity = v;
				constitution = v;
				mind = v;
			}

			union 
			{
				struct 
				{
					float strength;
					float dexterity;
					float constitution;
					float mind;
				};

				float v[4];
			};
		};

		struct skills
		{
			void operator+=(const float &v)
			{
				stealth += v;
				mecanism += v;
				intuition += v;
				etheral_link += v;
				object_knowledge += v;
				casting += v;
				projectile += v;
				close_combat += v;
				defense += v;
			}

			void operator=(const float &v)
			{
				stealth = v;
				mecanism = v;
				intuition = v;
				etheral_link = v;
				object_knowledge = v;
				casting = v;
				projectile = v;
				close_combat = v;
				defense = v;
			}

			union
			{
				struct 
				{
					float stealth;
					float mecanism;
					float intuition;
					float etheral_link;
					float object_knowledge;
					float casting;
					float projectile;
					float close_combat;
					float defense;
				};

				float v[9];
			};
		};

		struct stats
		{
			void operator=(const float &v)
			{
				life = v;
				maxlife = v;
				mana = v;
				maxmana = v;
			}

			float life;
			float maxlife;

			float mana;
			float maxmana;
		};

		// Modifier Values (Items, curses, etc...)
		struct mod_stats
		{
			attributes attribute;
			skills skill;
			stats stat;

			float armor_class;
			float resist_magic;
			float resist_poison;
			float critical_hit;
			float damages;
		};

		// Full Frame values (including items)
		struct full_stats
		{
			attributes attribute;
			skills skill;
			stats stat;

			float aimtime;
			float armor_class;
			float resist_magic;
			float resist_poison;
			float critical_hit;
			float damages;
			long weapon_type;
		};

		struct purchase_points
		{
			unsigned char attribute;
			unsigned char skill;
		};

		character() {}
		~character() {}

		static int get_xp_for_level(const int &level);

		void init();

		void hero_generate_fresh();
		void hero_generate_average();
		void hero_generate_powerful();
		void hero_generate_sp();
		void hero_generate_random();

		void compute_stats();
		void compute_full_stats();

		void add_xp(const int &v);
		void level_up();

		void add_poison(const float &v);
		void add_gold(const int &v);
		void add_gold(INTERACTIVE_OBJ *gold);
		void add_bag();

		void add_rune(RuneFlag _ulRune);
		void remove_rune(RuneFlag _ulRune);
		void add_all_runes();

		void remove_invisibility();

		float get_stealth(bool modified = false);
		float get_mecanism(bool modified = false);
		float get_intuition(bool modified = false);
		float get_etheral_link(bool modified = false);
		float get_object_knowledge(bool modified = false);
		float get_casting(bool modified = false);
		float get_projectile(bool modified = false);
		float get_defense(bool modified = false);
		float get_close_combat(bool modified = false);

		void set_invulnerable(const bool &b = true);
		bool can_steal(INTERACTIVE_OBJ *_io);

		unsigned char level;
		long xp;

		attributes attribute;
		skills skill;
		stats stat;
		
		float critical_hit;

		unsigned char armor_class;
		unsigned char resist_magic;
		unsigned char resist_poison;

		long gold;
		long aimtime;
		long weapon_type;

		skills old;

		mod_stats mod;
		full_stats full;

		purchase_points redistribute;

		char skin;
		
		Vec3f pos;
		Anglef angle;
		ANIM_USE useanim;
		IO_PHYSICS physics;
		
		// Jump Sub-data
		unsigned long jumpstarttime;
		float jumplastposition;
		long jumpphase; //!< 0 no jump, 1 doing anticipation anim, 2 moving_up, 3 moving_down, 4 finish_anim
		
		short climbing;
		short levitate;
		
		Anglef desiredangle;
		Vec3f size;
		void * inzone;

		long falling;
		short doingmagic;
		short Interface;
		
		PlayerMovement Current_Movement;
		PlayerMovement Last_Movement;
		long onfirmground;
		
		INTERACTIVE_OBJ *rightIO;
		INTERACTIVE_OBJ *leftIO;
		INTERACTIVE_OBJ *equipsecondaryIO;
		INTERACTIVE_OBJ *equipshieldIO;
		
		short equiped[MAX_EQUIPED]; 

		RuneFlags rune_flags;
		TextureContainer * heads[5];
		float damages;
		float poison;
		float hunger;
		float grnd_color;
		PlayerFlags playerflags;
		short bag;
		ARX_INTERFACE_MEMORIZE_SPELL SpellToMemorize;
	};
};

typedef arx::character ARXCHARACTER;

struct KEYRING_SLOT {
	char slot[64];
};

// Quests Management (QuestLogBook)

struct STRUCT_QUEST {
	std::string ident;
	std::string localised;
};

extern ARXCHARACTER player;
extern ARX_NECKLACE necklace;
extern EERIE_3DOBJ * hero;
extern ANIM_HANDLE * herowaitbook;
extern ANIM_HANDLE * herowait2;
extern ANIM_HANDLE * herowait_2h;
extern std::vector<STRUCT_QUEST> PlayerQuest;
extern std::vector<KEYRING_SLOT> Keyring;

extern float DeadCameraDistance;
extern long BLOCK_PLAYER_CONTROLS;
extern long USE_PLAYERCOLLISIONS;
extern long WILLRETURNTOCOMBATMODE;
extern float PLAYER_BASE_RADIUS;
extern float PLAYER_BASE_HEIGHT;

void ARX_PLAYER_LoadHeroAnimsAndMesh();

void ARX_PLAYER_BecomesDead();

void ARX_PLAYER_ClickedOnTorch(INTERACTIVE_OBJ * io);

void ARX_PLAYER_RectifyPosition();

void ARX_PLAYER_Frame_Update();

void ARX_PLAYER_Manage_Movement();
void ARX_PLAYER_Manage_Death();

void ARX_PLAYER_GotoAnyPoly();

void ARX_PLAYER_Quest_Add(const std::string & quest, bool _bLoad = false);
void ARX_PLAYER_Quest_Init();

void ARX_PLAYER_FrontPos(Vec3f * pos);

void ARX_PLAYER_FrameCheck(float _framedelay);

void ARX_PLAYER_Manage_Visual();

void ARX_KEYRING_Init();
void ARX_KEYRING_Add(const std::string & key);
void ARX_KEYRING_Combine(INTERACTIVE_OBJ * io);

void ARX_PLAYER_Reset_Fall();

void ARX_PLAYER_KillTorch();

void ARX_PLAYER_PutPlayerInNormalStance(long val);

void ARX_PLAYER_Start_New_Quest();
 
void ARX_PLAYER_Restore_Skin();

float GetPlayerStealth();

void ARX_GAME_Reset(long type = 0);
void Manage_sp_max();

bool ARX_PLAYER_IsInFightMode();

void ForcePlayerLookAtIO(INTERACTIVE_OBJ * io);

#endif // ARX_GAME_PLAYER_H