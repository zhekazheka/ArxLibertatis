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

#include "game/Character.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Inventory.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "graphics/particle/ParticleEffects.h"
#include "gui/Interface.h"
#include "gui/Speech.h"
#include "physics/Attractors.h"
#include "physics/Collisions.h"
#include "scene/Interactive.h"

// externs
extern float InventoryX;
extern float InventoryDir;
extern long COLLIDED_CLIMB_POLY;
extern long cur_mr;
extern long cur_rf;
extern bool bGCroucheToggle;
extern float MAX_ALLOWED_PER_SECOND;
extern long ON_PLATFORM;

extern long APPLY_PUSH;

// Emit player step noise
void arx::character::make_step_noise()
{
	if (ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_LEVITATE) >= 0)
	{
		return;
	}

	if (USE_PLAYERCOLLISIONS)
	{
		float volume = ARX_NPC_AUDIBLE_VOLUME_DEFAULT;
		float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT;

		if (Current_Movement & PLAYER_MOVE_STEALTH)
		{
			const float skill_stealth = full.skill.stealth / SKILL_STEALTH_MAX;
			volume -= ARX_NPC_AUDIBLE_VOLUME_RANGE * skill_stealth;
			factor += ARX_NPC_AUDIBLE_FACTOR_RANGE * skill_stealth;
		}

		Vec3f _pos = pos - Vec3f(0, -PLAYER_BASE_HEIGHT, 0);

		ARX_NPC_NeedStepSound(inter.iobj[0], &_pos, volume, factor);
	}

	if (currentdistance >= STEP_DISTANCE)
	{
		currentdistance -= STEP_DISTANCE * floorf(currentdistance / STEP_DISTANCE);
	}
}

bool arx::character::valid_jump_pos()
{
	if (LAST_ON_PLATFORM || climbing)
	{
		return true;
	}

	EERIE_CYLINDER tmpp;
	tmpp.height = physics.cyl.height;
	tmpp.origin = pos + Vec3f(0, -PLAYER_BASE_HEIGHT, 0);
	tmpp.radius = physics.cyl.radius * 0.85f;

	float tmp = CheckAnythingInCylinder(&tmpp, inter.iobj[0], CFLAG_PLAYER | CFLAG_JUST_TEST);

	if (tmp <= 20.0f)
	{
		return true;
	}

	long hum = 0;

	for (float vv = 0; vv < 360.0f; vv += 20.0f)
	{
		tmpp.origin = pos + Vec3f(EEsin(radians(vv)) * -20.0f, -PLAYER_BASE_HEIGHT, EEcos(radians(vv)) * 20.0f);
		tmpp.radius = physics.cyl.radius;
		float anything = CheckAnythingInCylinder(&tmpp, inter.iobj[0], CFLAG_JUST_TEST); // -cyl->origin.y;

		if (anything > 10)
		{
			hum = 1;
			break;
		}
	}

	if (!hum)
	{
		return true;
	}


	if (COLLIDED_CLIMB_POLY)
	{
		climbing = 1;
		return true;
	}

	if (tmp > 50.0f)
	{
		return false;
	}

	return true;
}

void arx::character::manage_movement()
{
	// Is our player able to move ?
	if (!CINEMASCOPE && !BLOCK_PLAYER_CONTROLS && inter.iobj[0])
	{
		// Compute current player speedfactor
		float speedfactor = inter.iobj[0]->basespeed + inter.iobj[0]->speed_modif;

		if (speedfactor < 0) speedfactor = 0;
		if (cur_mr == 3) speedfactor += 0.5f;
		if (cur_rf == 3) speedfactor += 1.5f;

		// Compute time things
		static float StoredTime = 0;
		float DeltaTime = StoredTime;

		if (Original_framedelay > 0)
		{
			DeltaTime = StoredTime + (float)Original_framedelay * speedfactor; //FrameDiff;
		}

		if (EDITMODE) 
		{
			DeltaTime = 25.f;
		}

		if (player.jump.phase)
		{
			while (DeltaTime > 25.f)
			{
				player.do_physics(DeltaTime);
				DeltaTime -= 25.f;
			}
		}
		else
		{
			player.do_physics(DeltaTime);
			DeltaTime = 0;
		}

		StoredTime = DeltaTime;
	}
}

void arx::character::do_physics(const float &DeltaTime)
{
	// A jump is requested so let's go !
	if (REQUEST_JUMP)
	{
		if ((Current_Movement & PLAYER_CROUCH) || (physics.cyl.height > -170.f))
		{
			float old = physics.cyl.height;
			physics.cyl.height = PLAYER_BASE_HEIGHT;
			physics.cyl.origin = pos + Vec3f(0, -PLAYER_BASE_HEIGHT, 0);
			float anything = CheckAnythingInCylinder(&physics.cyl, inter.iobj[0], CFLAG_JUST_TEST); // -cyl->origin.y;

			if (anything < 0.f)
			{
				Current_Movement |= PLAYER_CROUCH;
				physics.cyl.height = old;
				REQUEST_JUMP = 0;
			}
			else
			{
				bGCroucheToggle = false;
				Current_Movement &= ~PLAYER_CROUCH;
				physics.cyl.height = PLAYER_BASE_HEIGHT;
			}
		}

		if (!valid_jump_pos())
		{
			REQUEST_JUMP = 0;
		}

		if (REQUEST_JUMP)
		{
			float t = (float)ARXTime - (float)REQUEST_JUMP;

			if ((t >= 0.f) && (t <= 350.f))
			{
				REQUEST_JUMP = 0;
				ARX_NPC_SpawnAudibleSound(&pos, inter.iobj[0]);
				ARX_SPEECH_Launch_No_Unicode_Seek("player_jump", inter.iobj[0]);
				onfirmground = 0;
				jump.phase = arx::character::jump_data::anticipation;
			}
		}
	}


	if ((inter.iobj[0]->_npcdata->climb_count != 0.f) && (FrameDiff > 0))
	{
		inter.iobj[0]->_npcdata->climb_count -= MAX_ALLOWED_PER_SECOND * (float)FrameDiff * (1.0f / 10);

		if (inter.iobj[0]->_npcdata->climb_count < 0)
		{
			inter.iobj[0]->_npcdata->climb_count = 0.f;
		}
	}

	float d = 0;

	const float PLAYER_LEVITATE_HEIGHT = -220.0f;

	if (!EDITMODE && USE_PLAYERCOLLISIONS)
	{
		CollisionFlags levitate = 0;

		if(climbing)
		{
			levitate = CFLAG_LEVITATE;
		}

		if (levitate)
		{
			if (physics.cyl.height != PLAYER_LEVITATE_HEIGHT)
			{
				float old = physics.cyl.height;
				physics.cyl.height = PLAYER_LEVITATE_HEIGHT;
				physics.cyl.origin = pos + Vec3f(0, -PLAYER_BASE_HEIGHT, 0);
				float anything = CheckAnythingInCylinder(&physics.cyl, inter.iobj[0]);

				if (anything < 0.f)
				{
					physics.cyl.height = old;
					long num = ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_LEVITATE);

					if (num != -1)
					{
						spells[num].tolive = 0;
					}
				}
			}

			if (physics.cyl.height == PLAYER_LEVITATE_HEIGHT)
			{
				levitate = CFLAG_LEVITATE;
				climbing = 0;
				bGCroucheToggle = false;
				Current_Movement &= ~PLAYER_CROUCH;
			}
		}
		else if (physics.cyl.height == PLAYER_LEVITATE_HEIGHT)
		{
			physics.cyl.height = PLAYER_BASE_HEIGHT;
		}

		if ((jump.phase != jump_data::moving_up) && !levitate)
		{
			physics.cyl.origin = pos + Vec3f(0, 170.f, 0);
		}

		TRUE_FIRM_GROUND = (EEfabs(lastposy - pos.y) < DeltaTime * 1E-1f);

		lastposy = pos.y;
		DISABLE_JUMP = 0;
		float anything;
		EERIE_CYLINDER testcyl;
		memcpy(&testcyl, &physics.cyl, sizeof(EERIE_CYLINDER));
		testcyl.origin.y += 3.f;
		ON_PLATFORM = 0;
		anything = CheckAnythingInCylinder(&testcyl, inter.iobj[0], 0);
		LAST_ON_PLATFORM = ON_PLATFORM;

		if (jump.phase != jump_data::moving_up)
		{
			if (anything >= 0.f)
			{
				TRUE_FIRM_GROUND = false;
			}
			else
			{
				TRUE_FIRM_GROUND = true;
				testcyl.radius -= 30.f;
				testcyl.origin.y -= 10.f;
				anything = CheckAnythingInCylinder(&testcyl, inter.iobj[0], 0);

				if (anything < 0.f)
				{
					DISABLE_JUMP = true;
				}
			}
		}
		else
		{
			TRUE_FIRM_GROUND = false;
			LAST_ON_PLATFORM = false;
		}

		EERIE_CYLINDER cyl;
		cyl.origin = pos + Vec3f(0, 1.0f - PLAYER_BASE_HEIGHT, 0);
		cyl.radius = physics.cyl.radius;
		cyl.height = physics.cyl.height;
		float anything2 = CheckAnythingInCylinder(&cyl, inter.iobj[0], CFLAG_JUST_TEST | CFLAG_PLAYER); // -cyl->origin.y;


		if ((anything2 > -5)
		    &&  (physics.velocity.y > 15.f)
		    && !LAST_ON_PLATFORM
		    && !TRUE_FIRM_GROUND
		    && !jump.phase
		    && !levitate
		    && (anything > 80.f))
		{
			jump.phase = jump_data::finished;

			if (!falling)
			{
				falling = 1;
				start_fall();
			}
		}
		else if (!falling)
		{
			FALLING_TIME = 0;
		}

		if (jump.phase && levitate)
		{
			jump.phase = jump_data::not_jumping;
			falling = 0;
			Falling_Height = pos.y;
			FALLING_TIME = 0;
		}

		if (!LAST_FIRM_GROUND && TRUE_FIRM_GROUND)
		{
			jump.phase = jump_data::not_jumping;

			if ((FALLING_TIME > 0) && falling)
			{
				physics.velocity.x = 0.f;
				physics.velocity.z = 0.f;
				physics.forces.x = 0.f;
				physics.forces.z = 0.f;
				falling = 0;

				float fh = pos.y - Falling_Height;

				if (fh > 400.f)
				{
					float dmg = (fh - 400.f) * (1.0f / 15);

					if (dmg > 0.f)
					{
						Falling_Height = pos.y;
						FALLING_TIME = 0;
						ARX_DAMAGES_DamagePlayer(dmg, 0, -1);
						ARX_DAMAGES_DamagePlayerEquipment(dmg);
					}
				}
			}
		}

		LAST_FIRM_GROUND = TRUE_FIRM_GROUND;

		onfirmground = TRUE_FIRM_GROUND;

		if (onfirmground && !falling)
		{
			FALLING_TIME = 0;
		}

		// Apply Player Impulse Force
		Vec3f mv;
		float TheoricalMove = 230;
		long time = 1000;

		float jump_mul = 1.f;

		if (LAST_JUMP_ENDTIME + 600 > ARXTime)
		{
			JUMP_DIVIDE = 1;

			if (LAST_JUMP_ENDTIME + 300 > ARXTime)
			{
				jump_mul = 0.5f;
			}
			else
			{
				jump_mul = 0.5f;
				jump_mul += (float)(LAST_JUMP_ENDTIME + 300 - ARXTime) * (1.0f / 300);

				if (jump_mul > 1.f)
				{
					jump_mul = 1.f;
				}
			}
		}

		if (inter.iobj[0]->animlayer[0].cur_anim)
		{
			GetAnimTotalTranslate(inter.iobj[0]->animlayer[0].cur_anim, inter.iobj[0]->animlayer[0].altidx_cur, &mv);
			TheoricalMove = mv.length();

			time = inter.iobj[0]->animlayer[0].cur_anim->anims[inter.iobj[0]->animlayer[0].altidx_cur]->anim_time;

			if ((levitate) && (!climbing))
			{
				TheoricalMove = 70;
				time = 1000;
			}

			if (jump.phase)
			{
				TheoricalMove = 10;

				if (Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				{
					TheoricalMove = 420;

					if (Current_Movement & PLAYER_MOVE_STRAFE_LEFT)
					{
						TheoricalMove = 420;
					}

					if (Current_Movement & PLAYER_MOVE_STRAFE_RIGHT)
					{
						TheoricalMove = 420;
					}
				}
				else if (Current_Movement & PLAYER_MOVE_STRAFE_LEFT)
				{
					TheoricalMove = 140.f;
				}
				else if (Current_Movement & PLAYER_MOVE_STRAFE_RIGHT)
				{
					TheoricalMove = 140.f;
				}

				if (Current_Movement & PLAYER_MOVE_WALK_BACKWARD)
				{
					TheoricalMove = 40;
				}

				time = 1000;
			}
		}
		else
		{
			TheoricalMove = 100;
			time = 1000;
		}

		TheoricalMove *= jump_mul;
		float mval = TheoricalMove / time * DeltaTime;

		if (jump.phase == jump_data::moving_up)
		{
			moveto.y = pos.y;
			physics.velocity.y = 0;
		}

		Vec3f mv2 = moveto - pos;

		if (climbing)
		{
			physics.velocity.y *= (1.0f / 2);
		}

		if (mv2 == Vec3f::ZERO)
		{
		}
		else
		{
			mv2 *= 1.f / mv2.length() * mval * (1.0f / 80.0f);
		}

		if (climbing)
		{
			physics.velocity.x = 0;
			physics.velocity.z = 0;

			if (Current_Movement & PLAYER_MOVE_WALK_FORWARD)
			{
				moveto.x = pos.x;
				moveto.z = pos.z;
			}

			if (Current_Movement & PLAYER_MOVE_WALK_BACKWARD)
			{
				mv2.x = 0;
				mv2.z = 0;
				moveto.x = pos.x;
				moveto.z = pos.z;
			}
		}

		physics.forces += mv2;

		Vec3f modifplayermove(0, 0, 0);

		// No Vertical Interpolation
		if (jump.phase)
		{
			inter.iobj[0]->_npcdata->vvpos = -99999.f;
		}

		// Apply Gravity force if not LEVITATING or JUMPING
		if (!levitate && (jump.phase != jump_data::moving_up) && !LAST_ON_PLATFORM)
		{
			// constants
			const float WORLD_GRAVITY = 0.1f;
			const float JUMP_GRAVITY = 0.02f; // OLD SETTING 0.03f

			physics.forces.y += (falling ? JUMP_GRAVITY : WORLD_GRAVITY);

			Vec3f mod_vect(0, 0, 0);
			long mod_vect_count = -1;

			// Check for LAVA Damage !!!
			float epcentery;
			EERIEPOLY *ep = CheckInPoly(pos.x, pos.y + 150.f, pos.z, &epcentery);

			if (ep)
			{
				if ((ep->type & POLY_LAVA) && (EEfabs(epcentery - (pos.y - PLAYER_BASE_HEIGHT)) < 30))
				{
					const float LAVA_DAMAGE = 10.f;
					float mul = 1.f - (EEfabs(epcentery - (pos.y - PLAYER_BASE_HEIGHT)) * (1.0f / 30));
					float damages = LAVA_DAMAGE * FrameDiff * (1.0f / 100) * mul;
					damages = ARX_SPELLS_ApplyFireProtection(inter.iobj[0], damages);
					ARX_DAMAGES_DamagePlayer(damages, DAMAGE_TYPE_FIRE, 0);
					ARX_DAMAGES_DamagePlayerEquipment(damages);
					Vec3f pos;
					pos.x = pos.x;
					pos.y = pos.y - PLAYER_BASE_HEIGHT;
					pos.z = pos.z;
					ARX_PARTICLES_Spawn_Lava_Burn(&pos, inter.iobj[0]);
				}
			}

			LAST_VECT_COUNT = mod_vect_count;
		}

		// Apply Velocity Damping (Natural Velocity Attenuation. Stands for friction)
		float dampen = 1.f - (0.009f * DeltaTime);

		if (dampen < 0.001f)
		{
			dampen = 0.f;
		}

		physics.velocity.x *= dampen;
		physics.velocity.z *= dampen;

		if (EEfabs(physics.velocity.x) < 0.001f)
		{
			physics.velocity.x = 0;
		}

		if (EEfabs(physics.velocity.z) < 0.001f)
		{
			physics.velocity.z = 0;
		}

		// Apply Attraction
		Vec3f attraction;
		ARX_SPECIAL_ATTRACTORS_ComputeForIO(*inter.iobj[0], attraction);
		physics.forces += attraction;

		// Apply Push Player Force
		physics.forces += PUSH_PLAYER_FORCE;

		PUSH_PLAYER_FORCE = Vec3f::ZERO;

		// Apply Forces To Velocity
		physics.velocity += physics.forces * DeltaTime;

		// Apply Climbing Velocity
		if (climbing)
		{
			if (Current_Movement & PLAYER_MOVE_WALK_FORWARD)
			{
				physics.velocity.y = -0.2f * DeltaTime;
			}

			if (Current_Movement & PLAYER_MOVE_WALK_BACKWARD)
			{
				physics.velocity.y = 0.2f * DeltaTime;
			}
		}

		// Removes Y Velocity if onfirmground...
		if ((onfirmground == 1) && !climbing)
		{
			physics.velocity.y = 0.f;
		}

		float posy;
		EERIEPOLY *ep = CheckInPolyPrecis(pos.x, pos.y, pos.z, &posy);

		if ((ep == NULL) || (!climbing && (pos.y >= posy)))
		{
			physics.velocity.y = 0;
		}

		// Reset Forces
		physics.forces = Vec3f::ZERO;

		// Check if player is already on firm ground AND not moving
		if ((EEfabs(physics.velocity.x) < 0.001f) &&
		    (EEfabs(physics.velocity.z) < 0.001f) && (onfirmground == 1)
			 && (jump.phase == jump_data::not_jumping))
		{
			moveto = pos;
			goto lasuite;
		}
		else   // Need to apply some physics/collision tests
		{
			physics.startpos = physics.cyl.origin = pos + Vec3f(0, -PLAYER_BASE_HEIGHT, 0);
			physics.targetpos = physics.startpos + physics.velocity + modifplayermove * DeltaTime;

			// Jump Impulse
			if (jump.phase == jump_data::moving_up)
			{
				if (jump.last_position == -1.f)
				{
					jump.last_position = 0;
					jump.start_time = ARXTimeUL();
				}

				float jump_up_time  = 200.f;
				float jump_up_height =  130.f;
				long timee      = lARXTime;
				float offset_time = (float)timee - (float)jump.start_time;
				float divider   = 1.f / jump_up_time;
				float position    = (float)offset_time * divider;

				if (position > 1.f)
				{
					position = 1.f;
				}

				if (position < 0.f)
				{
					position = 0.f;
				}

				float p1 = position;
				float p2 = jump.last_position;
				physics.targetpos.y -= (p1 - p2) * jump_up_height;
				Full_Jump_Height += (p1 - p2) * jump_up_height;
				jump.last_position = position;
				levitate = 0;
			}

			bool test;
			APPLY_PUSH = 1;
			float PLAYER_CYLINDER_STEP = 40.f;

			if (climbing)
			{
				test = ARX_COLLISION_Move_Cylinder(&physics, inter.iobj[0], PLAYER_CYLINDER_STEP, CFLAG_EASY_SLIDING | CFLAG_CLIMBING | CFLAG_PLAYER);

				if (!COLLIDED_CLIMB_POLY)
				{
					climbing = 0;
				}
			}
			else
			{
				test = ARX_COLLISION_Move_Cylinder(&physics, inter.iobj[0], PLAYER_CYLINDER_STEP, levitate | CFLAG_EASY_SLIDING | CFLAG_PLAYER);

				if ((!test)
				    &&  ((!LAST_FIRM_GROUND) && (!TRUE_FIRM_GROUND)))
				{
					physics.velocity.x = 0.f;
					physics.velocity.z = 0.f;
					physics.forces.x = 0.f;
					physics.forces.z = 0.f;

					if ((FALLING_TIME > 0) && falling)
					{
						float fh = pos.y - Falling_Height;

						if (fh > 400.f)
						{
							float dmg = (fh - 400.f) * (1.0f / 15);

							if (dmg > 0.f)
							{
								Falling_Height = (pos.y + Falling_Height * 2) * (1.0f / 3);
								ARX_DAMAGES_DamagePlayer(dmg, 0, -1);
								ARX_DAMAGES_DamagePlayerEquipment(dmg);
							}
						}
					}
				}

				if ((test == false) && (jump.phase > jump_data::not_jumping))
				{
					physics.startpos.x = physics.cyl.origin.x = pos.x;
					physics.startpos.z = physics.cyl.origin.z = pos.z;
					physics.targetpos.x = physics.startpos.x;
					physics.targetpos.z = physics.startpos.z;

					if (physics.targetpos.y != physics.startpos.y)
					{
						test = ARX_COLLISION_Move_Cylinder(&physics, inter.iobj[0], PLAYER_CYLINDER_STEP, levitate | CFLAG_EASY_SLIDING | CFLAG_PLAYER);
						inter.iobj[0]->_npcdata->vvpos = -99999.f;
					}
				}
			}

			// LAST_ON_PLATFORM=ON_PLATFORM;
			if (COLLIDED_CLIMB_POLY)
			{
				climbing = 1;
			}

			if (climbing)
			{
				if ((Current_Movement != 0) && (Current_Movement != PLAYER_ROTATE)
				    && !(Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				    && !(Current_Movement & PLAYER_MOVE_WALK_BACKWARD))
				{
					climbing = 0;
				}

				if ((Current_Movement & PLAYER_MOVE_WALK_BACKWARD)  && !test)
				{
					climbing = 0;
				}

				if (climbing)
				{
					jump.phase = jump_data::not_jumping;
					falling = 0;
					FALLING_TIME = 0;
					Falling_Height = pos.y;
				}
			}

			if (jump.phase == jump_data::moving_up)
			{
				climbing = 0;
			}

			APPLY_PUSH = 0;
			moveto.x = physics.cyl.origin.x;
			moveto.y = physics.cyl.origin.y + PLAYER_BASE_HEIGHT;
			moveto.z = physics.cyl.origin.z;
			d = dist(pos, moveto);
		}
	}
	else
	{
		if (!EDITMODE)
		{
			Vec3f vect = moveto - pos;
			float divv = vect.length();

			if (divv > 0.f)
			{
				float mul = (float)FrameDiff * (1.0f / 1000) * 200.f;
				divv = mul / divv;
				vect *= divv;
				moveto = pos + vect;
			}
		}

		onfirmground = 0;
	}

	if (pos == moveto)
	{
		d = 0.f;
	}

	// Emit Stepsound
	if (USE_PLAYERCOLLISIONS && !EDITMODE)
	{
		if (Current_Movement & PLAYER_CROUCH)
		{
			d *= 2.f;
		}

		currentdistance += d;

		if (!jump.phase && !falling && (currentdistance >= STEP_DISTANCE))
		{
			make_step_noise();
		}
	}

	// Finally update player pos !
	pos = moveto;

lasuite:;

	// Get Player position color
	grnd_color = GetColorz(pos.x, pos.y + 90, pos.z);
	grnd_color -= 15.f;

	if (CURRENT_PLAYER_COLOR < grnd_color)
	{
		CURRENT_PLAYER_COLOR += FrameDiff * (1.0f / 8);
		CURRENT_PLAYER_COLOR = std::min(CURRENT_PLAYER_COLOR, grnd_color);
	}

	if (CURRENT_PLAYER_COLOR > grnd_color)
	{
		CURRENT_PLAYER_COLOR -= FrameDiff * (1.0f / 4);
		CURRENT_PLAYER_COLOR = std::max(CURRENT_PLAYER_COLOR, grnd_color);
	}

	if (InventoryDir != 0)
	{
		if ((Interface & INTER_COMBATMODE) || (doingmagic >= 2) || (InventoryDir == -1))
		{
			if (InventoryX > -160)
			{
				InventoryX -= INTERFACE_RATIO(FrameDiff * (1.0f / 3));
			}
		}
		else
		{
			if (InventoryX < 0)
			{
				InventoryX += InventoryDir * INTERFACE_RATIO(FrameDiff * (1.0f / 3));
			}
		}

		if (InventoryX <= -160)
		{
			InventoryX = -160;
			InventoryDir = 0;

			if (Interface & INTER_STEAL || ioSteal)
			{
				SendIOScriptEvent(ioSteal, SM_STEAL, "off");
				Interface &= ~INTER_STEAL;
				ioSteal = NULL;
			}

			SecondaryInventory = NULL;
			TSecondaryInventory = NULL;
			InventoryDir = 0;
		}
		else if (InventoryX >= 0)
		{
			InventoryX = 0;
			InventoryDir = 0;
		}
	}
}
