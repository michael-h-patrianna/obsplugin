/*!
 * @file output-config.cpp
 * @brief Implementation of the base output configuration class.
 */

#include "output-config.h"
#include <QJsonDocument>
#include <QRegularExpression>
#include <obs-frontend-api.h>

OutputConfig::OutputConfig() = default;
OutputConfig::~OutputConfig() = default;

void OutputConfig::loadFromObs(obs_data_t *data)
{
    enabled = obs_data_get_bool(data, "enabled");
    customTitle = QString::fromUtf8(obs_data_get_string(data, "custom_title"));
    sceneId = QString::fromUtf8(obs_data_get_string(data, "scene_id"));
    videoEncoderId = QString::fromUtf8(obs_data_get_string(data, "video_encoder_id"));
    videoBitrateKbps = obs_data_get_int(data, "video_bitrate_kbps");
    keyframeIntervalSec = obs_data_get_int(data, "keyframe_interval_s");
    encoderProfile = QString::fromUtf8(obs_data_get_string(data, "encoder_profile"));
    encoderPreset = QString::fromUtf8(obs_data_get_string(data, "encoder_preset"));
    resolution = QString::fromUtf8(obs_data_get_string(data, "resolution"));
    fps = obs_data_get_int(data, "fps");
    audioEncoderId = QString::fromUtf8(obs_data_get_string(data, "audio_encoder_id"));
    audioBitrateKbps = obs_data_get_int(data, "audio_bitrate_kbps");
    audioChannels = QString::fromUtf8(obs_data_get_string(data, "audio_channels"));
    audioSampleRate = obs_data_get_int(data, "audio_sample_rate");
    reconnectDelayMs = obs_data_get_int(data, "reconnect_delay_ms");
    maxRetries = obs_data_get_int(data, "max_retries");
    customParamsJson = QString::fromUtf8(obs_data_get_string(data, "custom_params_json"));
}

void OutputConfig::saveToObs(obs_data_t *data) const
{
    obs_data_set_bool(data, "enabled", enabled);
    obs_data_set_string(data, "custom_title", customTitle.toUtf8().constData());
    obs_data_set_string(data, "scene_id", sceneId.toUtf8().constData());
    obs_data_set_string(data, "video_encoder_id", videoEncoderId.toUtf8().constData());
    obs_data_set_int(data, "video_bitrate_kbps", videoBitrateKbps);
    obs_data_set_int(data, "keyframe_interval_s", keyframeIntervalSec);
    obs_data_set_string(data, "encoder_profile", encoderProfile.toUtf8().constData());
    obs_data_set_string(data, "encoder_preset", encoderPreset.toUtf8().constData());
    obs_data_set_string(data, "resolution", resolution.toUtf8().constData());
    obs_data_set_int(data, "fps", fps);
    obs_data_set_string(data, "audio_encoder_id", audioEncoderId.toUtf8().constData());
    obs_data_set_int(data, "audio_bitrate_kbps", audioBitrateKbps);
    obs_data_set_string(data, "audio_channels", audioChannels.toUtf8().constData());
    obs_data_set_int(data, "audio_sample_rate", audioSampleRate);
    obs_data_set_int(data, "reconnect_delay_ms", reconnectDelayMs);
    obs_data_set_int(data, "max_retries", maxRetries);
    obs_data_set_string(data, "custom_params_json", customParamsJson.toUtf8().constData());
}

bool OutputConfig::validate() const
{
    // Validate numeric ranges as per spec
    if (videoBitrateKbps < 50 || videoBitrateKbps > 60000) return false;
    if (keyframeIntervalSec < 1 || keyframeIntervalSec > 10) return false;
    if (audioBitrateKbps < 32 || audioBitrateKbps > 320) return false;
    if (reconnectDelayMs < 0 || reconnectDelayMs > 30000) return false;
    if (maxRetries < 0 || maxRetries > 100) return false;

    // Validate resolution format
    if (!validateResolution(resolution)) return false;

    // Validate FPS is in allowed values
    static const QSet<int> validFps = {24, 25, 30, 48, 50, 60};
    if (!validFps.contains(fps)) return false;

    // Validate audio channels
    static const QSet<QString> validChannels = {"mono", "stereo", "5.1"};
    if (!validChannels.contains(audioChannels)) return false;

    // Validate audio sample rate
    static const QSet<int> validSampleRates = {44100, 48000};
    if (!validSampleRates.contains(audioSampleRate)) return false;

    // Validate custom JSON if present
    if (!customParamsJson.isEmpty() && !validateJson(customParamsJson)) return false;

    return true;
}

QStringList OutputConfig::availableVideoEncoders()
{
    QStringList encoders;
    obs_enum_encoders([](void *param, obs_encoder_t *encoder) {
        if (obs_encoder_get_type(encoder) == OBS_ENCODER_VIDEO) {
            auto *list = static_cast<QStringList*>(param);
            list->append(QString::fromUtf8(obs_encoder_get_id(encoder)));
        }
        return true;
    }, &encoders);
    return encoders;
}

QStringList OutputConfig::availableAudioEncoders()
{
    QStringList encoders;
    obs_enum_encoders([](void *param, obs_encoder_t *encoder) {
        if (obs_encoder_get_type(encoder) == OBS_ENCODER_AUDIO) {
            auto *list = static_cast<QStringList*>(param);
            list->append(QString::fromUtf8(obs_encoder_get_id(encoder)));
        }
        return true;
    }, &encoders);
    return encoders;
}

QStringList OutputConfig::availableScenes()
{
    QStringList scenes;
    obs_enum_sources([](void *param, obs_source_t *source) {
        if (obs_source_get_type(source) == OBS_SOURCE_TYPE_SCENE) {
            auto *list = static_cast<QStringList*>(param);
            list->append(QString::fromUtf8(obs_source_get_name(source)));
        }
        return true;
    }, &scenes);
    return scenes;
}

QStringList OutputConfig::supportedProfiles(const QString &encoderId)
{
    // Return hardcoded list of common profiles for now
    // TODO: Get this from encoder capabilities when OBS API supports it
    if (encoderId.contains("x264", Qt::CaseInsensitive)) {
        return {"baseline", "main", "high"};
    } else if (encoderId.contains("nvenc", Qt::CaseInsensitive)) {
        return {"baseline", "main", "high"};
    } else if (encoderId.contains("qsv", Qt::CaseInsensitive)) {
        return {"baseline", "main", "high"};
    } else if (encoderId.contains("amd", Qt::CaseInsensitive)) {
        return {"baseline", "main", "high"};
    }
    return QStringList();
}

QStringList OutputConfig::supportedPresets(const QString &encoderId)
{
    // Return hardcoded list of common presets for now
    // TODO: Get this from encoder capabilities when OBS API supports it
    if (encoderId.contains("x264", Qt::CaseInsensitive)) {
        return {"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow"};
    } else if (encoderId.contains("nvenc", Qt::CaseInsensitive)) {
        return {"default", "hq", "hp", "ll", "llhq", "llhp"};
    } else if (encoderId.contains("qsv", Qt::CaseInsensitive)) {
        return {"speed", "balanced", "quality"};
    } else if (encoderId.contains("amd", Qt::CaseInsensitive)) {
        return {"speed", "balanced", "quality"};
    }
    return QStringList();
}

bool OutputConfig::validateResolution(const QString &res)
{
    static const QRegularExpression re(R"(^\d{3,4}x\d{3,4}$)");
    return re.match(res).hasMatch();
}

bool OutputConfig::validateJson(const QString &json)
{
    QJsonParseError error;
    QJsonDocument::fromJson(json.toUtf8(), &error);
    return error.error == QJsonParseError::NoError;
} 
