
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

#include "platform/Flags.h"

enum PlayerMovementFlag {
	PLAYER_MOVE_WALK_FORWARD  = (1<<0),
	PLAYER_MOVE_WALK_BACKWARD = (1<<1),
	PLAYER_MOVE_STRAFE_LEFT   = (1<<2),
	PLAYER_MOVE_STRAFE_RIGHT  = (1<<3),
	PLAYER_MOVE_JUMP          = (1<<4),
	PLAYER_MOVE_STEALTH       = (1<<5),
	PLAYER_ROTATE             = (1<<6),
	PLAYER_CROUCH             = (1<<7),
	PLAYER_LEAN_LEFT          = (1<<8),
	PLAYER_LEAN_RIGHT         = (1<<9)
};
DECLARE_FLAGS(PlayerMovementFlag, PlayerMovement)
DECLARE_FLAGS_OPERATORS(PlayerMovement)

enum PlayerFlag {
	PLAYERFLAGS_NO_MANA_DRAIN   = (1<<0),
	PLAYERFLAGS_INVULNERABILITY = (1<<1)
};
DECLARE_FLAGS(PlayerFlag, PlayerFlags)
DECLARE_FLAGS_OPERATORS(PlayerFlags)
