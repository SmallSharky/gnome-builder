/* ide-langserv-client.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "ide-langserv-client"

#include <egg-counter.h>
#include <egg-signal-group.h>
#include <jsonrpc-glib.h>
#include <unistd.h>

#include "ide-context.h"
#include "ide-debug.h"

#include "buffers/ide-buffer.h"
#include "buffers/ide-buffer-manager.h"
#include "diagnostics/ide-diagnostic.h"
#include "diagnostics/ide-diagnostics.h"
#include "langserv/ide-langserv-client.h"
#include "projects/ide-project.h"
#include "vcs/ide-vcs.h"

typedef struct
{
  EggSignalGroup *buffer_manager_signals;
  EggSignalGroup *project_signals;
  JsonrpcClient  *rpc_client;
  GIOStream      *io_stream;
  GHashTable     *diagnostics_by_uri;
} IdeLangservClientPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (IdeLangservClient, ide_langserv_client, IDE_TYPE_OBJECT)

enum {
  FILE_CHANGE_TYPE_CREATED = 1,
  FILE_CHANGE_TYPE_CHANGED = 2,
  FILE_CHANGE_TYPE_DELETED = 3,
};

enum {
  PROP_0,
  PROP_IO_STREAM,
  N_PROPS
};

enum {
  NOTIFICATION,
  N_SIGNALS
};

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static void
ide_langserv_client_buffer_loaded (IdeLangservClient *self,
                                   IdeBuffer         *buffer,
                                   IdeBufferManager  *buffer_manager)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);
  g_autoptr(JsonNode) params = NULL;
  g_autofree gchar *uri = NULL;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (IDE_IS_BUFFER (buffer));
  g_assert (IDE_IS_BUFFER_MANAGER (buffer_manager));

  uri = ide_buffer_get_uri (buffer);

  params = JCON_NEW (
    "textDocument", "{",
      "uri", JCON_STRING (uri),
    "}"
  );

  jsonrpc_client_notification_async (priv->rpc_client, "textDocument/didOpen", params, NULL, NULL, NULL);
}

static void
ide_langserv_client_buffer_unloaded (IdeLangservClient *self,
                                     IdeBuffer         *buffer,
                                     IdeBufferManager  *buffer_manager)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);
  g_autoptr(JsonNode) params = NULL;
  g_autofree gchar *uri = NULL;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (IDE_IS_BUFFER (buffer));
  g_assert (IDE_IS_BUFFER_MANAGER (buffer_manager));

  uri = ide_buffer_get_uri (buffer);

  params = JCON_NEW (
    "textDocument", "{",
      "uri", JCON_STRING (uri),
    "}"
  );

  jsonrpc_client_notification_async (priv->rpc_client, "textDocument/didClose", params, NULL, NULL, NULL);
}

static void
ide_langserv_client_buffer_manager_bind (IdeLangservClient *self,
                                         IdeBufferManager  *buffer_manager,
                                         EggSignalGroup    *signal_group)
{
  guint n_items;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (IDE_IS_BUFFER_MANAGER (buffer_manager));
  g_assert (EGG_IS_SIGNAL_GROUP (signal_group));

  n_items = g_list_model_get_n_items (G_LIST_MODEL (buffer_manager));

  for (guint i = 0; i < n_items; i++)
    {
      g_autoptr(IdeBuffer) buffer = NULL;

      buffer = g_list_model_get_item (G_LIST_MODEL (buffer_manager), i);
      ide_langserv_client_buffer_loaded (self, buffer, buffer_manager);
    }
}

static void
ide_langserv_client_buffer_manager_unbind (IdeLangservClient *self,
                                           EggSignalGroup    *signal_group)
{
  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (EGG_IS_SIGNAL_GROUP (signal_group));

  /* TODO: We need to track everything we've notified so that we
   *       can notify the peer to release its resources.
   */
}

static void
ide_langserv_client_project_file_trashed (IdeLangservClient *self,
                                          GFile             *file,
                                          IdeProject        *project)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);
  g_autoptr(JsonNode) params = NULL;
  g_autofree gchar *uri = NULL;

  IDE_ENTRY;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (G_IS_FILE (file));
  g_assert (IDE_IS_PROJECT (project));

  uri = g_file_get_uri (file);

  params = JCON_NEW (
    "changes", "["
      "{",
        "uri", JCON_STRING (uri),
        "type", JCON_INT (FILE_CHANGE_TYPE_DELETED),
      "}",
    "]"
  );

  jsonrpc_client_notification_async (priv->rpc_client, "workspace/didChangeWatchedFiles", params, NULL, NULL, NULL);

  IDE_EXIT;
}

static void
ide_langserv_client_project_file_renamed (IdeLangservClient *self,
                                          GFile             *src,
                                          GFile             *dst,
                                          IdeProject        *project)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);
  g_autoptr(JsonNode) params = NULL;
  g_autofree gchar *src_uri = NULL;
  g_autofree gchar *dst_uri = NULL;

  IDE_ENTRY;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (G_IS_FILE (src));
  g_assert (G_IS_FILE (dst));
  g_assert (IDE_IS_PROJECT (project));

  src_uri = g_file_get_uri (src);
  dst_uri = g_file_get_uri (dst);

  params = JCON_NEW (
    "changes", "["
      "{",
        "uri", JCON_STRING (src_uri),
        "type", JCON_INT (FILE_CHANGE_TYPE_DELETED),
      "}",
      "{",
        "uri", JCON_STRING (dst_uri),
        "type", JCON_INT (FILE_CHANGE_TYPE_CREATED),
      "}",
    "]"
  );

  jsonrpc_client_notification_async (priv->rpc_client, "workspace/didChangeWatchedFiles", params, NULL, NULL, NULL);

  IDE_EXIT;
}

static void
ide_langserv_client_text_document_publish_diagnostics (IdeLangservClient *self,
                                                       JsonNode          *params)
{
  const gchar *uri = NULL;
  JsonArray *diagnostics = NULL;
  gboolean success;

  IDE_ENTRY;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (params != NULL);

  success = JCON_EXTRACT (params,
    "uri", JCONE_STRING (uri),
    "diagnostics", JCONE_ARRAY (diagnostics)
  );

  if (!success)
    IDE_EXIT;

  IDE_TRACE_MSG ("Diagnostics received for %s", uri);

  IDE_EXIT;
}

static void
ide_langserv_client_real_notification (IdeLangservClient *self,
                                       const gchar       *method,
                                       JsonNode          *params)
{
  IDE_ENTRY;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (method != NULL);
  g_assert (params != NULL);

  if (g_str_equal (method, "textDocument/publishDiagnostics"))
    ide_langserv_client_text_document_publish_diagnostics (self, params);

  IDE_EXIT;
}

static void
ide_langserv_client_notification (IdeLangservClient *self,
                                  const gchar       *method,
                                  JsonNode          *params,
                                  JsonrpcClient     *rpc_client)
{
  GQuark detail;

  IDE_ENTRY;

  g_assert (IDE_IS_LANGSERV_CLIENT (self));
  g_assert (method != NULL);
  g_assert (params != NULL);
  g_assert (rpc_client != NULL);

  IDE_TRACE_MSG ("Notification: %s", method);

  /*
   * To avoid leaking quarks we do not create a quark for the string unless
   * it already exists. This should be fine in practice because we only need
   * the quark if there is a caller that has registered for it. And the callers
   * registering for it will necessarily create the quark.
   */
  detail = g_quark_try_string (method);

  g_signal_emit (self, signals [NOTIFICATION], detail, method, params);

  IDE_EXIT;
}

static void
ide_langserv_client_finalize (GObject *object)
{
  IdeLangservClient *self = (IdeLangservClient *)object;
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);

  g_clear_pointer (&priv->diagnostics_by_uri, g_hash_table_unref);
  g_clear_object (&priv->rpc_client);
  g_clear_object (&priv->buffer_manager_signals);
  g_clear_object (&priv->project_signals);

  G_OBJECT_CLASS (ide_langserv_client_parent_class)->finalize (object);
}

static void
ide_langserv_client_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  IdeLangservClient *self = IDE_LANGSERV_CLIENT (object);
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_IO_STREAM:
      g_value_set_object (value, priv->io_stream);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_langserv_client_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  IdeLangservClient *self = IDE_LANGSERV_CLIENT (object);
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_IO_STREAM:
      priv->io_stream = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_langserv_client_class_init (IdeLangservClientClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_langserv_client_finalize;
  object_class->get_property = ide_langserv_client_get_property;
  object_class->set_property = ide_langserv_client_set_property;

  klass->notification = ide_langserv_client_real_notification;

  properties [PROP_IO_STREAM] =
    g_param_spec_object ("io-stream",
                         "IO Stream",
                         "The GIOStream to communicate over",
                         G_TYPE_IO_STREAM,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  signals [NOTIFICATION] =
    g_signal_new ("notification",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  G_STRUCT_OFFSET (IdeLangservClientClass, notification),
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                  JSON_TYPE_NODE);
}

static void
ide_langserv_client_init (IdeLangservClient *self)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);

  priv->diagnostics_by_uri = g_hash_table_new_full (g_str_hash,
                                                    g_str_equal,
                                                    g_free,
                                                    (GDestroyNotify)ide_diagnostics_unref);

  priv->buffer_manager_signals = egg_signal_group_new (IDE_TYPE_BUFFER_MANAGER);

  egg_signal_group_connect_object (priv->buffer_manager_signals,
                                   "buffer-loaded",
                                   G_CALLBACK (ide_langserv_client_buffer_loaded),
                                   self,
                                   G_CONNECT_SWAPPED);
  egg_signal_group_connect_object (priv->buffer_manager_signals,
                                   "buffer-unloaded",
                                   G_CALLBACK (ide_langserv_client_buffer_unloaded),
                                   self,
                                   G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->buffer_manager_signals,
                           "bind",
                           G_CALLBACK (ide_langserv_client_buffer_manager_bind),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->buffer_manager_signals,
                           "unbind",
                           G_CALLBACK (ide_langserv_client_buffer_manager_unbind),
                           self,
                           G_CONNECT_SWAPPED);

  priv->project_signals = egg_signal_group_new (IDE_TYPE_PROJECT);

  egg_signal_group_connect_object (priv->project_signals,
                                   "file-trashed",
                                   G_CALLBACK (ide_langserv_client_project_file_trashed),
                                   self,
                                   G_CONNECT_SWAPPED);
  egg_signal_group_connect_object (priv->project_signals,
                                   "file-renamed",
                                   G_CALLBACK (ide_langserv_client_project_file_renamed),
                                   self,
                                   G_CONNECT_SWAPPED);
}

static void
ide_langserv_client_initialize_cb (GObject      *object,
                                   GAsyncResult *result,
                                   gpointer      user_data)
{
  JsonrpcClient *rpc_client = (JsonrpcClient *)object;
  g_autoptr(IdeLangservClient) self = user_data;
  g_autoptr(JsonNode) reply = NULL;
  g_autoptr(GError) error = NULL;

  g_assert (JSONRPC_IS_CLIENT (rpc_client));
  g_assert (G_IS_ASYNC_RESULT (result));
  g_assert (IDE_IS_LANGSERV_CLIENT (self));

  if (!jsonrpc_client_call_finish (rpc_client, result, &reply, &error))
    {
      g_warning ("Failed to initialize language server: %s", error->message);
      ide_langserv_client_stop (self);
      return;
    }

  /* TODO: Check for server capabilities */
}

IdeLangservClient *
ide_langserv_client_new (IdeContext *context,
                         GIOStream  *io_stream)
{
  g_return_val_if_fail (IDE_IS_CONTEXT (context), NULL);

  return g_object_new (IDE_TYPE_LANGSERV_CLIENT,
                       "context", context,
                       "io-stream", io_stream,
                       NULL);
}

void
ide_langserv_client_start (IdeLangservClient *self)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);
  g_autoptr(JsonNode) params = NULL;
  g_autofree gchar *root_path = NULL;
  IdeBufferManager *buffer_manager;
  IdeContext *context;
  IdeProject *project;
  IdeVcs *vcs;
  GFile *workdir;

  IDE_ENTRY;

  g_return_if_fail (IDE_IS_LANGSERV_CLIENT (self));

  context = ide_object_get_context (IDE_OBJECT (self));

  if (!G_IS_IO_STREAM (priv->io_stream) || !IDE_IS_CONTEXT (context))
    {
      g_warning ("Cannot start %s due to misconfiguration.",
                 G_OBJECT_TYPE_NAME (self));
      return;
    }

  priv->rpc_client = jsonrpc_client_new (priv->io_stream);

  g_signal_connect_object (priv->rpc_client,
                           "notification",
                           G_CALLBACK (ide_langserv_client_notification),
                           self,
                           G_CONNECT_SWAPPED);


  buffer_manager = ide_context_get_buffer_manager (context);
  egg_signal_group_set_target (priv->buffer_manager_signals, buffer_manager);

  project = ide_context_get_project (context);
  egg_signal_group_set_target (priv->project_signals, project);

  vcs = ide_context_get_vcs (context);
  workdir = ide_vcs_get_working_directory (vcs);
  root_path = g_file_get_path (workdir);

  /*
   * The first thing we need to do is initialize the client with information
   * about our project. So that we will perform asynchronously here. It will
   * also start our read loop.
   */

  params = JCON_NEW (
    "processId", JCON_INT (getpid ()),
    "rootPath", JCON_STRING (root_path),
    "capabilities", "{", "}"
  );

  jsonrpc_client_call_async (priv->rpc_client, "initialize", params, NULL,
                             ide_langserv_client_initialize_cb,
                             g_object_ref (self));

  IDE_EXIT;
}

void
ide_langserv_client_stop (IdeLangservClient *self)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);

  IDE_ENTRY;

  g_return_if_fail (IDE_IS_LANGSERV_CLIENT (self));

  if (priv->rpc_client != NULL)
    {
      jsonrpc_client_close_async (priv->rpc_client, NULL, NULL, NULL);
      g_clear_object (&priv->rpc_client);
    }

  IDE_EXIT;
}

static void
ide_langserv_client_call_cb (GObject      *object,
                             GAsyncResult *result,
                             gpointer      user_data)
{
  JsonrpcClient *client = (JsonrpcClient *)object;
  g_autoptr(JsonNode) return_value = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GTask) task = user_data;

  IDE_ENTRY;

  g_assert (JSONRPC_IS_CLIENT (client));
  g_assert (G_IS_ASYNC_RESULT (result));

  if (!jsonrpc_client_call_finish (client, result, &return_value, &error))
    {
      g_task_return_error (task, error);
      return;
    }

  g_task_return_pointer (task, g_steal_pointer (&return_value), (GDestroyNotify)json_node_unref);

  IDE_EXIT;
}

void
ide_langserv_client_call_async (IdeLangservClient   *self,
                                const gchar         *method,
                                JsonNode            *params,
                                GCancellable        *cancellable,
                                GAsyncReadyCallback  callback,
                                gpointer             user_data)
{
  IdeLangservClientPrivate *priv = ide_langserv_client_get_instance_private (self);
  g_autoptr(GTask) task = NULL;

  IDE_ENTRY;

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, ide_langserv_client_call_async);

  if (priv->rpc_client == NULL)
    {
      g_task_return_new_error (task,
                               G_IO_ERROR,
                               G_IO_ERROR_NOT_CONNECTED,
                               "No connection to language server");
      IDE_EXIT;
    }

  jsonrpc_client_call_async (priv->rpc_client,
                             method,
                             params,
                             cancellable,
                             ide_langserv_client_call_cb,
                             g_steal_pointer (&task));

  IDE_EXIT;
}

gboolean
ide_langserv_client_call_finish (IdeLangservClient  *self,
                                 GAsyncResult       *result,
                                 JsonNode          **return_value,
                                 GError            **error)
{
  g_autoptr(JsonNode) local_return_value = NULL;
  gboolean ret;

  IDE_ENTRY;

  g_return_val_if_fail (IDE_IS_LANGSERV_CLIENT (self), FALSE);
  g_return_val_if_fail (G_IS_TASK (result), FALSE);

  local_return_value = g_task_propagate_pointer (G_TASK (result), error);
  ret = local_return_value != NULL;

  if (return_value != NULL)
    *return_value = g_steal_pointer (&local_return_value);

  IDE_RETURN (ret);
}
