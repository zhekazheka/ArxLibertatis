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

#ifndef ARX_GAME_CHARACTER_H
#define ARX_GAME_CHARACTER_H

#include <string>
#include <vector>

#include "game/Spells.h"
#include "game/Keyring.h"

#include "graphics/data/Mesh.h"
#include "math/MathFwd.h"
#include "platform/Flags.h"

#include "game/Runes.h"
#include "game/Playerflags.h"

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
		Vec3f PUSH_PLAYER_FORCE;

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

		char SKIN_MOD;
		char QUICK_MOD;

		keyring keyring;
	};
};

typedef arx::character ARXCHARACTER;
extern ARXCHARACTER player;

#endif
