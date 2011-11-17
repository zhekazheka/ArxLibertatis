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

#ifndef ARX_GAME_ATTRIBUTES_H
#define ARX_GAME_ATTRIBUTES_H

namespace arx
{
	struct attributes
	{
		float &operator[](const int &i)
		{
			return val[i];
		}

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
};

#endif // ARX_GAME_ATTRIBUTES_H