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

#include <string>
#include <vector>

#include "graphics/data/Mesh.h"

namespace arx
{
	class keyring
	{
	public:

		struct KEYRING_SLOT {
			void init()
			{
				for (int i = 0; i < 64; i++)
				{
					slot[i] = 0;
				}
			}
			char slot[64];
		};

		keyring() {}
		~keyring() {}
	
		// Init/Reset player Keyring structures
		void init()
		{
			ring.clear();
		}
		
		// Add a key to Keyring
		void add(const std::string &key)
		{
			ring.resize(ring.size() + 1);
			ring.back().init();
			strcpy(ring.back().slot, key.c_str());
		}

		// Sends COMBINE event to "io" for each keyring entry
		void combine(INTERACTIVE_OBJ * io)
		{
			for (int i = 0; i < ring.size(); i++) 
			{
				if (SendIOScriptEvent(io, SM_COMBINE, ring[i].slot) == REFUSE) 
				{
					break;
				}
			}
		}

		std::vector<KEYRING_SLOT> ring;
	};
};
