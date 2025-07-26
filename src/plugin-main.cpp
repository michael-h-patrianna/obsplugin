/*
 * PlayFame – OBS Plugin
 */

#include "plugin-main.h"
#include "plugin-dock.h"
#include "plugin-support.h"
#include "obs-config-helper.h"

#include <obs-frontend-api.h>
#include <obs-module.h>

#include <QMetaObject>
#include <QThread>
#include <QWidget>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("playfame", "en-US")

/* ------------------------------------------------------------------------- */
/*  Globals                                                                  */
/* ------------------------------------------------------------------------- */
static PlayFameDock    *g_main_dock     = nullptr;
static OBSConfigHelper *g_plugin_config = nullptr;

/**
 * @brief Destroy the dock safely on the UI thread.
 *
 * If we are already on Qt's GUI thread, delete directly. Otherwise marshal a
 * blocking call to ensure the object goes away *before* this function returns.
 */
static void destroy_dock_safe()
{
    if (!g_main_dock)
        return;

    QObject *obj = g_main_dock;                 /* keep valid pointer        */
    g_main_dock  = nullptr;                     /* mark as gone immediately  */

    if (QThread::currentThread() == obj->thread()) {
        delete obj;
    } else {
        QMetaObject::invokeMethod(
            obj,                                /* context = dock itself     */
            [obj]() { delete obj; },
            Qt::BlockingQueuedConnection);
    }
}

/**
 * @brief OBS-frontend event callback.
 *
 * Removes the dock during the *UI shutdown* phase, while frontend callbacks
 * are still valid.  This avoids using frontend API from obs_module_unload().
 */
static void on_frontend_event(enum obs_frontend_event e, void *)
{
    if (e == OBS_FRONTEND_EVENT_EXIT) {
        destroy_dock_safe();
    }
}

/**
 * @brief Called by OBS when the plugin is loaded.
 *
 * Creates / loads the configuration helper and registers the dock.
 *
 * @return `true` if initialisation succeeded.
 */
bool obs_module_load(void)
{
    obs_log(LOG_INFO, "[playfame] Loading plugin…");

    /* 1 Initialise configuration --------------------------------------- */
    g_plugin_config = new OBSConfigHelper("playfame_config.json");
    g_plugin_config->load();

    /* 2 Create dock in the UI thread ----------------------------------- */
    QWidget *mainWindow =
        static_cast<QWidget *>(obs_frontend_get_main_window());
    if (!mainWindow) {
        obs_log(LOG_WARNING, "[playfame] Could not acquire main window");
        return false;
    }

    QMetaObject::invokeMethod(
        mainWindow,
        [mainWindow]() {
            auto *dock = new PlayFameDock(g_plugin_config, mainWindow);
            if (!dock->registerDock()) {
                obs_log(LOG_ERROR, "[playfame] Failed to register dock");
                dock->deleteLater();
            } else {
                g_main_dock = dock;
            }
        },
        Qt::QueuedConnection);

    /* 3 Listen for EXIT so we can tear the dock down while callbacks stay valid */
    obs_frontend_add_event_callback(on_frontend_event, nullptr);

    obs_log(LOG_INFO, "[playfame] Plugin loaded successfully");
    return true;
}

/**
 * @brief Called by OBS when the plugin is unloaded.
 *
 * Front-end callbacks are *already* gone here, so we must **not** touch
 * `obs_frontend_*` APIs.  Any UI objects should have been destroyed in the
 * EXIT event; we only clean up residual data.
 */
void obs_module_unload(void)
{
    obs_log(LOG_INFO, "[playfame] Unloading plugin…");

    /* Ensure no dangling dock survives (defensive) */
    destroy_dock_safe();

    /* Persist & free configuration ------------------------------------ */
    if (g_plugin_config) {
        g_plugin_config->save();
        delete g_plugin_config;
        g_plugin_config = nullptr;
    }

    obs_log(LOG_INFO, "[playfame] Plugin unloaded");
}

