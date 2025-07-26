/*!
 * @file plugin-main.h
 * @brief Entry point declarations for the PlayFame OBS plugin.
 *
 * Declares the required load and unload functions for OBS module integration.
 *
 * @author <Developer> <Email Address>
 * @copyright Copyright (C) <Year> <Developer>
 * @license GNU General Public License v2 or later
 * @see https://www.gnu.org/licenses/
 */

#pragma once

#include <obs-module.h>

/// Called by OBS when the module is loaded.
bool obs_module_load(void);

/// Called by OBS when the module is unloaded.
void obs_module_unload(void);

