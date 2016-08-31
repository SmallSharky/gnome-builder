/* ide-editor-view-addin-private.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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

#ifndef IDE_EDITOR_VIEW_ADDIN_PRIVATE_H
#define IDE_EDITOR_VIEW_ADDIN_PRIVATE_H

#include "editor/ide-editor-view-addin.h"
#include "sourceview/ide-source-view.h"

G_BEGIN_DECLS

void ide_editor_view_addin_language_changed   (IdeEditorViewAddin *self,
                                               const gchar        *language_id);
void ide_editor_view_addin_load               (IdeEditorViewAddin *self,
                                               IdeEditorView      *view);
void ide_editor_view_addin_unload             (IdeEditorViewAddin *self,
                                               IdeEditorView      *view);
void ide_editor_view_addin_load_source_view   (IdeEditorViewAddin *self,
                                               IdeSourceView      *source_view);
void ide_editor_view_addin_unload_source_view (IdeEditorViewAddin *self,
                                               IdeSourceView      *source_view);
G_END_DECLS

#endif /* IDE_EDITOR_VIEW_ADDIN_PRIVATE_H */
