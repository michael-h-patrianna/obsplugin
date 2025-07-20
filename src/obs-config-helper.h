#pragma once

#include <obs-module.h>
#include <QString>
#include <QVariant>
// #include <QMap> // QMap is not used in the current implementation, can be removed

/**
 * @brief Wrapper for OBS configuration storage.
 *
 * This class provides a secure and stable way to store plugin configurations
 * using OBS's data handling functions, offering type and value validation.
 */
class OBSConfigHelper
{
public:
    /**
     * @brief Constructs an OBSConfigHelper.
     * @param configFile The base name of the configuration file (e.g., "my_plugin_settings.json").
     * OBS typically stores these in its config directory.
     */
    OBSConfigHelper(const char *configFile);
    ~OBSConfigHelper();

    /**
     * @brief Loads configuration data from the specified file.
     * @return true if loading was successful, false otherwise.
     */
    bool load();

    /**
     * @brief Saves the current configuration data to the specified file.
     * @return true if saving was successful, false otherwise.
     */
    bool save();

    /**
     * @brief Sets a configuration value with optional type and range validation.
     * @param section The section name in the configuration (e.g., "General").
     * @param key The key within the section (e.g., "volume").
     * @param value The QVariant containing the value to set.
     * @param type The expected QMetaType::Type of the value.
     * @param min An optional QVariant representing the minimum allowed value for validation.
     * @param max An optional QVariant representing the maximum allowed value for validation.
     * @return true if the value was set successfully and passed validation, false otherwise.
     */
    bool setValue(const QString &section, const QString &key, const QVariant &value, QMetaType::Type type, const QVariant &min = QVariant(), const QVariant &max = QVariant());

    /**
     * @brief Retrieves a configuration value.
     * @param section The section name.
     * @param key The key within the section.
     * @param defaultValue The default value to return if the key is not found or cannot be converted.
     * The type of defaultValue is used to determine the expected type to retrieve.
     * @return The retrieved QVariant value, or defaultValue if not found or conversion fails.
     */
    QVariant getValue(const QString &section, const QString &key, const QVariant &defaultValue = QVariant()) const;

private:
    obs_data_t *configData;
    QString configFilePath;

    /**
     * @brief Validates a QVariant value against an expected type and optional min/max range.
     * @param value The QVariant value to validate.
     * @param type The expected QMetaType::Type.
     * @param min An optional QVariant representing the minimum allowed value.
     * @param max An optional QVariant representing the maximum allowed value.
     * @return true if the value is valid according to the type and range constraints, false otherwise.
     */
    bool validate(const QVariant &value, QMetaType::Type type, const QVariant &min, const QVariant &max) const;
};
