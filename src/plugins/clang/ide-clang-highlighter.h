/* ide-clang-highlighter.h
 *
 * Copyright 2015 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <ide.h>

G_BEGIN_DECLS

#define IDE_TYPE_CLANG_HIGHLIGHTER (ide_clang_highlighter_get_type())

#define IDE_CLANG_HIGHLIGHTER_TYPE          "c:type"
#define IDE_CLANG_HIGHLIGHTER_FUNCTION_NAME "def:function"
#define IDE_CLANG_HIGHLIGHTER_ENUM_NAME     "def:constant"
#define IDE_CLANG_HIGHLIGHTER_MACRO_NAME    "c:macro-name"

G_DECLARE_FINAL_TYPE (IdeClangHighlighter, ide_clang_highlighter, IDE, CLANG_HIGHLIGHTER, IdeObject)

G_END_DECLS
