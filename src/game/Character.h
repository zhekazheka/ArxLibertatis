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

#pragma once

#ifndef ARX_GAME_CHARACTER_H
#define ARX_GAME_CHARACTER_H

#include <string>
#include <vector>

#include "graphics/data/Mesh.h"
#include "math/MathFwd.h"
#include "game/Spells.h"
#include "game/Keyring.h"
#include "game/Runes.h"
#include "game/Necklace.h"
#include "game/Playerflags.h"
#include "game/Quest.h"

namespace arx
{
	template <class T>
	T limit(const T &v, const T &l, const T &h) { return (v < l ? l : v > h ? h : v); }

	class character 
	{
	public:

		static const float STEP_DISTANCE;
		static const float SKILL_STEALTH_MAX;

		struct attributes
		{
			float &operator[](const int &i) { return val[i]; }

			void operator+=(const float &v)
			{
				for (int i = 0; i < 4; i++)
				{
					val[i] += v;
				}
			}

			void operator=(const float &v)
			{
				for (int i = 0; i < 4; i++)
				{
					val[i] = v;
				}
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

				float val[4];
			};
		};

		struct skills
		{
			float &operator[](const int &i) { return val[i]; }

			void operator+=(const float &v)
			{
				for (int i = 0; i < 9; i++)
				{
					val[i] += v;
				}
			}

			void operator=(const float &v)
			{
				for (int i = 0; i < 9; i++)
				{
					val[i] = v;
				}
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

				float val[9];
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

			void limit_life() { life = arx::limit(life, 0.0f, maxlife); }
			void limit_mana() { mana = arx::limit(mana, 0.0f, maxmana); }
			void limit() { limit_life(); limit_mana(); }

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

		character();
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

		float get_stealth_for_color() const;
		float get_stealth(bool modified = false) const;
		float get_mecanism(bool modified = false) const;
		float get_intuition(bool modified = false) const;
		float get_etheral_link(bool modified = false) const;
		float get_object_knowledge(bool modified = false) const;
		float get_casting(bool modified = false) const;
		float get_projectile(bool modified = false) const;
		float get_defense(bool modified = false) const;
		float get_close_combat(bool modified = false) const;

		void set_invulnerable(const bool &b = true);
		bool can_steal(INTERACTIVE_OBJ *_io);

		void frame_check(const float &frame_delta);

		void start_fall();
		void reset_fall();

		void manage_visual();
		void manage_movement();
		void frame_update();

		void get_front_pos(Vec3f &pos) const;
		void look_at(INTERACTIVE_OBJ *io);
		void make_step_noise();

		void torch_kill();
		void torch_clicked(INTERACTIVE_OBJ *io);
		void torch_manage();

		//
		unsigned char level;
		long xp;

		attributes attribute;
		skills skill;
		stats stat;
		
		skills old;

		mod_stats mod;
		full_stats full;

		float critical_hit;

		unsigned char armor_class;
		unsigned char resist_magic;
		unsigned char resist_poison;

		long gold;
		long aimtime;
		long weapon_type;

		purchase_points redistribute;

		Vec3f pos;
		Vec3f PUSH_PLAYER_FORCE;

		Anglef angle;
		ANIM_USE useanim;
		IO_PHYSICS physics;
		
		std::vector<STRUCT_QUEST> quest;

		// Jump Sub-data
		unsigned long jumpstarttime;
		float jumplastposition;
		long jumpphase; //!< 0 no jump, 1 doing anticipation anim, 2 moving_up, 3 moving_down, 4 finish_anim

		Anglef desiredangle;
		Vec3f size;
		void *inzone;

		long falling;
		
		PlayerMovement Current_Movement;
		PlayerMovement Last_Movement;
		long onfirmground;
		
		INTERACTIVE_OBJ *rightIO;
		INTERACTIVE_OBJ *leftIO;
		INTERACTIVE_OBJ *equipsecondaryIO;
		INTERACTIVE_OBJ *equipshieldIO;
		INTERACTIVE_OBJ *CURRENT_TORCH;

		RuneFlags rune_flags;
		TextureContainer *heads[5];
		PlayerFlags playerflags;
		ARX_INTERFACE_MEMORIZE_SPELL SpellToMemorize;

		arx::keyring keyring;
		arx::necklace necklace;

		float CURRENT_PLAYER_COLOR;
		float PLAYER_ROTATION;
		float LASTPLAYERA;
		float lastposy;
		float Falling_Height;

		float damages;
		float poison;
		float hunger;
		float grnd_color;

		float currentdistance;
		float Full_Jump_Height;
		float DeadCameraDistance;

		unsigned long ROTATE_START;
		unsigned long LAST_JUMP_ENDTIME;
		unsigned long REQUEST_JUMP;
		unsigned long LastHungerSample;
		unsigned long FALLING_TIME;
		long LAST_VECT_COUNT;
		long JUMP_DIVIDE;
		long DeadTime;
		long FistParticles;
		long sp_max;

		short equiped[MAX_EQUIPED]; 
		short bag;
		short doingmagic;
		short Interface;
		short climbing;
		short levitate;

		char skin;
		char SKIN_MOD;
		char QUICK_MOD;

		bool PLAYER_PARALYSED;
		bool USE_PLAYERCOLLISIONS;
		bool BLOCK_PLAYER_CONTROLS;
		bool WILLRETURNTOCOMBATMODE;
		bool LAST_ON_PLATFORM;
		bool LAST_FIRM_GROUND;
		bool TRUE_FIRM_GROUND;
		bool DISABLE_JUMP;
		bool STARTED_A_GAME;

	private:
			
		void do_physics(const float &DeltaTime);
		bool valid_jump_pos();
	};
};

extern arx::character player;

#endif
