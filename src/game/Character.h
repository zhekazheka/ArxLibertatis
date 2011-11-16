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

#include "game/Attributes.h"
#include "game/Keyring.h"
#include "game/Necklace.h"
#include "game/Playerflags.h"
#include "game/Quest.h"
#include "game/Runes.h"
#include "game/Skills.h"
#include "game/Spells.h"
#include "game/Status.h"
#include "graphics/data/Mesh.h"
#include "math/MathFwd.h"

namespace arx
{
	class character
	{
	public:

		static const float STEP_DISTANCE;
		static const float SKILL_STEALTH_MAX;

		character();

		~character() 
		{
		}

		void init();

		void hero_generate_fresh();
		void hero_generate_average();
		void hero_generate_powerful();
		void hero_generate_sp();
		void hero_generate_random();

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

		void compute_stats();
		void compute_full_stats();

		static int get_xp_for_level(const int &level);
		void add_xp(const int &v);
		void level_up();

		void add_rune(RuneFlag _ulRune);
		void remove_rune(RuneFlag _ulRune);
		void add_all_runes();

		void add_bag();
		void add_gold(const int &v);
		void add_gold(INTERACTIVE_OBJ *gold);

		void add_poison(const float &v);

		void remove_invisibility();
		void set_invulnerable(const bool &b = true);

		bool can_steal(INTERACTIVE_OBJ *_io);

		void start_fall();
		void reset_fall();

		void frame_check(const float &frame_delta);

		void manage_visual();
		void manage_movement();
		void frame_update();

		void get_front_pos(Vec3f &pos) const;
		void look_at(INTERACTIVE_OBJ *io);

		void make_step_noise();

		void torch_kill();
		void torch_clicked(INTERACTIVE_OBJ *io);
		void torch_manage();

		// experience

		long xp;

		unsigned char level;

		// attributes / skills / status

		struct purchase_points
		{
			unsigned char attribute;
			unsigned char skill;
		};

		purchase_points redistribute;

		attributes attribute;
		skills skill;
		stats stat;

		skills old;

		/// modifiers (equipment, spells, ...)
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

		mod_stats mod;
		
		/// full values (including modifiers)
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

		full_stats full;

		float critical_hit;

		long aimtime;

		unsigned char armor_class;
		unsigned char resist_magic;
		unsigned char resist_poison;

		float damages;
		float poison;
		float hunger;

		// physics / animation / movement

		IO_PHYSICS physics;

		ANIM_USE useanim;

		Anglef angle;
		Anglef desiredangle;

		Vec3f pos;
		Vec3f PUSH_PLAYER_FORCE;
		Vec3f size;

		PlayerMovement Current_Movement;
		PlayerMovement Last_Movement;

		struct jump_data
		{
			enum jump_phase
			{
				none = 0,
				anticipation,
				moving_up,
				moving_down,
				finished,
				post_sync,
			};

			float last_position;
			unsigned long start_time;
			jump_phase phase;
		};

		jump_data jump;

		float PLAYER_ROTATION;
		float LASTPLAYERA;
		float lastposy;
		float Falling_Height;
		float currentdistance;
		float Full_Jump_Height;
		float DeadCameraDistance;

		unsigned long ROTATE_START;
		unsigned long LAST_JUMP_ENDTIME;
		unsigned long REQUEST_JUMP;
		unsigned long FALLING_TIME;

		long JUMP_DIVIDE;
		long falling;
		long onfirmground;

		short doingmagic;
		short climbing;
		short levitate;

		bool LAST_ON_PLATFORM;
		bool LAST_FIRM_GROUND;
		bool TRUE_FIRM_GROUND;
		bool PLAYER_PARALYSED;
		bool USE_PLAYERCOLLISIONS;
		bool BLOCK_PLAYER_CONTROLS;
		bool DISABLE_JUMP;

		// component objects

		arx::keyring keyring;
		arx::necklace necklace;

		/// TODO rune_flags -> necklace
		RuneFlags rune_flags;

		/// TODO quest -> class
		std::vector<STRUCT_QUEST> quest;

		// equipment
		INTERACTIVE_OBJ *rightIO;
		INTERACTIVE_OBJ *leftIO;
		INTERACTIVE_OBJ *equipsecondaryIO;
		INTERACTIVE_OBJ *equipshieldIO;
		INTERACTIVE_OBJ *CURRENT_TORCH;

		long weapon_type;

		short equiped[MAX_EQUIPED];

		// inventory
		long gold;

		short bag;

		// ungrouped / unknown
		void *inzone;

		TextureContainer *heads[5];
		ARX_INTERFACE_MEMORIZE_SPELL SpellToMemorize;
		PlayerFlags playerflags;

		float CURRENT_PLAYER_COLOR;
		float grnd_color;

		unsigned long LastHungerSample;

		long LAST_VECT_COUNT;
		long DeadTime;
		long FistParticles;
		long sp_max;

		short Interface;

		char skin;
		char SKIN_MOD;
		char QUICK_MOD;

		bool WILLRETURNTOCOMBATMODE;
		bool STARTED_A_GAME;

	private:

		void do_physics(const float &delta);
		bool valid_jump_pos();
	};
};

extern arx::character player;

#endif // ARX_GAME_CHARACTER_H
