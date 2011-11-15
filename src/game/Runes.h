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

#ifndef ARX_GAME_RUNES_H
#define ARX_GAME_RUNES_H

#include "platform/Flags.h"

enum RuneFlag {
	FLAG_AAM         = (1<<(RUNE_AAM)),
	FLAG_CETRIUS     = (1<<(RUNE_CETRIUS)),
	FLAG_COMUNICATUM = (1<<(RUNE_COMUNICATUM)),
	FLAG_COSUM       = (1<<(RUNE_COSUM)),
	FLAG_FOLGORA     = (1<<(RUNE_FOLGORA)),
	FLAG_FRIDD       = (1<<(RUNE_FRIDD)),
	FLAG_KAOM        = (1<<(RUNE_KAOM)),
	FLAG_MEGA        = (1<<(RUNE_MEGA)),
	FLAG_MORTE       = (1<<(RUNE_MORTE)),
	FLAG_MOVIS       = (1<<(RUNE_MOVIS)),
	FLAG_NHI         = (1<<(RUNE_NHI)),
	FLAG_RHAA        = (1<<(RUNE_RHAA)),
	FLAG_SPACIUM     = (1<<(RUNE_SPACIUM)),
	FLAG_STREGUM     = (1<<(RUNE_STREGUM)),
	FLAG_TAAR        = (1<<(RUNE_TAAR)),
	FLAG_TEMPUS      = (1<<(RUNE_TEMPUS)),
	FLAG_TERA        = (1<<(RUNE_TERA)),
	FLAG_VISTA       = (1<<(RUNE_VISTA)),
	FLAG_VITAE       = (1<<(RUNE_VITAE)),
	FLAG_YOK         = (1<<(RUNE_YOK))
};

DECLARE_FLAGS(RuneFlag, RuneFlags)
DECLARE_FLAGS_OPERATORS(RuneFlags)

#endif // ARX_GAME_RUNES_H
