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

#include "game/Inventory.h"
#include "game/Player.h"
#include "graphics/math.h"
#include "graphics/particle/ParticleEffects.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "gui/Interface.h"

extern bool bBookHalo;
extern bool bGoldHalo;
extern unsigned long ulBookHaloTime;
extern unsigned long ulGoldHaloTime;
extern long cur_rf;
extern long cur_mr;
extern long SPECIAL_PNUX;

extern long sp_max;

extern long DANAESIZX;
extern long DANAESIZY;


float ARX_EQUIPMENT_ApplyPercent(INTERACTIVE_OBJ * io, long ident, float trueval);

// TODO really strange thing here is full is used in only some of the stats
float arx::character::get_stealth(bool modified)
{
	return skill.stealth + 
		full.attribute.dexterity * 2.0f +
		(modified ? mod.skill.stealth : 0.0f);
}

float arx::character::get_mecanism(bool modified)
{
	return skill.mecanism + 
		full.attribute.mind +
		full.attribute.dexterity +
		(modified ? mod.skill.mecanism : 0.0f);
}

float arx::character::get_intuition(bool modified)
{
	return skill.intuition + 
		full.attribute.mind * 2.0f +
		(modified ? mod.skill.intuition : 0.0f);
}

float arx::character::get_etheral_link(bool modified)
{
	return skill.etheral_link + 
		full.attribute.mind * 2.0f +
		(modified ? mod.skill.etheral_link : 0.0f);
}

// full attribute values not used here - verify as a bug?
float arx::character::get_object_knowledge(bool modified)
{
	return skill.object_knowledge + 
		attribute.mind * 3.0f +
		attribute.dexterity + 
		attribute.strength * (1.0f / 2.0f) +
		(modified ? mod.skill.object_knowledge : 0.0f);
}

float arx::character::get_casting(bool modified)
{
	return skill.casting + 
		(full.attribute.mind * 2.0f) +
		(modified ? mod.skill.casting : 0.0f);
}

float arx::character::get_projectile(bool modified)
{
	return skill.projectile + 
		full.attribute.strength +
		full.attribute.dexterity * 2.0f +
		(modified ? mod.skill.projectile : 0.0f);
}

float arx::character::get_close_combat(bool modified)
{
	return skill.close_combat + 
		full.attribute.dexterity + 
		full.attribute.strength * 2.0f +
		(modified ? mod.skill.close_combat : 0.0f);
}

float arx::character::get_defense(bool modified)
{
	return skill.defense + 
		full.attribute.constitution * 3.0f +
		(modified ? mod.skill.defense : 0.0f);
}

// Compute secondary attributes for player
void arx::character::compute_stats() 
{
	stat.maxlife = attribute.constitution * (level + 2);
	stat.maxmana = attribute.mind * (level + 1);

	float fCalc = get_defense() * (1.0f / 10.0f);
	armor_class = max((unsigned char)1, checked_range_cast<unsigned char>(fCalc));

	damages = 100;
	resist_magic = (unsigned char)(float)(attribute.mind * 2.f * (1.f + (get_casting()) * (1.0f / 200.0f)));

	fCalc = player.attribute.constitution * 2 + ((player.get_defense(true) * (1.0f / 4.0f)));
	resist_poison = checked_range_cast<unsigned char>(fCalc);
	damages = max(1.0f, (player.attribute.strength - 10) * (1.0f / 2.0f));
	aimtime = 1500;
}

// Compute FULL versions of player stats including Equiped Items and spells, and any other effect altering them.
void arx::character::compute_full_stats()
{
	compute_stats();

	mod.stat          = 0.0f;
	mod.attribute     = 0.0f;
	mod.skill         = 0.0f;
	mod.armor_class   = 0.0f;
	mod.resist_magic  = 0.0f;
	mod.resist_poison = 0.0f;
	mod.critical_hit  = 0.0f;
	mod.damages       = 0.0f;

	ARX_EQUIPMENT_IdentifyAll();

	full.weapon_type = ARX_EQUIPMENT_GetPlayerWeaponType();

	// Check for Equipment Modulators
	INTERACTIVE_OBJ *io = inter.iobj[0];
	mod.attribute.strength     = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_STRENGTH,         attribute.strength);
	mod.attribute.dexterity    = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_DEXTERITY,        attribute.dexterity);
	mod.attribute.constitution = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_CONSTITUTION,     attribute.constitution);
	mod.attribute.mind         = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_MIND,             attribute.mind);
	mod.armor_class            = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Armor_Class,      armor_class);
	mod.skill.stealth          = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Stealth,          get_stealth());
	mod.skill.mecanism         = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Mecanism,         get_mecanism());
	mod.skill.intuition        = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Intuition,        get_intuition());
	mod.skill.etheral_link     = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Etheral_Link,     get_etheral_link());
	mod.skill.object_knowledge = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Object_Knowledge, get_object_knowledge());
	mod.skill.casting          = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Casting,          get_casting());
	mod.skill.projectile       = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Projectile,       get_projectile());
	mod.skill.close_combat     = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Close_Combat,     get_close_combat());
	mod.skill.defense          = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Defense,          get_defense());
	mod.resist_magic           = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Resist_Magic,     resist_magic);
	mod.resist_poison          = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Resist_Poison,    resist_poison);
	mod.critical_hit           = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Critical_Hit,     critical_hit);
	mod.damages                = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_Damages,          0);

	//CHECK OVERFLOW
	float fFullAimTime  = ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_AimTime, 0);
	float fCalcHandicap = (full.attribute.dexterity - 10.f) * 20.f;

	//CAST
	full.aimtime = checked_range_cast<long>(fFullAimTime);
	if (full.aimtime <= 0) full.aimtime = aimtime;
	full.aimtime -= checked_range_cast<long>(fCalcHandicap);
	if (full.aimtime <= 1500) full.aimtime = 1500;

	// PERCENTILE.....
	// Check for Equipment modulators
	mod.attribute.strength     += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_STRENGTH,         attribute.strength + mod.attribute.strength);
	mod.attribute.dexterity    += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_DEXTERITY,        attribute.dexterity + mod.attribute.dexterity);
	mod.attribute.constitution += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_CONSTITUTION,     attribute.constitution + mod.attribute.constitution);
	mod.attribute.mind         += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_MIND,             attribute.mind + mod.attribute.mind);
	mod.armor_class            += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Armor_Class,      armor_class + mod.armor_class);
	mod.skill.stealth          += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Stealth,          get_stealth(true));
	mod.skill.mecanism         += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Mecanism,         get_mecanism(true));
	mod.skill.intuition        += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Intuition,        get_intuition(true));
	mod.skill.etheral_link     += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Etheral_Link,     get_etheral_link(true));
	mod.skill.object_knowledge += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Object_Knowledge, get_object_knowledge(true));
	mod.skill.casting          += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Casting,          get_casting(true));
	mod.skill.projectile       += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Projectile,       get_projectile(true));
	mod.skill.close_combat     += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Close_Combat,     get_close_combat(true));
	mod.skill.defense          += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Defense,          get_defense(true));
	mod.resist_magic           += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Resist_Magic,     resist_magic + mod.resist_magic);
	mod.resist_poison          += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Resist_Poison,    resist_poison + mod.resist_poison);
	mod.critical_hit           += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Critical_Hit,     critical_hit + mod.critical_hit);
	mod.damages                += ARX_EQUIPMENT_ApplyPercent(io, IO_EQUIPITEM_ELEMENT_Damages,          damages);
	//full.AimTime                = ARX_EQUIPMENT_ApplyPercent(io,IO_EQUIPITEM_ELEMENT_AimTime,0);

	// Check for Spell Modificators
	if (inter.iobj[0])
		for (long i = 0; i < inter.iobj[0]->nb_spells_on; i++)
		{
			long n = inter.iobj[0]->spells_on[i];

			if (spells[n].exist)
			{
				switch (spells[n].type)
				{
				case SPELL_ARMOR:
					mod.armor_class            += spells[n].caster_level;
					break;

				case SPELL_LOWER_ARMOR:
					mod.armor_class            -= spells[n].caster_level;
					break;

				case SPELL_CURSE:
					mod.attribute.strength     -= spells[n].caster_level;
					mod.attribute.constitution -= spells[n].caster_level;
					mod.attribute.dexterity    -= spells[n].caster_level;
					mod.attribute.mind         -= spells[n].caster_level;
					break;

				case SPELL_BLESS:
					mod.attribute.strength     += spells[n].caster_level;
					mod.attribute.dexterity    += spells[n].caster_level;
					mod.attribute.constitution += spells[n].caster_level;
					mod.attribute.mind         += spells[n].caster_level;
					break;

				default: 
					break;
				}
			}
		}

		if (cur_mr == 3)
		{
			mod.attribute.strength     += 1.0f;
			mod.attribute.mind         += 10.0f;
			mod.attribute.constitution += 1.0f;
			mod.attribute.dexterity    += 10.0f;
			mod.skill.stealth          += 5.0f;
			mod.skill.mecanism         += 5.0f;
			mod.skill.intuition        += 100.0f;
			mod.skill.etheral_link     += 100.0f;
			mod.skill.object_knowledge += 100.0f;
			mod.skill.casting          += 5.0f;
			mod.skill.projectile       += 5.0f;
			mod.skill.close_combat     += 5.0f;
			mod.skill.defense          += 100.0f;
			mod.resist_magic           += 100.0f;
			mod.resist_poison          += 100.0f;
			mod.critical_hit           += 5.0f;
			mod.damages                += 2.0f;
			mod.armor_class            += 100.0f;
			full.aimtime                = 100.0f;
		}

		if (sp_max)
		{
			mod.attribute     += 5.0f;
			mod.skill         += 50.0f;
			mod.resist_magic  += 10.0f;
			mod.resist_poison += 10.0f;
			mod.critical_hit  += 50.0f;
			mod.damages       += 10.0f;
			mod.armor_class   += 20.0f;
			full.aimtime       = 100.0f;
		}

		if (SPECIAL_PNUX)
		{
			mod.attribute.strength     += (long)(rnd() * 5.0f);
			mod.attribute.mind         += (long)(rnd() * 5.0f);
			mod.attribute.constitution += (long)(rnd() * 5.0f);
			mod.attribute.dexterity    += (long)(rnd() * 5.0f);
			mod.skill.stealth          += (long)(rnd() * 20.0f);
			mod.skill.mecanism         += (long)(rnd() * 20.0f);
			mod.skill.intuition        += (long)(rnd() * 20.0f);
			mod.skill.etheral_link     += (long)(rnd() * 20.0f);
			mod.skill.object_knowledge += (long)(rnd() * 20.0f);
			mod.skill.casting          += (long)(rnd() * 20.0f);
			mod.skill.projectile       += (long)(rnd() * 20.0f);
			mod.skill.close_combat     += (long)(rnd() * 20.0f);
			mod.skill.defense          += (long)(rnd() * 30.0f);
			mod.resist_magic           += (long)(rnd() * 20.0f);
			mod.resist_poison          += (long)(rnd() * 20.0f);
			mod.critical_hit           += (long)(rnd() * 20.0f);
			mod.damages                += (long)(rnd() * 20.0f);
			mod.armor_class            += (long)(rnd() * 20.0f);
		}

		if (cur_rf == 3)
		{
			mod.attribute.mind         += 10.0f;
			mod.skill.casting          += 100.0f;
			mod.skill.etheral_link     += 100.0f;
			mod.skill.object_knowledge += 100.0f;
			mod.resist_magic           += 20.0f;
			mod.resist_poison          += 20.0f;
			mod.damages                += 1.0f;
			mod.armor_class            += 5.0f;
		}

		full.armor_class            = max(0.0f, armor_class            + mod.armor_class);
		full.attribute.strength     = max(0.0f, attribute.strength     + mod.attribute.strength);
		full.attribute.mind         = max(0.0f, attribute.mind         + mod.attribute.mind);
		full.attribute.constitution = max(0.0f, attribute.constitution + mod.attribute.constitution);
		full.attribute.dexterity    = max(0.0f, attribute.dexterity    + mod.attribute.dexterity);

		full.skill.stealth          = get_stealth(true);
		full.skill.mecanism         = get_mecanism(true);
		full.skill.intuition        = get_intuition(true);
		full.skill.etheral_link     = get_etheral_link(true);
		full.skill.object_knowledge = get_object_knowledge(true);
		full.skill.casting          = get_casting(true);
		full.skill.projectile       = get_projectile(true);
		full.skill.close_combat     = get_close_combat(true);
		full.skill.defense          = get_defense(true);

		full.resist_magic           = max(0.0f, resist_magic  + mod.resist_magic);
		full.resist_poison          = max(0.0f, resist_poison + mod.resist_poison);

		full.critical_hit           = max(0.0f, critical_hit  + mod.critical_hit);

		full.damages                = max(1.0f, damages       + mod.damages + full.skill.close_combat * 1E-1f);

		full.stat.life              = stat.life;
		full.stat.mana              = stat.mana;

		full.stat.maxlife           = (level + 2.0f) * full.attribute.constitution + mod.stat.maxlife;
		full.stat.maxmana           = (level + 1.0f) * full.attribute.mind         + mod.stat.maxmana;

		stat.life                   = std::min(stat.life, full.stat.maxlife);
		stat.mana                   = std::min(stat.mana, full.stat.maxmana);
}

// Creates a Fresh hero
void arx::character::hero_generate_fresh()
{
	attribute = 6.0f;
	skill     = 0.0f;
	old       = 0.0f;

	redistribute.attribute = 16;
	redistribute.skill     = 18;

	level  = 0;
	xp     = get_xp_for_level(level);
	poison = 0.f;
	hunger = 100.f;
	skin   = 0;
	bag    = 1;

	compute_stats();

	rune_flags = 0;

	SpellToMemorize.bSpell = false;
}

void ARX_SPSound()
{
	ARX_SOUND_PlayCinematic("kra_zoha_equip.wav");
}

void arx::character::hero_generate_sp()
{
	ARX_SPSound();

	attribute = 12.0f;
	skill     = 5.0f;
	old       = 0.0f;

	redistribute.attribute = 6;
	redistribute.skill     = 10;

	level  = 1;
	xp     = get_xp_for_level(level);
	poison = 0.f;
	hunger = 100.f;
	skin   = 4;

	compute_stats();
	stat.life = stat.maxlife;
	stat.mana = stat.maxmana;

	rune_flags = RuneFlags::all();
	SpellToMemorize.bSpell = false;

	SKIN_MOD  = 0;
	QUICK_MOD = 0;
}

void arx::character::hero_generate_powerful()
{
	attribute = 18.0f;
	skill     = 82.0f;
	old       = 0.0f;

	redistribute.attribute = 0;
	redistribute.skill     = 0;

	level  = 10;
	xp     = get_xp_for_level(level);
	poison = 0.f;
	hunger = 100.f;
	skin   = 0;

	compute_stats();
	stat.life = player.stat.maxlife;
	stat.mana = player.stat.maxmana;

	rune_flags = RuneFlags::all();
	SpellToMemorize.bSpell = false;
}

void arx::character::hero_generate_average()
{
	hero_generate_fresh();

	attribute = 10.0f;
	skill     = 2.0f;

	redistribute.attribute = 0;
	redistribute.skill     = 0;

	level  = 0;
	xp     = get_xp_for_level(level);
	hunger = 100.f;

	compute_stats();
}

// Quickgenerate a random hero
void arx::character::hero_generate_random() 
{
	char old_skin = skin;
	hero_generate_fresh();
	skin = old_skin;

	while (redistribute.attribute)
	{
		unsigned int i = rand() % 4;
		if (attribute.v[i] < 18)
		{
			attribute.v[i]++;
			redistribute.attribute--;
		}
	}

	while (redistribute.skill)
	{
		unsigned int i = rand() % 9;
		if (skill.v[i] < 18)
		{
			skill.v[i]++;
			redistribute.skill--;
		}
	}

	level  = 0;
	xp     = get_xp_for_level(level);
	hunger = 100.f;

	compute_stats();
}

// Returns necessary Experience for a given level
int arx::character::get_xp_for_level(const int &level)
{
	int level_xp[] =
	{
		0, 
		2000,   4000,   6000,   10000,  16000,  26000,  42000, 
		68000,  110000, 178000, 300000, 450000, 600000, 750000,
	};

	return (level < 15 ? level_xp[level] : level * 60000);
}

// Manages Player Level Up event
void arx::character::level_up()
{
	ARX_SOUND_PlayInterface(SND_PLAYER_LEVEL_UP);

	level++;

	redistribute.skill     += 15;
	redistribute.attribute += 1;

	compute_stats();

	stat.life = stat.maxlife;
	stat.mana = stat.maxmana;

	old = skill;

	SendIOScriptEvent(inter.iobj[0], SM_NULL, "", "level_up");
}

// add "val" to player xp
void arx::character::add_xp(const int &v) 
{
	xp += v;

	while (xp >= get_xp_for_level(level + 1)) 
	{
		level_up();
	}
}

// Function to poison player by "val" poison level
void arx::character::add_poison(const float &val)
{
	// Make a poison saving throw to see if player is affected
	if (resist_poison < rnd() * 100.0f)
	{
		poison += val;
		ARX_SOUND_PlayInterface(SND_PLAYER_POISONED);
	}
}

void arx::character::add_gold(const int &v) 
{
	player.gold += v;
	bGoldHalo = true;
	ulGoldHaloTime = 0;
}

void arx::character::add_gold(INTERACTIVE_OBJ *gold) 
{
	arx_assert(gold->ioflags & IO_GOLD);

	add_gold(gold->_itemdata->price * max((short)1, gold->_itemdata->count));

	ARX_SOUND_PlayInterface(SND_GOLD);

	if (gold->scriptload) 
	{
		RemoveFromAllInventories(gold);
		ReleaseInter(gold);
	} else 
	{
		gold->show = SHOW_FLAG_KILLED;
		gold->GameFlags &= ~GFLAG_ISINTREATZONE;
	}
}

void arx::character::add_bag()
{
	if (bag < 3)
	{
		player.bag++; 
	}
}

bool arx::character::can_steal(INTERACTIVE_OBJ *_io)
{
	return (_io->_itemdata->stealvalue > 0 &&
				full.skill.stealth >= _io->_itemdata->stealvalue && 
				_io->_itemdata->stealvalue < 100.f);
}

// Add _ulRune to player runes
void arx::character::add_rune(RuneFlag _ulRune)
{
	int iNbSpells = 0;
	int iNbSpellsAfter = 0;

	for (size_t i = 0; i < SPELL_COUNT; i++)
	{
		if (spellicons[i].bSecret == false)
		{
			long j = 0;
			bool bOk = true;

			while ((j < 4) && (spellicons[i].symbols[j] != 255))
			{
				if (!(rune_flags & (RuneFlag)(1 << spellicons[i].symbols[j])))
				{
					bOk = false;
				}

				j++;
			}

			if (bOk)
			{
				iNbSpells ++;
			}
		}
	}

	rune_flags |= _ulRune;

	for (size_t i = 0; i < SPELL_COUNT; i++)
	{
		if (spellicons[i].bSecret == false)
		{
			long j = 0;
			bool bOk = true;

			while ((j < 4) && (spellicons[i].symbols[j] != 255))
			{
				if (!(rune_flags & (RuneFlag)(1 << spellicons[i].symbols[j])))
				{
					bOk = false;
				}

				j++;
			}

			if (bOk)
			{
				iNbSpellsAfter ++;
			}
		}
	}

	if (iNbSpellsAfter > iNbSpells)
	{
		MakeBookFX(DANAESIZX - INTERFACE_RATIO(35), DANAESIZY - INTERFACE_RATIO(148), 0.00001f);
		bBookHalo = true;
		ulBookHaloTime = 0;
	}
}

// Remove _ulRune from player runes
void arx::character::remove_rune(RuneFlag _ulRune)
{
	rune_flags &= ~_ulRune;
}

void arx::character::add_all_runes()
{
	RuneFlag all_runes[20] =
	{
		FLAG_AAM, FLAG_CETRIUS, FLAG_COMUNICATUM, FLAG_COSUM,
		FLAG_FOLGORA, FLAG_FRIDD, FLAG_KAOM, FLAG_MEGA,
		FLAG_MORTE, FLAG_MOVIS, FLAG_NHI, FLAG_RHAA,
		FLAG_SPACIUM, FLAG_STREGUM, FLAG_TAAR, FLAG_TEMPUS,
		FLAG_TERA, FLAG_VISTA, FLAG_VITAE, FLAG_YOK,
	};

	for (int i = 0; i < 20; i++)
	{
		add_rune(all_runes[i]);
	}
}

void arx::character::set_invulnerable(const bool &b)
{
	if (b)
	{
		playerflags |= PLAYERFLAGS_INVULNERABILITY;
	} else
	{
		playerflags &= ~PLAYERFLAGS_INVULNERABILITY;
	}
}

// Init Local Player Data
void arx::character::init() 
{
	Interface = INTER_MINIBOOK | INTER_MINIBACK | INTER_LIFE_MANA;
	physics.cyl.height = PLAYER_BASE_HEIGHT;
	physics.cyl.radius = PLAYER_BASE_RADIUS;
	stat.life = stat.maxlife = full.stat.maxlife = 100.f;
	stat.mana = stat.maxmana = full.stat.maxmana = 100.f;
	falling = 0;
	rightIO = NULL;
	leftIO = NULL;
	equipsecondaryIO = NULL;
	equipshieldIO = NULL;
	gold = 0;
	bag = 1;
	doingmagic = 0;
	hero_generate_fresh();
}
