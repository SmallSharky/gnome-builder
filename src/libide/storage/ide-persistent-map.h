/* ide-persistent-map.h
 *
 * Copyright 2017 Anoop Chandu <anoopchandu96@gmail.com>
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

#include <gio/gio.h>

#include "ide-version-macros.h"

#define IDE_TYPE_PERSISTENT_MAP (ide_persistent_map_get_type ())

IDE_AVAILABLE_IN_3_30
G_DECLARE_FINAL_TYPE (IdePersistentMap, ide_persistent_map, IDE, PERSISTENT_MAP, GObject)

IDE_AVAILABLE_IN_3_30
IdePersistentMap *ide_persistent_map_new                        (void);
IDE_AVAILABLE_IN_3_30
gboolean          ide_persistent_map_load_file                  (IdePersistentMap     *self,
                                                                 GFile                *file,
                                                                 GCancellable         *cancellable,
                                                                 GError              **error);
IDE_AVAILABLE_IN_3_30
void              ide_persistent_map_load_file_async            (IdePersistentMap     *self,
                                                                 GFile                *file,
                                                                 GCancellable         *cancellable,
                                                                 GAsyncReadyCallback   callback,
                                                                 gpointer              user_data);
IDE_AVAILABLE_IN_3_30
gboolean          ide_persistent_map_load_file_finish           (IdePersistentMap     *self,
                                                                 GAsyncResult         *result,
                                                                 GError              **error);
IDE_AVAILABLE_IN_3_30
GVariant         *ide_persistent_map_lookup_value               (IdePersistentMap     *self,
                                                                 const gchar          *key);
IDE_AVAILABLE_IN_3_30
gint64            ide_persistent_map_builder_get_metadata_int64 (IdePersistentMap     *self,
                                                                 const gchar          *key);
