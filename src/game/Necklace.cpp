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
 
#include "game/Necklace.h"

#include "game/Spells.h"
#include "scene/Object.h"
#include "graphics/data/TextureContainer.h"

void arx::necklace::clear()
{
	if (lacet)
	{
		delete lacet;
		lacet = NULL;
	}

	for (long i=0; i<20; i++)
	{
		if (runes[i]) 
		{
			delete runes[i];
			runes[i] = NULL;
		}

		if (pTexTab[i])
		{
			pTexTab[i] = NULL;
		}
	}
}

void arx::necklace::init()
{
	lacet = loadObject("graph/interface/book/runes/lacet.teo");
	
	runes[RUNE_AAM]         = loadObject("graph/interface/book/runes/runes_aam.teo");
	runes[RUNE_CETRIUS]     = loadObject("graph/interface/book/runes/runes_citrius.teo");
	runes[RUNE_COMUNICATUM] = loadObject("graph/interface/book/runes/runes_comunicatum.teo");
	runes[RUNE_COSUM]       = loadObject("graph/interface/book/runes/runes_cosum.teo");
	runes[RUNE_FOLGORA]     = loadObject("graph/interface/book/runes/runes_folgora.teo");
	runes[RUNE_FRIDD]       = loadObject("graph/interface/book/runes/runes_fridd.teo");
	runes[RUNE_KAOM]        = loadObject("graph/interface/book/runes/runes_kaom.teo");
	runes[RUNE_MEGA]        = loadObject("graph/interface/book/runes/runes_mega.teo");
	runes[RUNE_MORTE]       = loadObject("graph/interface/book/runes/runes_morte.teo");
	runes[RUNE_MOVIS]       = loadObject("graph/interface/book/runes/runes_movis.teo");
	runes[RUNE_NHI]         = loadObject("graph/interface/book/runes/runes_nhi.teo");
	runes[RUNE_RHAA]        = loadObject("graph/interface/book/runes/runes_rhaa.teo");
	runes[RUNE_SPACIUM]     = loadObject("graph/interface/book/runes/runes_spacium.teo");
	runes[RUNE_STREGUM]     = loadObject("graph/interface/book/runes/runes_stregum.teo");
	runes[RUNE_TAAR]        = loadObject("graph/interface/book/runes/runes_taar.teo");
	runes[RUNE_TEMPUS]      = loadObject("graph/interface/book/runes/runes_tempus.teo");
	runes[RUNE_TERA]        = loadObject("graph/interface/book/runes/runes_tera.teo");
	runes[RUNE_VISTA]       = loadObject("graph/interface/book/runes/runes_vista.teo");
	runes[RUNE_VITAE]       = loadObject("graph/interface/book/runes/runes_vitae.teo");
	runes[RUNE_YOK]         = loadObject("graph/interface/book/runes/runes_yok.teo");
	
	pTexTab[RUNE_AAM]         = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_aam[icon]");
	pTexTab[RUNE_CETRIUS]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_cetrius[icon]");
	pTexTab[RUNE_COMUNICATUM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_comunicatum[icon]");
	pTexTab[RUNE_COSUM]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_cosum[icon]");
	pTexTab[RUNE_FOLGORA]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_folgora[icon]");
	pTexTab[RUNE_FRIDD]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_fridd[icon]");
	pTexTab[RUNE_KAOM]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_kaom[icon]");
	pTexTab[RUNE_MEGA]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_mega[icon]");
	pTexTab[RUNE_MORTE]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_morte[icon]");
	pTexTab[RUNE_MOVIS]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_movis[icon]");
	pTexTab[RUNE_NHI]         = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_nhi[icon]");
	pTexTab[RUNE_RHAA]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_rhaa[icon]");
	pTexTab[RUNE_SPACIUM]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_spacium[icon]");
	pTexTab[RUNE_STREGUM]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_stregum[icon]");
	pTexTab[RUNE_TAAR]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_taar[icon]");
	pTexTab[RUNE_TEMPUS]      = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_tempus[icon]");
	pTexTab[RUNE_TERA]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_tera[icon]");
	pTexTab[RUNE_VISTA]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_vista[icon]");
	pTexTab[RUNE_VITAE]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_vitae[icon]");
	pTexTab[RUNE_YOK]         = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_yok[icon]");
	
	// TODO why -1?
	// adejr: last rune is RUNE_AKBAA, which doesn't require a halo as 
	// it is never displayed to the user in the questbook.
	// although i don't see the harm in creating one, i assume it would be useless.
	for (size_t i = 0; i < RUNE_COUNT - 1; i++) 
	{ 
		if (pTexTab[i]) 
		{
			pTexTab[i]->CreateHalo();
		}
	}
}
