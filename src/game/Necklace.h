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

#ifndef ARX_GAME_NECKLACE_H
#define ARX_GAME_NECKLACE_H

#include "game/Spells.h"

struct EERIE_3DOBJ;
class TextureContainer;

namespace arx
{
	class necklace
	{
public:
		necklace()
		{
			lacet = 0;
			for (int i = 0; i < RUNE_COUNT; i++)
			{
				runes[i] = 0;
				pTexTab[i] = 0;
			}
		}

		~necklace()
		{
			clear();
		}

		void clear();
		void init();

		EERIE_3DOBJ *lacet;
		EERIE_3DOBJ *runes[RUNE_COUNT];
		TextureContainer *pTexTab[RUNE_COUNT];
	};
};

#endif // ARX_GAME_NECKLACE_H
