/* ide-search-provider.h
 *
 * Copyright 2015-2017 Christian Hergert <chergert@redhat.com>
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

#include "ide-version-macros.h"

#include "ide-object.h"

G_BEGIN_DECLS

#define IDE_TYPE_SEARCH_PROVIDER (ide_search_provider_get_type())

IDE_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (IdeSearchProvider, ide_search_provider, IDE, SEARCH_PROVIDER, IdeObject)

struct _IdeSearchProviderInterface
{
  GTypeInterface parent_interface;

  void       (*search_async)  (IdeSearchProvider    *self,
                               const gchar          *query,
                               guint                 max_results,
                               GCancellable         *cancellable,
                               GAsyncReadyCallback   callback,
                               gpointer              user_data);
  GPtrArray *(*search_finish) (IdeSearchProvider    *self,
                               GAsyncResult         *result,
                               GError              **error);
};

IDE_AVAILABLE_IN_ALL
void       ide_search_provider_search_async  (IdeSearchProvider    *self,
                                              const gchar          *query,
                                              guint                 max_results,
                                              GCancellable         *cancellable,
                                              GAsyncReadyCallback   callback,
                                              gpointer              user_data);
IDE_AVAILABLE_IN_ALL
GPtrArray *ide_search_provider_search_finish (IdeSearchProvider    *self,
                                              GAsyncResult         *result,
                                              GError              **error);

G_END_DECLS
