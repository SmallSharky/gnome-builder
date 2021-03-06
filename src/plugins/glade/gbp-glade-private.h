/* gbp-glade-private.h
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <ide.h>
#include <gladeui/glade.h>
#include <gladeui/glade-adaptor-chooser.h>

#include "gbp-glade-view.h"

G_BEGIN_DECLS

struct _GbpGladeView
{
  IdeLayoutView        parent_instance;

  GFile               *file;
  GladeProject        *project;
  DzlSignalGroup      *project_signals;

  GladeDesignView     *designer;
  GladeAdaptorChooser *chooser;
  GtkBox              *main_box;
};

void     _gbp_glade_view_init_actions   (GbpGladeView  *self);
void     _gbp_glade_view_init_shortcuts (GtkWidget     *widget);
void     _gbp_glade_view_update_actions (GbpGladeView  *self);
gboolean _gbp_glade_view_reload         (GbpGladeView  *self);
gboolean _gbp_glade_view_save           (GbpGladeView  *self,
                                         GError       **error);

G_END_DECLS
