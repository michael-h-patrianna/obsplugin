/*!
 * @file obs-config-helper.h
 * @brief Declaration of the OBSConfigHelper class for managing OBS configuration.
 *
 * @author <Developer> <Email Address>
 * @copyright Copyright (C) <Year> <Developer>
 * @license GNU General Public License v2 or later
 * @see https://www.gnu.org/licenses/
 */

#pragma once

#include <obs.h>
#include <QString>
#include <QVariant>

/**
 * @class OBSConfigHelper
 * @brief Helper class for managing OBS configuration data.
 *
 * Provides methods to load and save configuration data from/to OBS settings.
 */
class OBSConfigHelper {
public:
    explicit OBSConfigHelper(const char *configFile);
    ~OBSConfigHelper();

    /// Load configuration from file
    bool load();

    /// Save configuration to file
    bool save();

    /// Load a section from OBS settings
    void loadSection(const QString &section, obs_data_t *data);

    /// Save a section to OBS settings
    void saveSection(const QString &section, obs_data_t *data);

    /// Get a value from a section
    QVariant getValue(const QString &section, const QString &key, const QVariant &defaultValue = QVariant()) const;

    /// Set a value in a section with validation
    bool setValue(const QString &section, const QString &key, const QVariant &value, 
                  QMetaType::Type type = QMetaType::UnknownType,
                  const QVariant &min = QVariant(), const QVariant &max = QVariant());

    /// Set a value in a section (simple version)
    void setValue(const QString &section, const QString &key, const QVariant &value);

private:
    bool validate(const QVariant &value, QMetaType::Type type, 
                  const QVariant &min, const QVariant &max) const;

    QString configFilePath;
    obs_data_t *configData{nullptr};
};

