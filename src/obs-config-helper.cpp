/*!
 * @file obs-config-helper.cpp
 * @brief Implementation of the OBSConfigHelper class.
 */

#include "obs-config-helper.h"

#include "plugin-support.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <util/platform.h>
#include <QFileInfo>
#include <obs-module.h>

static inline QString typeNameCompat(QMetaType::Type t)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QLatin1String(QMetaType(static_cast<int>(t)).name());
#else
    return QLatin1String(QMetaType::typeName(t));
#endif
}

// Convenience wrapper for the new QVariant::canConvert(const QMetaType &)
static inline bool canConvertCompat(const QVariant &v, QMetaType::Type t)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return v.canConvert(QMetaType(static_cast<int>(t)));
#else
    return v.canConvert(t);
#endif
}

OBSConfigHelper::OBSConfigHelper(const char *configFile)
{
    /* 31.x way: resolve per-module config path (must free with bfree) */
    char *raw = obs_module_config_path(configFile);
    configFilePath = QString::fromUtf8(raw ? raw : "");
    bfree(raw);

    // create the folder if needed
    const QByteArray dirUtf8 = QFileInfo(configFilePath).path().toUtf8();
    os_mkdirs(dirUtf8.constData());                      // util/platform.h

    configData = obs_data_create();   // start empty until load() is called
    obs_log(LOG_INFO, "[OBSConfigHelper] Using config file: %s", configFilePath.toUtf8().constData());
}

OBSConfigHelper::~OBSConfigHelper()
{
    obs_data_release(configData);
}

/* ------------------------------------------------------------------------- */
/*  SAFER LOAD/SAVE                                                          */
/* ------------------------------------------------------------------------- */
bool OBSConfigHelper::load()
{
    obs_data_release(configData);  /* drop any previous data */
    configData = obs_data_create_from_json_file_safe(
        configFilePath.toUtf8().constData(), ".bak");

    if (!configData)               /* corrupted or first run */
        configData = obs_data_create();

    return configData != nullptr;
}

bool OBSConfigHelper::save()
{
    if (!configData)
        return false;

    return obs_data_save_json_safe(
        configData,
        configFilePath.toUtf8().constData(),
        ".tmp",   /* temp extension */
        ".bak");  /* backup extension */
}

/* ------------------------------------------------------------------------- */
/*  SET / GET WITH VALIDATION                                                */
/* ------------------------------------------------------------------------- */
bool OBSConfigHelper::setValue(const QString &section, const QString &key,
                               const QVariant &value, QMetaType::Type type,
                               const QVariant &min, const QVariant &max)
{
    if (!configData)
        return false;

    if (type != QMetaType::UnknownType && !validate(value, type, min, max))
        return false;

    /* grab (or lazily create) the section object --------------------------- */
    obs_data_t *sectionObj   = obs_data_get_obj(configData, section.toUtf8().constData());
    bool        newSection   = (sectionObj == nullptr);

    if (newSection)          // create a blank one if the section didn't exist
        sectionObj = obs_data_create();

    /* --------------------------------------------------------------------- */
    /*  write the key *before* attaching the section to the parent           */
    /* --------------------------------------------------------------------- */
    QMetaType::Type actualType = type == QMetaType::UnknownType ? 
        static_cast<QMetaType::Type>(value.typeId()) : type;

    switch (actualType) {
    case QMetaType::Int:      obs_data_set_int   (sectionObj, key.toUtf8().constData(), value.toInt());        break;
    case QMetaType::LongLong: obs_data_set_int   (sectionObj, key.toUtf8().constData(), value.toLongLong());   break;
    case QMetaType::Double:   obs_data_set_double(sectionObj, key.toUtf8().constData(), value.toDouble());     break;
    case QMetaType::Bool:     obs_data_set_bool  (sectionObj, key.toUtf8().constData(), value.toBool());       break;
    case QMetaType::QString:  obs_data_set_string(sectionObj, key.toUtf8().constData(),
                                                 value.toString().toUtf8().constData());                      break;
    case QMetaType::QByteArray:
        obs_data_set_string(sectionObj, key.toUtf8().constData(),
                            value.toByteArray().constData());
        break;
    default:
        obs_log(LOG_WARNING, "[OBSConfigHelper] Unsupported type for %s -> %s",
                key.toUtf8().constData(), typeNameCompat(actualType).toUtf8().constData());
        obs_data_release(sectionObj);
        return false;
    }

    /* attach (or re-attach) the fully-populated section ------------------- */
    obs_data_set_obj(configData, section.toUtf8().constData(), sectionObj);
    obs_data_release(sectionObj);     // balance our ref
    return true;
}

void OBSConfigHelper::setValue(const QString &section, const QString &key, const QVariant &value)
{
    setValue(section, key, value, QMetaType::UnknownType);
}

QVariant OBSConfigHelper::getValue(const QString &section, const QString &key,
                                   const QVariant &def) const
{
    if (!configData)
        return def;

    obs_data_t *sectionObj =
        obs_data_get_obj(configData, section.toUtf8().constData());

    if (!sectionObj)
        return def;

    QVariant out;
    switch (def.typeId()) {
    case QMetaType::Int:      out = obs_data_get_int   (sectionObj, key.toUtf8().constData()); break;
    case QMetaType::LongLong: out = (qlonglong)obs_data_get_int(sectionObj, key.toUtf8().constData()); break;
    case QMetaType::Double:   out = obs_data_get_double(sectionObj, key.toUtf8().constData()); break;
    case QMetaType::Bool:     out = obs_data_get_bool  (sectionObj, key.toUtf8().constData()); break;
    case QMetaType::QString:  out = QString::fromUtf8(obs_data_get_string(sectionObj, key.toUtf8().constData())); break;
    case QMetaType::QByteArray:
        out = QByteArray(obs_data_get_string(sectionObj, key.toUtf8().constData()));
        break;
    default:                  out = def; break;
    }

    obs_data_release(sectionObj);
    return out.isValid() ? out : def;
}

void OBSConfigHelper::loadSection(const QString &section, obs_data_t *data)
{
    if (!configData || !data)
        return;

    obs_data_t *sectionObj = obs_data_get_obj(configData, section.toUtf8().constData());
    if (sectionObj) {
        obs_data_apply(data, sectionObj);
        obs_data_release(sectionObj);
    }
}

void OBSConfigHelper::saveSection(const QString &section, obs_data_t *data)
{
    if (!configData || !data)
        return;

    obs_data_set_obj(configData, section.toUtf8().constData(), data);
}

// -------------------------------------------------------------------------
//  validate()  –  Qt 6-clean
// -------------------------------------------------------------------------
bool OBSConfigHelper::validate(const QVariant &value,
                               QMetaType::Type   type,
                               const QVariant   &min,
                               const QVariant   &max) const
{
    // 1) type check –––––––––––––––––––––––––––––––––––––––––––––––––––––
    if (value.metaType().id() != static_cast<int>(type)) {
        obs_log(LOG_WARNING, "[OBSConfigHelper] Validation failed: type mismatch.  Expected %s, got %s",
                typeNameCompat(type).toUtf8().constData(), value.metaType().name());
        return false;
    }

    // 2) numeric types ––––––––––––––––––––––––––––––––––––––––––––––––––
    auto isIntLike = [](QMetaType::Type t) {
        return t == QMetaType::Int   || t == QMetaType::Long
            || t == QMetaType::LongLong || t == QMetaType::Short
            || t == QMetaType::UInt  || t == QMetaType::ULong
            || t == QMetaType::ULongLong || t == QMetaType::UShort;
    };

    if (isIntLike(type)) {
        qlonglong v = value.toLongLong();
        if (min.isValid()) {
            if (!canConvertCompat(min, type) || v < min.toLongLong())
                return false;
        }
        if (max.isValid()) {
            if (!canConvertCompat(max, type) || v > max.toLongLong())
                return false;
        }
        return true;
    }

    if (type == QMetaType::Double || type == QMetaType::Float) {
        double v = value.toDouble();
        if (min.isValid()) {
            if (!canConvertCompat(min, type) || v < min.toDouble())
                return false;
        }
        if (max.isValid()) {
            if (!canConvertCompat(max, type) || v > max.toDouble())
                return false;
        }
        return true;
    }

    // 3) QString –––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    if (type == QMetaType::QString) {
        const QString v = value.toString();
        if (min.isValid() && (!canConvertCompat(min, type) || v < min.toString()))
            return false;
        if (max.isValid() && (!canConvertCompat(max, type) || v > max.toString()))
            return false;
        return true;
    }

    // 4) Bool (ranges don't make sense) –––––––––––––––––––––––––––––––––
    if (type == QMetaType::Bool)
        return true;         // nothing more to validate

    // 5) fallback – unsupported type/min/max combo ––––––––––––––––––––––
    if (min.isValid() || max.isValid()) {
        obs_log(LOG_WARNING, "[OBSConfigHelper] OBSConfigHelper::validate(): min/max validation not "
                      "implemented for %s", typeNameCompat(type).toUtf8().constData());
        return false;
    }
    return true;
}

