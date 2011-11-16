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

#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Inventory.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "graphics/particle/ParticleEffects.h"
#include "gui/Interface.h"
#include "gui/Speech.h"
#include "io/log/Logger.h"
#include "physics/Attractors.h"
#include "physics/Collisions.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

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

// externs
extern float PLAYER_ARMS_FOCAL;
extern long CURRENT_BASE_FOCAL;
extern long TRAP_DETECT;
extern long TRAP_SECRET;

extern long HERO_SHOW_1ST;
extern long MOVE_PRECEDENCE;
extern long EXTERNALVIEW;

// Updates Many player infos each frame...
void arx::character::frame_update()
{
	if (ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_PARALYSE) >= 0)
	{
		PLAYER_PARALYSED = 1;
	}
	else
	{
		inter.iobj[0]->ioflags &= ~IO_FREEZESCRIPT;
		PLAYER_PARALYSED = 0;
	}

	// Reset player moveto info
	moveto = pos;

	// Reset current movement flags
	Current_Movement = 0;

	// Updates player angles to desired angles
	angle.a = desiredangle.a;
	angle.b = desiredangle.b;

	// Updates player Extra-Rotate Informations
	INTERACTIVE_OBJ *io;
	io = inter.iobj[0];

	if ((io) && (io->_npcdata->ex_rotate))
	{
		float v = angle.a;

		if (v > 160)
		{
			v = -(360 - v);
		}

		if (Interface & INTER_COMBATMODE)
		{
			if (ARX_EQUIPMENT_GetPlayerWeaponType() == WEAPON_BOW)
			{
				io->_npcdata->ex_rotate->group_rotate[0].a = 0; // head
				io->_npcdata->ex_rotate->group_rotate[1].a = 0; // neck
				io->_npcdata->ex_rotate->group_rotate[2].a = 0; // chest
				io->_npcdata->ex_rotate->group_rotate[3].a = v; // belt
			}
			else
			{
				v *= (1.0f / 10);
				io->_npcdata->ex_rotate->group_rotate[0].a = v; // head
				io->_npcdata->ex_rotate->group_rotate[1].a = v; // neck
				io->_npcdata->ex_rotate->group_rotate[2].a = v * 4; // chest
				io->_npcdata->ex_rotate->group_rotate[3].a = v * 4; // belt
			}
		}
		else
		{
			v *= (1.0f / 4);
			io->_npcdata->ex_rotate->group_rotate[0].a = v; // head
			io->_npcdata->ex_rotate->group_rotate[1].a = v; // neck
			io->_npcdata->ex_rotate->group_rotate[2].a = v; // chest
			io->_npcdata->ex_rotate->group_rotate[3].a = v; // belt*/
		}

		if ((Interface & INTER_COMBATMODE) || (doingmagic == 2))
		{
			io->_npcdata->ex_rotate->flags &= ~EXTRA_ROTATE_REALISTIC;
		}
	}

	// Changes player ARMS focal depending on alpha angle.
	if (angle.a > 180)
	{
		PLAYER_ARMS_FOCAL = (float)CURRENT_BASE_FOCAL - 80.f;
	}
	else
	{
		PLAYER_ARMS_FOCAL = (float)CURRENT_BASE_FOCAL - 80.f + angle.a;
	}

	PLAYER_ARMS_FOCAL = static_cast<float>(CURRENT_BASE_FOCAL);

	compute_full_stats();

	TRAP_DETECT = checked_range_cast<long>(full.skill.mecanism);
	TRAP_SECRET = checked_range_cast<long>(full.skill.intuition);

	if (ARX_SPELLS_GetSpellOn(inter.iobj[0], SPELL_DETECT_TRAP) >= 0)
	{
		TRAP_DETECT = 100;
	}

	ModeLight |= MODE_DEPTHCUEING;

	player.torch_manage();
}

//   Manages Player visual
//   Choose the set of animations to use to represent current player situation.
void arx::character::manage_visual()
{
	unsigned long tim = ARXTimeUL();

	if (Current_Movement & PLAYER_ROTATE)
	{
		if (ROTATE_START == 0)
		{
			ROTATE_START = tim;
		}
	}
	else
	{
		if (ROTATE_START)
		{
			float diff = (float)tim - (float)ROTATE_START;

			if (diff > 100)
			{
				ROTATE_START = 0;
			}
		}
	}

	static long special[3];
	long light = 0;

	if (inter.iobj[0])
	{
		INTERACTIVE_OBJ *io = inter.iobj[0];

		if (!BLOCK_PLAYER_CONTROLS)
		{
			if (sp_max)
			{
				io->halo.color.r = 1.f;
				io->halo.color.g = 0.f;
				io->halo.color.b = 0.f;
				io->halo.flags |= HALO_ACTIVE | HALO_DYNLIGHT;
				io->halo.radius = 20.f;
				stat.life += (float)FrameDiff * (1.0f / 10);
				stat.life = std::min(stat.life, stat.maxlife);
				stat.mana += (float)FrameDiff * (1.0f / 10);
				stat.mana = std::min(stat.mana, stat.maxmana);
			}
		}

		if (cur_mr == 3)
		{
			stat.life += (float)FrameDiff * (1.0f / 20);
			stat.life = std::min(stat.life, stat.maxlife);
			stat.mana += (float)FrameDiff * (1.0f / 20);
			stat.mana = std::min(stat.mana, stat.maxmana);
		}

		io->pos.x = pos.x;
		io->pos.y = pos.y - PLAYER_BASE_HEIGHT;
		io->pos.z = pos.z;

		if ((jumpphase == 0) && (!LAST_ON_PLATFORM))
		{
			float tempo;

			EERIEPOLY *ep = CheckInPolyPrecis(pos.x, pos.y, pos.z, &tempo);

			if (ep && ((io->pos.y > tempo - 30.f) && (io->pos.y < tempo)))
			{
				onfirmground = 1;
			}
		}

		ComputeVVPos(io);
		io->pos.y = io->_npcdata->vvpos;

		if ((!(Current_Movement & PLAYER_CROUCH)) && (physics.cyl.height > -150.f))
		{
			float old = physics.cyl.height;
			physics.cyl.height = PLAYER_BASE_HEIGHT;
			physics.cyl.origin.x = pos.x;
			physics.cyl.origin.y = pos.y - PLAYER_BASE_HEIGHT;
			physics.cyl.origin.z = pos.z;
			float anything = CheckAnythingInCylinder(&physics.cyl, inter.iobj[0]);

			if (anything < 0.f)
			{
				Current_Movement |= PLAYER_CROUCH;
				physics.cyl.height = old;
			}
		}

		if (stat.life > 0)
		{
			io->angle.a = 0;
			io->angle.b = 180.f - angle.b; // +90.f;
			io->angle.g = 0;
		}

		io->GameFlags |= GFLAG_ISINTREATZONE;

		ANIM_USE *ause0 = &io->animlayer[0];
		ANIM_USE *ause1 = &io->animlayer[1];
		ANIM_USE *ause3 = &io->animlayer[3];

		ause0->next_anim = NULL;
		inter.iobj[0]->animlayer[1].next_anim = NULL;
		inter.iobj[0]->animlayer[2].next_anim = NULL;
		inter.iobj[0]->animlayer[3].next_anim = NULL;
		ANIM_HANDLE **alist = io->anims;

		if (ause0->flags & EA_FORCEPLAY)
		{
			if (ause0->flags & EA_ANIMEND)
			{
				ause0->flags &= ~EA_FORCEPLAY;
				ause0->flags |= EA_STATICANIM;
				io->move = io->lastmove = Vec3f::ZERO;
			}
			else
			{
				ause0->flags &= ~EA_STATICANIM;
				io->pos = pos = moveto = pos + io->move;
				io->pos.y += -PLAYER_BASE_HEIGHT;
				goto nochanges;
			}
		}

		ANIM_HANDLE *ChangeMoveAnim = NULL;
		ANIM_HANDLE *ChangeMoveAnim2 = NULL;
		long ChangeMA_Loop = 1;
		long ChangeMA_Stopend = 0;

		if (io->ioflags & IO_FREEZESCRIPT)
		{
			goto nochanges;
		}

		if (stat.life <= 0)
		{
			HERO_SHOW_1ST = -1;
			io->animlayer[1].cur_anim = NULL;
			ChangeMoveAnim = alist[ANIM_DIE];
			ChangeMA_Loop = 0;
			ChangeMA_Stopend = 1;
			goto makechanges;
		}

		if ((Current_Movement == 0) || (Current_Movement == PLAYER_MOVE_STEALTH))
		{
			if (Interface & INTER_COMBATMODE)
			{
				ChangeMoveAnim = alist[ANIM_FIGHT_WAIT];
			}
			else if (EXTERNALVIEW)
			{
				ChangeMoveAnim = alist[ANIM_WAIT];
			}
			else
			{
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];
			}

			ChangeMA_Loop = 1;
		}

		if (Current_Movement & PLAYER_ROTATE)
		{
			if (Interface & INTER_COMBATMODE)
			{
				ChangeMoveAnim = alist[ANIM_FIGHT_WAIT];
			}
			else if (EXTERNALVIEW)
			{
				ChangeMoveAnim = alist[ANIM_WAIT];
			}
			else
			{
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];
			}

			ChangeMA_Loop = 1;
		}

		if ((ROTATE_START) && (angle.a > 60.f) && (angle.a < 180.f) && (LASTPLAYERA > 60.f) && (LASTPLAYERA < 180.f))
		{
			if (PLAYER_ROTATION < 0)
			{
				if (Interface & INTER_COMBATMODE)
				{
					ChangeMoveAnim = alist[ANIM_U_TURN_LEFT_FIGHT];
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_U_TURN_LEFT];
				}
			}
			else
			{
				if (Interface & INTER_COMBATMODE)
				{
					ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT_FIGHT];
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT];
				}
			}

			ChangeMA_Loop = 1;

			if ((ause0->cur_anim == alist[ANIM_U_TURN_LEFT]) || (ause0->cur_anim == alist[ANIM_U_TURN_LEFT_FIGHT]))
			{
				float fv = PLAYER_ROTATION * 5;
				long vv = fv;
				io->frameloss -= fv - (float)vv;

				if (io->frameloss < 0)
				{
					io->frameloss = 0;
				}

				ause0->ctime -= vv;

				if (ause0->ctime < 0)
				{
					ause0->ctime = 0;
				}
			}
			else if ((ause0->cur_anim == alist[ANIM_U_TURN_RIGHT]) || (ause0->cur_anim == alist[ANIM_U_TURN_RIGHT_FIGHT]))
			{
				long vv = PLAYER_ROTATION * 5;
				float fv = PLAYER_ROTATION * 5;
				io->frameloss += fv - (float)vv;

				if (io->frameloss < 0)
				{
					io->frameloss = 0;
				}

				ause0->ctime += vv;

				if (ause0->ctime < 0)
				{
					ause0->ctime = 0;
				}
			}
		}

		LASTPLAYERA = angle.a;

		{
			long tmove = Current_Movement;

			if (((tmove & PLAYER_MOVE_STRAFE_LEFT) && (tmove & PLAYER_MOVE_STRAFE_RIGHT)))
			{
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;
			}

			if (MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_LEFT)
			{
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;
			}

			if (MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_RIGHT)
			{
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;
			}

			if (MOVE_PRECEDENCE == PLAYER_MOVE_WALK_FORWARD)
			{
				tmove &= ~PLAYER_MOVE_WALK_BACKWARD;
			}

			if (Current_Movement & PLAYER_MOVE_WALK_FORWARD)
			{
				tmove = PLAYER_MOVE_WALK_FORWARD;
			}

			if (tmove & PLAYER_MOVE_STRAFE_LEFT)
			{
				if (Interface & INTER_COMBATMODE)
				{
					ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_LEFT];
				}
				else if (Current_Movement & PLAYER_MOVE_STEALTH)
				{
					ChangeMoveAnim = alist[ANIM_STRAFE_LEFT];
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_STRAFE_RUN_LEFT];
				}
			}

			if (tmove & PLAYER_MOVE_STRAFE_RIGHT)
			{
				if (Interface & INTER_COMBATMODE)
				{
					ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_RIGHT];
				}
				else if (Current_Movement & PLAYER_MOVE_STEALTH)
				{
					ChangeMoveAnim = alist[ANIM_STRAFE_RIGHT];
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_STRAFE_RUN_RIGHT];
				}
			}

			if (tmove & PLAYER_MOVE_WALK_BACKWARD)
			{
				if (Interface & INTER_COMBATMODE)
				{
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_BACKWARD];
				}
				else if (Current_Movement & PLAYER_MOVE_STEALTH)
				{
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				}
				else if (Current_Movement & PLAYER_CROUCH)
				{
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_RUN_BACKWARD];
				}
			}

			if (tmove & PLAYER_MOVE_WALK_FORWARD)
			{
				if (Interface & INTER_COMBATMODE)
				{
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_FORWARD];
				}
				else if (Current_Movement & PLAYER_MOVE_STEALTH)
				{
					ChangeMoveAnim = alist[ANIM_WALK];
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_RUN];
				}
			}
		}

		if (ChangeMoveAnim == NULL)
		{
			if (EXTERNALVIEW)
			{
				ChangeMoveAnim = alist[ANIM_WAIT];
			}
			else
			{
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];
			}

			ChangeMA_Loop = 1;
		}

		// Finally update anim
		if ((ause1->cur_anim == NULL) &&
		    ((ause0->cur_anim == alist[ANIM_WAIT]) || (ause0->cur_anim == alist[ANIM_WAIT_SHORT])) &&
		    !(Current_Movement & PLAYER_CROUCH))
		{
			if ((Current_Movement & PLAYER_LEAN_LEFT) && (Current_Movement & PLAYER_LEAN_RIGHT))
			{
			}
			else
			{
				if (Current_Movement & PLAYER_LEAN_LEFT)
				{
					ChangeMoveAnim2 = alist[ANIM_LEAN_LEFT];
					// ChangeMA_Loop=0;
				}

				if (Current_Movement & PLAYER_LEAN_RIGHT)
				{
					ChangeMoveAnim2 = alist[ANIM_LEAN_RIGHT];
				}
			}
		}

		if ((ChangeMoveAnim2 == NULL) && ause3->cur_anim &&
		    ((ause3->cur_anim == alist[ANIM_LEAN_RIGHT]) ||
		     (ause3->cur_anim == alist[ANIM_LEAN_LEFT])))
		{
			AcquireLastAnim(io);
			ause3->cur_anim = NULL;
		}

		if ((Current_Movement & PLAYER_CROUCH) && !(Last_Movement & PLAYER_CROUCH)
		    && !levitate)
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_START];
			ChangeMA_Loop = 0;
		}
		else if (!(Current_Movement & PLAYER_CROUCH) && (Last_Movement & PLAYER_CROUCH))
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_END];
			ChangeMA_Loop = 0;
		}
		else if (Current_Movement & PLAYER_CROUCH)
		{
			if (ause0->cur_anim == alist[ANIM_CROUCH_START])
			{
				if (!(ause0->flags & EA_ANIMEND))
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_START];
					ChangeMA_Loop = 0;
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = 1;
					const float PLAYER_CROUCH_HEIGHT = -120.0f;
					physics.cyl.height = PLAYER_CROUCH_HEIGHT;
				}
			}
			else
			{
				if ((ChangeMoveAnim == alist[ANIM_STRAFE_LEFT])
				    ||  (ChangeMoveAnim == alist[ANIM_STRAFE_RUN_LEFT])
				    ||  (ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_LEFT]))
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_LEFT];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_STRAFE_RIGHT])
				         || (ChangeMoveAnim == alist[ANIM_STRAFE_RUN_RIGHT])
				         || (ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_RIGHT]))
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_RIGHT];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_WALK])
				         || (ChangeMoveAnim == alist[ANIM_RUN])
				         || (ChangeMoveAnim == alist[ANIM_FIGHT_WALK_FORWARD]))
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_WALK_BACKWARD])
				         || (ChangeMoveAnim == alist[ANIM_FIGHT_WALK_BACKWARD]))
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK_BACKWARD];
					ChangeMA_Loop = 1;
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = 1;
				}
			}
		}

		if (ause0->cur_anim == alist[ANIM_CROUCH_END])
		{
			if (!(ause0->flags & EA_ANIMEND))
			{
				goto nochanges;
			}
		}

retry:;

		if (ARX_SPELLS_ExistAnyInstance(SPELL_FLYING_EYE))
		{
			FistParticles |= 1;
		}
		else
		{
			FistParticles &= ~1;
		}

		if (FistParticles)
		{
			light = 1;

			if (FistParticles & 1)
			{
				ChangeMoveAnim = alist[ANIM_MEDITATION];
				ChangeMA_Loop = 1;
			}

			EERIE_3DOBJ *eobj = io->obj;
			long pouet = 2;

			while (pouet) {
				long id;

				if (pouet == 2)
				{
					id = io->obj->fastaccess.primary_attach;
				}
				else
				{
					id = GetActionPointIdx(io->obj, "left_attach");
				}

				pouet--;

				if (id != -1)
				{
					if (special[pouet] == -1)
					{
						special[pouet] = GetFreeDynLight();
					}

					if (special[pouet] != -1)
					{
						EERIE_LIGHT *el = &DynLight[special[pouet]];
						el->intensity = 1.3f;
						el->exist = 1;
						el->fallend = 180.f;
						el->fallstart = 50.f;

						if (FistParticles & 2)
						{
							el->rgb.r = 1.f;
							el->rgb.g = 0.3f;
							el->rgb.b = 0.2f;
						}
						else
						{
							el->rgb.r = 0.7f;
							el->rgb.g = 0.3f;
							el->rgb.b = 1.f;
						}

						el->pos.x = eobj->vertexlist3[id].v.x;
						el->pos.y = eobj->vertexlist3[id].v.y;
						el->pos.z = eobj->vertexlist3[id].v.z;
					}
					else
					{
						LogWarning << "Maximum number of dynamic lights exceeded.";
						/*
						   EERIE_LIGHT * el = &DynLight[special[pouet]];
						   el->intensity = 1.3f + rnd() * 0.2f;
						   el->fallend = 175.f + rnd() * 10.f;

						   if (FistParticles & 2)
						   {
						   el->fallstart *= 2.f;
						   el->fallend *= 3.f;
						   }
						 */
					}

					for (long kk = 0; kk < 2; kk++)
					{
						Vec3f  target;
						target.x = eobj->vertexlist3[id].v.x;
						target.y = eobj->vertexlist3[id].v.y;
						target.z = eobj->vertexlist3[id].v.z;
						long j = ARX_PARTICLES_GetFree();

						if ((j != -1) && (!ARXPausedTimer))
						{
							ParticleCount++;
							particle[j].exist       = true;
							particle[j].zdec        = 0;
							particle[j].ov.x        = target.x + 1.f - rnd() * 2.f;
							particle[j].ov.y        = target.y + 1.f - rnd() * 2.f;
							particle[j].ov.z        = target.z + 1.f - rnd() * 2.f;
							particle[j].move.x      = 0.1f - 0.2f * rnd();
							particle[j].move.y      = -2.2f * rnd();
							particle[j].move.z      = 0.1f - 0.2f * rnd();
							particle[j].siz         = 5.f;
							particle[j].tolive      = 1500 + (unsigned long)(float)(rnd() * 2000.f);
							particle[j].scale.x     = 0.2f;
							particle[j].scale.y     = 0.2f;
							particle[j].scale.z     = 0.2f;
							particle[j].timcreation = lARXTime;
							particle[j].tc          = TC_smoke;
							particle[j].special     = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING; // | SUBSTRACT;
							particle[j].sourceionum = 0;
							particle[j].source      = &eobj->vertexlist3[id].v;
							particle[j].fparam      = 0.0000001f;

							if(FistParticles & 2)
							{
								particle[j].move.y *= 2.f;
								particle[j].rgb = Color3f(1.0f - rnd() * 0.1f, 0.3f + rnd() * 0.1f, 0.2f - rnd() * 0.1f);
							}
							else
							{
								particle[j].rgb = Color3f(0.7f - rnd() * 0.1f, 0.3f - rnd() * 0.1f, 1.0f - rnd() * 0.1f);
							}
						}
					}
				}
			}

			goto makechanges;
		}
		else if (ARX_SPELLS_GetSpellOn(io, SPELL_LEVITATE) >= 0)
		{
			ChangeMoveAnim = alist[ANIM_LEVITATE];
			ChangeMA_Loop = 1;
			goto makechanges;
		}
		else if (jumpphase)
		{
			switch (jumpphase)
			{
			case 1: // Anticipation
				FALLING_TIME = 0;
				Full_Jump_Height = 0;
				jumpphase = 2;
				ChangeMoveAnim = alist[ANIM_JUMP_UP];
				jumpstarttime = ARXTimeUL();
				jumplastposition = -1.f;
				break;

			case 2: // Moving Up
				ChangeMoveAnim = alist[ANIM_JUMP_UP];

				if (jumplastposition >= 1.f)
				{
					jumpphase = 4;
					ChangeMoveAnim = alist[ANIM_JUMP_CYCLE];
					start_fall();
				}

				break;

			case 4: // Post-synch
				LAST_JUMP_ENDTIME = ARXTimeUL();

				if (((ause0->cur_anim == alist[ANIM_JUMP_END])
				     && (ause0->flags & EA_ANIMEND))
				    || onfirmground)
				{
					jumpphase = 5;
					ChangeMoveAnim = alist[ANIM_JUMP_END_PART2];
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_JUMP_END];
				}

				break;

			case 5: // Post-synch
				LAST_JUMP_ENDTIME = ARXTimeUL();

				if ((ause0->cur_anim == alist[ANIM_JUMP_END_PART2])
				    && (ause0->flags & EA_ANIMEND))
				{
					AcquireLastAnim(io);
					jumpphase = 0;
					goto retry;
				}
				else if ((ause0->cur_anim == alist[ANIM_JUMP_END_PART2])
				         && ((EEfabs(physics.velocity.x) + EEfabs(physics.velocity.z)) > 4)
				         && (ause0->ctime > 1))
				{
					AcquireLastAnim(io);
					jumpphase = 0;
					goto retry;
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_JUMP_END_PART2];
				}

				break;
			}

			if (ChangeMoveAnim && (ChangeMoveAnim != ause0->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if ((ChangeMoveAnim == alist[ANIM_U_TURN_LEFT]) ||
				    (ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT]) ||
				    (ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT]) ||
				    (ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]))
				{
					ause0->flags |= EA_EXCONTROL;
				}
			}

			if ((ChangeMoveAnim2) && (ChangeMoveAnim2 != ause3->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		}
		else
		{
makechanges:;

			if ((ChangeMoveAnim) && (ChangeMoveAnim != ause0->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if (ChangeMA_Loop)
				{
					ause0->flags |= EA_LOOP;
				}

				if (ChangeMA_Stopend)
				{
					ause0->flags |= EA_STOPEND;
				}

				if ((ChangeMoveAnim == alist[ANIM_U_TURN_LEFT])
				    ||  (ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT])
				    ||  (ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT])
				    ||  (ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]))
				{
					ause0->flags |= EA_EXCONTROL;
				}
			}

			if ((ChangeMoveAnim2) && (ChangeMoveAnim2 != ause3->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		}

		memcpy(&io->physics, &physics, sizeof(IO_PHYSICS));
	}

nochanges:;

	Last_Movement = Current_Movement;

	if (!light)
	{
		if (special[2] != -1)
		{
			DynLight[special[2]].exist = 0;
			special[2] = -1;
		}

		if (special[1] != -1)
		{
			DynLight[special[1]].exist = 0;
			special[1] = -1;
		}
	}
}
