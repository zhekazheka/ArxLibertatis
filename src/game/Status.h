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

#ifndef ARX_GAME_STATUS_H
#define ARX_GAME_STATUS_H

namespace arx
{	
	template <class T>
	T limit(const T &v, const T &l, const T &h)
	{
		return (v < l ? l : v > h ? h : v);
	}

	struct stats
	{
		void operator=(const float &v)
		{
			life = v;
			maxlife = v;
			mana = v;
			maxmana = v;
		}

		void limit_life()
		{
			life = arx::limit(life, 0.0f, maxlife);
		}

		void limit_mana()
		{
			mana = arx::limit(mana, 0.0f, maxmana);
		}

		void limit()
		{
			limit_life(); limit_mana();
		}

		float life;
		float maxlife;

		float mana;
		float maxmana;
	};
};

#endif // ARX_GAME_STATUS_H
