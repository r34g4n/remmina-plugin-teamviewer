/*
 *     Project: Remmina Plugin TEAMVIEWER
 * Description: Remmina protocol plugin to launch a TeamViewer connection.
 *      Author: Fabio Castelli (Muflone) <muflone@vbsimple.net>
 *   Copyright: 2012-2014 Fabio Castelli (Muflone)
 *     License: GPL-2+
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of ERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plugin_config.h"
#include <remmina/remmina_plugin.h>
#if GTK_VERSION == 3
  # include <gtk/gtkx.h>
#endif

static RemminaPluginService *remmina_plugin_service = NULL;

static void remmina_plugin_init(RemminaProtocolWidget *gp)
{
  remmina_plugin_service->log_printf("[%s] remmina_plugin_init\n", PLUGIN_NAME);
}

static gboolean remmina_plugin_open_connection(RemminaProtocolWidget *gp)
{
  remmina_plugin_service->log_printf("[%s] remmina_plugin_open_connection\n", PLUGIN_NAME);
  #define GET_PLUGIN_STRING(value) \
    g_strdup(remmina_plugin_service->file_get_string(remminafile, value))
  #define GET_PLUGIN_BOOLEAN(value) \
    remmina_plugin_service->file_get_int(remminafile, value, FALSE)
  #define GET_PLUGIN_INT(value, default_value) \
    remmina_plugin_service->file_get_int(remminafile, value, default_value)
  #define GET_PLUGIN_PASSWORD(value) \
    g_strdup(remmina_plugin_service->file_get_secret(remminafile, value));

  RemminaFile *remminafile;
  gboolean ret;
  GPid pid;
  GError *error = NULL;
  gchar *argv[50];
  gint argc;
  gint i;
  
  gchar *option_str;

  remminafile = remmina_plugin_service->protocol_plugin_get_file(gp);

  argc = 0;
  argv[argc++] = strdup("teamviewer");

  // Some tvw_main skips the first argument so we're adding a fake argument in the first place
  if (GET_PLUGIN_BOOLEAN("adddashes"))
    argv[argc++] = g_strdup("--");
  argv[argc++] = g_strdup("-i");
  argv[argc++] = GET_PLUGIN_STRING("server");;

  option_str = GET_PLUGIN_PASSWORD("password");
  if (option_str)
  {
    argv[argc++] = g_strdup("--Password");
    argv[argc++] = g_strdup(option_str);
  }

  argv[argc++] = NULL;

  remmina_plugin_service->log_printf("[TEAMVIEWER] starting teamviewer\n");
  ret = g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
    NULL, NULL, &pid, &error);
  remmina_plugin_service->log_printf("[TEAMVIEWER] started with pid %d\n", 
    &pid);

  for (i = 0; i < argc; i++)
    g_free(argv[i]);

  if (!ret)
    remmina_plugin_service->protocol_plugin_set_error(gp, "%s", error->message);

  return FALSE;
}

static gboolean remmina_plugin_close_connection(RemminaProtocolWidget *gp)
{
  remmina_plugin_service->log_printf("[%s] remmina_plugin_close_connection\n", PLUGIN_NAME);
  remmina_plugin_service->protocol_plugin_emit_signal(gp, "disconnect");
  return FALSE;
}

static const RemminaProtocolSetting remmina_plugin_basic_settings[] =
{
  { REMMINA_PROTOCOL_SETTING_TYPE_SERVER, NULL, NULL, FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_PASSWORD, NULL, NULL, FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_CHECK, "adddashes", N_("Add dashes as first argument"), FALSE, NULL, NULL },
  { REMMINA_PROTOCOL_SETTING_TYPE_END, NULL, NULL, FALSE, NULL, NULL }
};

static RemminaProtocolPlugin remmina_plugin =
{
  REMMINA_PLUGIN_TYPE_PROTOCOL,
  PLUGIN_NAME,
  PLUGIN_DESCRIPTION,
  GETTEXT_PACKAGE,
  PLUGIN_VERSION,
  PLUGIN_APPICON,
  PLUGIN_APPICON,
  remmina_plugin_basic_settings,
  NULL,
  REMMINA_PROTOCOL_SSH_SETTING_NONE,
  NULL,
  remmina_plugin_init,
  remmina_plugin_open_connection,
  remmina_plugin_close_connection,
  NULL,
  NULL
};

G_MODULE_EXPORT gboolean remmina_plugin_entry(RemminaPluginService *service)
{
  remmina_plugin_service = service;

  if (!service->register_plugin((RemminaPlugin *) &remmina_plugin))
  {
    return FALSE;
  }
  return TRUE;
}