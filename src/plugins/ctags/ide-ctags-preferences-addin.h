/* ide-ctags-preferences-addin.h
 *
 * Copyright 2018 Christian Hergert <chergert@redhat.com>
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

#include <glib-object.h>

G_BEGIN_DECLS

#define IDE_TYPE_CTAGS_PREFERENCES_ADDIN (ide_ctags_preferences_addin_get_type())

G_DECLARE_FINAL_TYPE (IdeCtagsPreferencesAddin, ide_ctags_preferences_addin, IDE, CTAGS_PREFERENCES_ADDIN, GObject)

G_END_DECLS
