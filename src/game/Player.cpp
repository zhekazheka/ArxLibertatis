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

#include "game/Character.h"
#include "game/Player.h"

#include <vector>

#include "ai/PathFinderManager.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"

#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/NPC.h"

#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/Speech.h"
#include "gui/Text.h"

#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleManager.h"

#include "io/PakReader.h"

#include "physics/Collisions.h"

#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

void Manage_sp_max();

// globals
EERIE_3DOBJ *hero = NULL;
ANIM_HANDLE *herowaitbook = NULL;
ANIM_HANDLE *herowait2 = NULL;
ANIM_HANDLE *herowait_2h = NULL;
TextureContainer *PLAYER_SKIN_TC = NULL;

float sp_max_y[64];
Color sp_max_col[64];
char  sp_max_ch[64];
long  sp_max_nb;

// externs
extern bool bBookHalo;
extern unsigned long ulBookHaloTime;
extern float sp_max_start;

bool ARX_PLAYER_IsInFightMode()
{
	if (player.Interface & INTER_COMBATMODE)
	{
		return true;
	}

	if (inter.iobj && inter.iobj[0] && inter.iobj[0]->animlayer[1].cur_anim)
	{
		ANIM_USE *ause1 = &inter.iobj[0]->animlayer[1];
		ANIM_HANDLE **alist = inter.iobj[0]->anims;

		AnimationNumber animation_case[] =
		{
			ANIM_BARE_READY, ANIM_BARE_UNREADY,
			ANIM_DAGGER_READY_PART_1, ANIM_DAGGER_READY_PART_2, ANIM_DAGGER_UNREADY_PART_1, ANIM_DAGGER_UNREADY_PART_2,
			ANIM_1H_READY_PART_1, ANIM_1H_READY_PART_2, ANIM_1H_UNREADY_PART_1, ANIM_1H_UNREADY_PART_2,
			ANIM_2H_READY_PART_1, ANIM_2H_READY_PART_2, ANIM_2H_UNREADY_PART_1, ANIM_2H_UNREADY_PART_2,
			ANIM_MISSILE_READY_PART_1, ANIM_MISSILE_READY_PART_2, ANIM_MISSILE_UNREADY_PART_1, ANIM_MISSILE_UNREADY_PART_2,
			ANIM_NONE,
		};

		for (int i = 0; animation_case[i] != ANIM_NONE; i++)
		{
			if (ause1->cur_anim == alist[animation_case[i]])
			{
				return true;
			}
		}
	}

	return false;
}

// Reset all extra-rotation groups of player
void ARX_PLAYER_RectifyPosition()
{
	INTERACTIVE_OBJ *io = inter.iobj[0];

	if ((io) && (io->_npcdata->ex_rotate))
	{
		for (long n = 0; n < MAX_EXTRA_ROTATE; n++)
		{
			io->_npcdata->ex_rotate->group_rotate[n].a = 0;
			io->_npcdata->ex_rotate->group_rotate[n].b = 0;
			io->_npcdata->ex_rotate->group_rotate[n].g = 0;
		}

		io->_npcdata->ex_rotate->flags = 0;
	}
}

void arx::character::torch_kill()
{
	CURRENT_TORCH->show = SHOW_FLAG_IN_SCENE;
	ARX_SOUND_PlaySFX(SND_TORCH_END);
	ARX_SOUND_Stop(SND_TORCH_LOOP);

	if (CanBePutInInventory(CURRENT_TORCH))
	{
		if (CURRENT_TORCH)
		{
			CURRENT_TORCH->show = SHOW_FLAG_IN_INVENTORY;
		}
	}
	else
	{
		PutInFrontOfPlayer(CURRENT_TORCH);
	}

	CURRENT_TORCH = NULL;
	SHOW_TORCH = 0;
	DynLight[0].exist = 0;
}

void arx::character::torch_clicked(INTERACTIVE_OBJ *io)
{
	if (io == NULL)
	{
		return;
	}

	if (CURRENT_TORCH == NULL)
	{
		if (io->durability > 0)
		{
			if (io->ignition > 0)
			{
				if (ValidDynLight(io->ignit_light))
				{
					DynLight[io->ignit_light].exist = 0;
				}

				io->ignit_light = -1;

				if (io->ignit_sound != audio::INVALID_ID)
				{
					ARX_SOUND_Stop(io->ignit_sound);
					io->ignit_sound = audio::INVALID_ID;
				}

				io->ignition = 0;
			}

			SHOW_TORCH = 1;

			ARX_SOUND_PlaySFX(SND_TORCH_START);
			ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);

			RemoveFromAllInventories(io);

			CURRENT_TORCH = io;

			io->show = SHOW_FLAG_ON_PLAYER;

			if (DRAGINTER == io)
			{
				DRAGINTER = NULL;
			}
		}
	}
	else if (CURRENT_TORCH == io)
	{
		player.torch_kill();
	}
	else
	{
		player.torch_kill();

		if (io->durability > 0)
		{
			if (io->ignition > 0)
			{
				if (io->ignit_light != -1)
				{
					DynLight[io->ignit_light].exist = 0;
					io->ignit_light = -1;
				}

				if (io->ignit_sound != audio::INVALID_ID)
				{
					ARX_SOUND_Stop(io->ignit_sound);
					io->ignit_sound = audio::INVALID_ID;
				}

				io->ignition = 0;
			}

			SHOW_TORCH = 1;

			ARX_SOUND_PlaySFX(SND_TORCH_START);
			ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);

			RemoveFromAllInventories(io);

			CURRENT_TORCH = io;

			io->show = SHOW_FLAG_ON_PLAYER;

			if (DRAGINTER == io)
			{
				DRAGINTER = NULL;
			}
		}
	}
}

void arx::character::torch_manage()
{
	if (CURRENT_TORCH)
	{
		CURRENT_TORCH->ignition = 0;
		CURRENT_TORCH->durability -= FrameDiff * (1.0f / 10000);

		if (CURRENT_TORCH->durability <= 0)
		{
			ARX_SPEECH_ReleaseIOSpeech(CURRENT_TORCH);
			// Need To Kill timers
			ARX_SCRIPT_Timer_Clear_By_IO(CURRENT_TORCH);
			CURRENT_TORCH->show = SHOW_FLAG_KILLED;
			CURRENT_TORCH->GameFlags &= ~GFLAG_ISINTREATZONE;
			RemoveFromAllInventories(CURRENT_TORCH);
			ARX_INTERACTIVE_DestroyDynamicInfo(CURRENT_TORCH);
			ARX_SOUND_PlaySFX(SND_TORCH_END);
			ARX_SOUND_Stop(SND_TORCH_LOOP);
			ARX_INTERACTIVE_DestroyIO(CURRENT_TORCH);
			CURRENT_TORCH = NULL;
			SHOW_TORCH = 0;
			DynLight[0].exist = 0;
		}
	}
}

// Add quest "quest" to player Questbook
void ARX_PLAYER_Quest_Add(const std::string &quest, bool _bLoad)
{
	std::string output = getLocalised(quest);

	if (!output.empty())
	{
		player.quest.push_back(STRUCT_QUEST());
		player.quest.back().ident = quest;
		player.quest.back().localised = output;
		bBookHalo = !_bLoad;
		ulBookHaloTime = 0;
	}
}

void ARX_PLAYER_Restore_Skin()
{
	fs::path tx[4] =
	{
		"graph/obj3d/textures/npc_human_base_hero_head",
		"graph/obj3d/textures/npc_human_chainmail_hero_head",
		"graph/obj3d/textures/npc_human_chainmail_mithril_hero_head",
		"graph/obj3d/textures/npc_human_leather_hero_head",
	};

	switch (player.skin)
	{
	case 0:
		break;

	case 1:
		tx[0] = "graph/obj3d/textures/npc_human_base_hero2_head";
		tx[1] = "graph/obj3d/textures/npc_human_chainmail_hero2_head";
		tx[2] = "graph/obj3d/textures/npc_human_chainmail_mithril_hero2_head";
		tx[3] = "graph/obj3d/textures/npc_human_leather_hero2_head";
		break;

	case 2:
		tx[0] = "graph/obj3d/textures/npc_human_base_hero3_head";
		tx[1] = "graph/obj3d/textures/npc_human_chainmail_hero3_head";
		tx[2] = "graph/obj3d/textures/npc_human_chainmail_mithril_hero3_head";
		tx[3] = "graph/obj3d/textures/npc_human_leather_hero3_head";
		break;

	case 3:
		tx[0] = "graph/obj3d/textures/npc_human_base_hero4_head";
		tx[1] = "graph/obj3d/textures/npc_human_chainmail_hero4_head";
		tx[2] = "graph/obj3d/textures/npc_human_chainmail_mithril_hero4_head";
		tx[3] = "graph/obj3d/textures/npc_human_leather_hero4_head";
		break;

	case 4:
		tx[0] = "graph/obj3d/textures/npc_human_cm_hero_head";
		break;

	case 5:
	// just in case
	case 6:
		tx[0] = "graph/obj3d/textures/npc_human__base_hero_head";
		break;
	}

	TextureContainer *tmpTC;

	// TODO maybe it would be better to replace the textures in the player object
	// instead of replacing the texture data for all objects that use these textures

	if (PLAYER_SKIN_TC && !tx[0].empty())
	{
		PLAYER_SKIN_TC->LoadFile(tx[0]);
	}

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_chainmail_hero_head");

	if (tmpTC && !tx[1].empty())
	{
		tmpTC->LoadFile(tx[1]);
	}

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_chainmail_mithril_hero_head");

	if (tmpTC && !tx[2].empty())
	{
		tmpTC->LoadFile(tx[2]);
	}

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_leather_hero_head");

	if (tmpTC && !tx[3].empty())
	{
		tmpTC->LoadFile(tx[3]);
	}
}

// Load Mesh & anims for hero
void ARX_PLAYER_LoadHeroAnimsAndMesh()
{
	const char *OBJECT_HUMAN_BASE   = "graph/obj3d/interactive/npc/human_base/human_base.teo";
	const char *ANIM_WAIT_BOOK      = "graph/obj3d/anims/npc/human_wait_book.tea";
	const char *ANIM_WAIT_NORMAL    = "graph/obj3d/anims/npc/human_normal_wait.tea";
	const char *ANIM_WAIT_TWOHANDED = "graph/obj3d/anims/npc/human_wait_book_2handed.tea";

	const char *HUMAN_BASE_HEAD     = "graph/obj3d/textures/npc_human_base_hero_head";

	hero = loadObject(OBJECT_HUMAN_BASE, false);
	PLAYER_SKIN_TC = TextureContainer::Load(HUMAN_BASE_HEAD);

	herowaitbook = EERIE_ANIMMANAGER_Load(ANIM_WAIT_BOOK);
	herowait2 = EERIE_ANIMMANAGER_Load(ANIM_WAIT_NORMAL);
	herowait_2h = EERIE_ANIMMANAGER_Load(ANIM_WAIT_TWOHANDED);

	INTERACTIVE_OBJ *io = CreateFreeInter(0);
	io->obj = hero;

	player.skin = 0;
	ARX_PLAYER_Restore_Skin();

	ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0], 0);
	ARX_INTERACTIVE_HideGore(inter.iobj[0], 1);
	io->ident = -1;

	// todo free
	io->_npcdata = new IO_NPCDATA;

	io->ioflags = IO_NPC;
	io->_npcdata->maxlife = io->_npcdata->life = 10.f;
	io->_npcdata->vvpos = -99999.f;

	// todo free
	io->armormaterial = "leather";
	io->filename = "graph/obj3d/interactive/player/player.teo";
	loadScript(io->script, resources->getFile("graph/obj3d/interactive/player/player.asl"));

	const long eogg[4] =
	{
		EERIE_OBJECT_GetGroup(io->obj, "head"),
		EERIE_OBJECT_GetGroup(io->obj, "neck"),
		EERIE_OBJECT_GetGroup(io->obj, "chest"),
		EERIE_OBJECT_GetGroup(io->obj, "belt"),
	};

	if ((eogg[0] != -1) && (eogg[1] != -1) && (eogg[2] != -1) && (eogg[3] != -1))
	{
		io->_npcdata->ex_rotate = (EERIE_EXTRA_ROTATE *)malloc(sizeof(EERIE_EXTRA_ROTATE));

		if (io->_npcdata->ex_rotate)
		{
			io->_npcdata->ex_rotate->group_number[0] = (short)eogg[0];
			io->_npcdata->ex_rotate->group_number[1] = (short)eogg[1];
			io->_npcdata->ex_rotate->group_number[2] = (short)eogg[2];
			io->_npcdata->ex_rotate->group_number[3] = (short)eogg[3];

			for (long n = 0; n < MAX_EXTRA_ROTATE; n++)
			{
				io->_npcdata->ex_rotate->group_rotate[n] = Anglef::ZERO;
			}

			io->_npcdata->ex_rotate->flags = 0;
		}
	}

	ARX_INTERACTIVE_RemoveGoreOnIO(inter.iobj[0]);
}


// Called When player has just died
void ARX_PLAYER_BecomesDead()
{
	player.STARTED_A_GAME = 0;
	// a mettre au final
	player.BLOCK_PLAYER_CONTROLS = true;

	if (inter.iobj[0])
	{
		player.Interface &= ~INTER_COMBATMODE;
		player.Interface = 0;
		player.DeadCameraDistance = 40.f;
		player.DeadTime = 0;
	}

	for(size_t i = 0; i < MAX_SPELLS; i++)
	{
		if(spells[i].exist && (spells[i].caster == 0))
		{
			spells[i].tolive = 0;
		}
	}
}

// Manage Player Death Visual
void ARX_PLAYER_Manage_Death()
{
	player.PLAYER_PARALYSED = false;
	float ratio = (float)(player.DeadTime - 2000) * (1.0f / 5000);

	if (ratio >= 1.f)
	{
		ratio = 1.f;
	}

	if (ratio == 1.f)
	{
		ARX_MENU_Launch();
		player.DeadTime = 0;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	EERIEDrawBitmap(0.f, 0.f, static_cast<float>(DANAESIZX), static_cast<float>(DANAESIZY), 0.000091f, NULL, Color::gray(ratio));
}

// Teleport player to any poly...
void ARX_PLAYER_GotoAnyPoly()
{
	for (long j = 0; j < ACTIVEBKG->Zsize; j++)
	{
		for (long i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i + j * ACTIVEBKG->Xsize];

			if (eg->nbpoly)
			{
				player.pos.x = moveto.x = eg->polydata[0].center.x;
				player.pos.y = moveto.y = eg->polydata[0].center.y + PLAYER_BASE_HEIGHT;
				player.pos.z = moveto.z = eg->polydata[0].center.z;
			}
		}
	}
}

// Force Player to standard stance... (Need some improvements...)
void ARX_PLAYER_PutPlayerInNormalStance(long val)
{
	if (player.Current_Movement & PLAYER_CROUCH)
	{
		player.Current_Movement &= ~PLAYER_CROUCH;
	}

	player.Current_Movement = 0;
	ARX_PLAYER_RectifyPosition();

	if (player.jumpphase || player.falling)
	{
		player.physics.cyl.origin = player.pos + Vec3f(0.0f, 170.0f, 0.0f);

		IO_PHYSICS phys;
		memcpy(&phys, &player.physics, sizeof(IO_PHYSICS));
		AttemptValidCylinderPos(&phys.cyl, inter.iobj[0], CFLAG_RETURN_HEIGHT);
		player.pos.y = phys.cyl.origin.y - 170.f;
		player.jumpphase = 0;
		player.falling = 0;
	}

	if (player.Interface & INTER_COMBATMODE)
	{
		player.Interface &= ~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}

	ARX_SOUND_Stop(SND_MAGIC_DRAW);

	if (!val)
	{
		for(size_t i = 0; i < MAX_SPELLS; i++)
		{
			if (spells[i].exist && ((spells[i].caster == 0) || (spells[i].target == 0)))
			{
				switch (spells[i].type)
				{
				case SPELL_MAGIC_SIGHT:
				case SPELL_LEVITATE:
				case SPELL_SPEED:
				case SPELL_FLYING_EYE:
					spells[i].tolive = 0;
					break;
				default:
					break;
				}
			}
		}
	}
}

void ARX_PLAYER_Start_New_Quest()
{
	player.SKIN_MOD = 0;
	player.QUICK_MOD = 0;
	EERIE_PATHFINDER_Clear();
	EERIE_PATHFINDER_Release();
	player.hero_generate_fresh();
	player.CURRENT_TORCH = NULL;
	FreeAllInter();
	SecondaryInventory = NULL;
	TSecondaryInventory = NULL;
	ARX_EQUIPMENT_UnEquipAllPlayer();
	ARX_Changelevel_CurGame_Clear();
	inter.iobj[0]->halo.flags = 0;
}

void Manage_sp_max()
{
	float v = ARXTime - sp_max_start;

	if ((sp_max_start != 0) && (v < 20000))
	{
		float modi = (20000 - v) * (1.0f / 2000) * (1.0f / 10);
		float sizX = 16;
		float px = (float)DANAECENTERX - (float)sp_max_nb * (1.0f / 2) * sizX;
		float py = (float)DANAECENTERY;

		for (long i = 0; i < sp_max_nb; i++)
		{
			float dx = px + sizX * (float)i;
			float dy = py + sp_max_y[i];
			sp_max_y[i] = EEsin(dx + (float)ARXTime * (1.0f / 100)) * 30.f * modi;
			std::string tex(1, sp_max_ch[i]);

			UNICODE_ARXDrawTextCenter(hFontInBook, dx - 1, dy - 1, tex, Color::none);
			UNICODE_ARXDrawTextCenter(hFontInBook, dx + 1, dy + 1, tex, Color::none);
			UNICODE_ARXDrawTextCenter(hFontInBook, dx, dy, tex, sp_max_col[i]);
		}
	}
}
