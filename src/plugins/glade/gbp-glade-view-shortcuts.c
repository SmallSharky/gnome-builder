/* gbp-glade-view-shortcuts.c
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

#include "config.h"

#define G_LOG_DOMAIN "gbp-glade-view-shortcuts"

#include <dazzle.h>
#include <glib/gi18n.h>

#include "gbp-glade-private.h"
#include "gbp-glade-view.h"

#define I_(s) (g_intern_static_string(s))

static DzlShortcutEntry glade_view_shortcuts[] = {
  { "org.gnome.builder.glade-view.save",
    0, NULL,
    NC_("shortcut window", "Glade shortcuts"),
    NC_("shortcut window", "Designer"),
    NC_("shortcut window", "Save the interface design") },

  { "org.gnome.builder.glade-view.preview",
    0, NULL,
    NC_("shortcut window", "Glade shortcuts"),
    NC_("shortcut window", "Designer"),
    NC_("shortcut window", "Preview the interface design") },
};

void
_gbp_glade_view_init_shortcuts (GtkWidget *widget)
{
  DzlShortcutController *controller;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  controller = dzl_shortcut_controller_find (widget);

  dzl_shortcut_controller_add_command_action (controller,
                                              I_("org.gnome.builder.glade-view.save"),
                                              "<Primary>s",
                                              DZL_SHORTCUT_PHASE_BUBBLE,
                                              I_("glade-view.save"));

  dzl_shortcut_controller_add_command_action (controller,
                                              I_("org.gnome.builder.glade-view.preview"),
                                              "<Control><Alt>p",
                                              DZL_SHORTCUT_PHASE_BUBBLE,
                                              I_("glade-view.preview"));

  dzl_shortcut_manager_add_shortcut_entries (NULL,
                                             glade_view_shortcuts,
                                             G_N_ELEMENTS (glade_view_shortcuts),
                                             GETTEXT_PACKAGE);
}
