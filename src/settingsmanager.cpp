#include "core_interface.h"
#include "osal_dynamiclib.h"
#include "settingsmanager.h"
#include "version.h"
#include "debug.h"

#include <fstream>
#include <string>

using namespace std;

SettingsManager::SettingsManager() : settings_initialized(false)
{

}

SettingsManager::~SettingsManager()
{
    unloadSettings();
}

QString SettingsManager::getCurrentPlugin(QString type)
{
    if(type == "video")
    {
        const char *video_plugin = ConfigGetParamString(config_section, "VideoPlugin");

        if((string)video_plugin == "mupen64plus-video-n64.so")
        {
            return "gles2n64";
        }
        else if((string)video_plugin == "mupen64plus-video-GLideN64.so")
        {
            return "GLideN64";
        }
        else if((string)video_plugin == "mupen64plus-video-glide64mk2.so")
        {
            return "glide64mk2";
        }
        else if((string)video_plugin == "mupen64plus-video-rice.so")
        {
            return "rice";
        }
        else
        {
            return QString::fromStdString(video_plugin);
        }
    }
    else if(type == "audio")
    {
        return "sdl-audio";
    }
    else if(type == "input")
    {
        const char *input_plugin = ConfigGetParamString(config_section, "InputPlugin");

        if((string)input_plugin == "mupen64plus-input-sdl.so")
        {
            return "sdl";
        }
        else if((string)input_plugin == "mupen64plus-input-sdltouch.so")
        {
            return "sdl-touch";
        }
        else
        {
            return QString::fromStdString(input_plugin);
        }
    }
    else if(type == "rsp")
    {
        return "hle";
    }
    else if(type == "core")
    {
        return "mupen64plus";
    }
    else
    {
        return "unimplemented";
    }
}

qint32 SettingsManager::getCurrentPluginIndex(QString type)
{
    if(type == "video")
    {
        const char *video_plugin = ConfigGetParamString(config_section, "VideoPlugin");

        if((string)video_plugin == "mupen64plus-video-n64.so")
        {
            return 0;
        }
        else if((string)video_plugin == "mupen64plus-video-GLideN64.so")
        {
            return 1;
        }
        else if((string)video_plugin == "mupen64plus-video-glide64mk2.so")
        {
            return 2;
        }
        else if((string)video_plugin == "mupen64plus-video-rice.so")
        {
            return 3;
        }
        else
        {
            return 4;
        }
    }
    else if(type == "audio")
    {
        return 0;
    }
    else if(type == "input")
    {
        const char *input_plugin = ConfigGetParamString(config_section, "InputPlugin");

        if((string)input_plugin == "mupen64plus-input-sdltouch.so")
        {
            return 0;
        }
        else if((string)input_plugin == "mupen64plus-input-sdl.so")
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }
    else if(type == "rsp")
    {
        return 0;
    }
    else
    {
        return 0;
    }
}

void list_parameters_callback(void *context, const char *setting_name, m64p_type setting_type)
{
    SettingsManager *sm = (SettingsManager*)context;
    // skip Version version and other version settings
    if(strstr(setting_name, "ersion") == NULL)
    {
        sm->addToCurrentSettings(setting_name, setting_type);
    }
}

void SettingsManager::addToCurrentSettings(const char *setting_name, m64p_type setting_type)
{
    my_current_settings.append(setting_name);
}

void SettingsManager::loadSettings()
{
    qDebug("attaching core");
    m64p_error err;

    err = AttachCoreLib(NULL);

    qDebug(("attached core lib, return code: " + to_string(err)).c_str());

    err = CoreStartup(CORE_API_VERSION, NULL, NULL, (void*)"Core", DebugCallback, NULL, NULL);

    qDebug(("started core lib, return code: " + to_string(err)).c_str());

    err = ConfigOpenSection("UI-Console", &config_section);

    qDebug(("config section opened: " + to_string(err)).c_str());
}

void SettingsManager::unloadSettings()
{
    if(ConfigSaveSection != NULL)
    {
        qDebug("saving and detaching core");
        ConfigSaveSection("UI-Console");
        ConfigSaveFile();
        CoreShutdown();
        DetachCoreLib();
    }
    else
    {
        qDebug("core already detached. might happen if the app is closed");
    }
}


QStringList SettingsManager::getPossibleSettings(QString plugin_type, QString plugin_name)
{
    m64p_handle section;
    QStringList list;
    string section_name = "";
    m64p_error err;
    qDebug("loading possible settings");

    if(settings_initialized == false)
    {
        // load the plugins once to initialize settings.
        m64p_handle video_plugin, audio_plugin, input_plugin, rsp_plugin;

        ptr_PluginStartup PluginStartup;
        ptr_PluginShutdown PluginShutdown;

        std::string plugin_path = ConfigGetParamString(config_section, "PluginPath");
        err = osal_dynlib_open(&video_plugin, (plugin_path + "/" + ConfigGetParamString(config_section, "VideoPlugin")).c_str());
        qDebug(((std::string)"Loaded video plugin, return code: " + to_string(err)).c_str());
        PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(video_plugin, "PluginStartup");
        PluginStartup(CoreHandle, (void*)"Video", DebugCallback);
        err = osal_dynlib_open(&audio_plugin, (plugin_path + "/" + ConfigGetParamString(config_section, "AudioPlugin")).c_str());
        qDebug(((std::string)"Loaded audio plugin, return code: " + to_string(err)).c_str());
        PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(audio_plugin, "PluginStartup");
        PluginStartup(CoreHandle, (void*)"Audio", DebugCallback);
        err = osal_dynlib_open(&input_plugin, (plugin_path + "/" + ConfigGetParamString(config_section, "InputPlugin")).c_str());
        qDebug(((std::string)"Loaded input plugin, return code: " + to_string(err)).c_str());
        PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(input_plugin, "PluginStartup");
        PluginStartup(CoreHandle, (void*)"Input", DebugCallback);
        err = osal_dynlib_open(&rsp_plugin, (plugin_path + "/" + ConfigGetParamString(config_section, "RspPlugin")).c_str());
        qDebug(((std::string)"Loaded rsp plugin, return code: " + to_string(err)).c_str());
        PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(rsp_plugin, "PluginStartup");
        PluginStartup(CoreHandle, (void*)"Rsp", DebugCallback);

        PluginShutdown = (ptr_PluginShutdown)osal_dynlib_getproc(video_plugin, "PluginShutdown");
        if(PluginShutdown) PluginShutdown();
        else qDebug("Video: no shutdown function");
        osal_dynlib_close(video_plugin);

        PluginShutdown = (ptr_PluginShutdown)osal_dynlib_getproc(audio_plugin, "PluginShutdown");
        if(PluginShutdown) PluginShutdown();
        else qDebug("Audio: no shutdown function");
        osal_dynlib_close(audio_plugin);

        PluginShutdown = (ptr_PluginShutdown)osal_dynlib_getproc(input_plugin, "PluginShutdown");
        if(PluginShutdown) PluginShutdown();
        else qDebug("Input: no shutdown function");
        osal_dynlib_close(input_plugin);

        PluginShutdown = (ptr_PluginShutdown)osal_dynlib_getproc(rsp_plugin, "PluginShutdown");
        if(PluginShutdown) PluginShutdown();
        else qDebug("Rsp: no shutdown function");
        osal_dynlib_close(rsp_plugin);

        video_plugin = audio_plugin = input_plugin = rsp_plugin = NULL;

        settings_initialized = true;
    }

    if(plugin_type == "video")
    {
        if(plugin_name == "gles2n64")
        {
            section_name = "Video-gles2n64";
        }
        else if(plugin_name == "GLideN64")
        {
            section_name = "Video-GLideN64";
        }
        else if(plugin_name == "glide64mk2")
        {
            section_name = "Video-Glide64mk2";
        }
        else if(plugin_name == "rice")
        {
            section_name = "Video-Rice";
        }
    }
    else if(plugin_type == "audio")
    {
        section_name = "Audio-SDL";
    }
    else if(plugin_type == "input")
    {
        if(plugin_name == "sdl")
        {
            section_name = "Input-SDL-Control1";
        }
        else if(plugin_name == "sdl-touch")
        {
            section_name = "Input-Touch";
        }
    }
    else if(plugin_type == "rsp")
    {
        section_name = "rsp";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    my_current_settings.clear();

    err = ConfigOpenSection(section_name.c_str(), &section);
    ConfigListParameters(section, this, list_parameters_callback);

    list = my_current_settings;

    my_current_settings.clear();

    return list;
}

QStringList SettingsManager::getPossibleGeneralSettings(QString plugin_type)
{
    m64p_handle section;
    m64p_error err;
    QStringList list;

    my_current_settings.clear();

    if(plugin_type == "video")
    {
        err = ConfigOpenSection("Video-General", &section);
        ConfigListParameters(section, this, list_parameters_callback);
    }

    list = my_current_settings;

    my_current_settings.clear();

    return list;
}

QString SettingsManager::getSettingValue(QString plugin_type, QString plugin_name, QString setting_name)
{
    std::string value;
    m64p_handle section;
    m64p_type type;
    string section_name = "";

    if(plugin_type == "video")
    {
        if(plugin_name == "gles2n64")
        {
            section_name = "Video-gles2n64";
        }
        else if(plugin_name == "GLideN64")
        {
            section_name = "Video-GLideN64";
        }
        else if(plugin_name == "glide64mk2")
        {
            section_name = "Video-Glide64mk2";
        }
        else if(plugin_name == "rice")
        {
            section_name = "Video-Rice";
        }
    }
    else if(plugin_type == "audio")
    {
        section_name = "Audio-SDL";
    }
    else if(plugin_type == "input")
    {
        if(plugin_name == "sdl")
        {
            section_name = "Input-SDL-Control1";
        }
        else if(plugin_name == "sdl-touch")
        {
            section_name = "Input-Touch";
        }
    }
    else if(plugin_type == "rsp")
    {
        section_name = "rsp";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    ConfigOpenSection(section_name.c_str(), &section);
    ConfigGetParameterType(section, setting_name.toStdString().c_str(), &type);

    if(M64TYPE_BOOL == type)
    {
        value = to_string(ConfigGetParamBool(section, setting_name.toStdString().c_str()));
    }
    else if(M64TYPE_FLOAT == type)
    {
        value = to_string(ConfigGetParamFloat(section, setting_name.toStdString().c_str()));
    }
    else if(M64TYPE_INT == type)
    {
        value = to_string(ConfigGetParamInt(section, setting_name.toStdString().c_str()));
    }
    else if(M64TYPE_STRING == type)
    {
        value = to_string(ConfigGetParamString(section, setting_name.toStdString().c_str()));
    }

    return QString::fromStdString(value);
}

QString SettingsManager::getGeneralSettingValue(QString plugin_type, QString plugin_name, QString setting_name)
{
    std::string value;
    m64p_handle section;
    m64p_type type;
    string section_name = "";

    if(plugin_type == "video")
    {
        section_name = "Video-General";
    }
    else if(plugin_type == "audio")
    {
        section_name = "";
    }
    else if(plugin_type == "input")
    {
        section_name = "";
    }
    else if(plugin_type == "rsp")
    {
        section_name = "";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    if(section_name == "") return "";

    ConfigOpenSection(section_name.c_str(), &section);
    ConfigGetParameterType(section, setting_name.toStdString().c_str(), &type);

    if(M64TYPE_BOOL == type)
    {
        value = to_string(ConfigGetParamBool(section, setting_name.toStdString().c_str()));
    }
    else if(M64TYPE_FLOAT == type)
    {
        value = to_string(ConfigGetParamFloat(section, setting_name.toStdString().c_str()));
    }
    else if(M64TYPE_INT == type)
    {
        value = to_string(ConfigGetParamInt(section, setting_name.toStdString().c_str()));
    }
    else if(M64TYPE_STRING == type)
    {
        value = to_string(ConfigGetParamString(section, setting_name.toStdString().c_str()));
    }

    return QString::fromStdString(value);
}

QString SettingsManager::getSettingHint(QString plugin_type, QString plugin_name, QString setting_name)
{
    m64p_handle section;
    string section_name = "";

    if(plugin_type == "video")
    {
        if(plugin_name == "gles2n64")
        {
            section_name = "Video-gles2n64";
        }
        else if(plugin_name == "GLideN64")
        {
            section_name = "Video-GLideN64";
        }
        else if(plugin_name == "glide64mk2")
        {
            section_name = "Video-Glide64mk2";
        }
        else if(plugin_name == "rice")
        {
            section_name = "Video-Rice";
        }
    }
    else if(plugin_type == "audio")
    {
        section_name = "Audio-SDL";
    }
    else if(plugin_type == "input")
    {
        if(plugin_name == "sdl")
        {
            section_name = "Input-SDL-Control1";
        }
        else if(plugin_name == "sdl-touch")
        {
            section_name = "Input-Touch";
        }
    }
    else if(plugin_type == "rsp")
    {
        section_name = "rsp";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    ConfigOpenSection(section_name.c_str(), &section);
    return ConfigGetParameterHelp(section, setting_name.toStdString().c_str());
}

QString SettingsManager::getGeneralSettingHint(QString plugin_type, QString plugin_name, QString setting_name)
{
    m64p_handle section;
    string section_name = "";

    if(plugin_type == "video")
    {
        section_name = "Video-General";
    }
    else if(plugin_type == "audio")
    {
        section_name = "";
    }
    else if(plugin_type == "input")
    {
        section_name = "";
    }
    else if(plugin_type == "rsp")
    {
        section_name = "";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    ConfigOpenSection(section_name.c_str(), &section);
    return ConfigGetParameterHelp(section, setting_name.toStdString().c_str());
}

qint32 SettingsManager::saveGeneralSetting(QString plugin_type, QString setting_name, QString value)
{
    m64p_handle section;
    m64p_type type;
    string section_name = "";

    if(plugin_type == "video")
    {
        section_name = "Video-General";
    }
    else if(plugin_type == "audio")
    {
        section_name = "";
    }
    else if(plugin_type == "input")
    {
        section_name = "";
    }
    else if(plugin_type == "rsp")
    {
        section_name = "";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    qDebug(("section name: " + section_name).c_str());

    ConfigOpenSection(section_name.c_str(), &section);
    ConfigGetParameterType(section, setting_name.toStdString().c_str(), &type);
    if(M64TYPE_BOOL == type)
    {
        qDebug("saving boolean value");
        int bvalue = (value == "true" || value == "1");
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, &bvalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else if(M64TYPE_FLOAT == type)
    {
        qDebug("saving float value");
        float fvalue = value.toFloat();
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, &fvalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else if(M64TYPE_INT == type)
    {
        qDebug("saving int value");
        int ivalue = value.toInt();
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, &ivalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else if(M64TYPE_STRING == type)
    {
        qDebug("saving string value");
        char *svalue = strdup(value.toStdString().c_str());
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, svalue);
        free(svalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else
    {
        qDebug("unknown setting type");
    }

    return -1;
}

qint32 SettingsManager::saveSetting(QString plugin_type, QString plugin_name, QString setting_name, QString value)
{
    m64p_handle section;
    m64p_type type;
    string section_name = "";


    if(plugin_type == "video")
    {
        if(plugin_name == "gles2n64")
        {
            section_name = "Video-gles2n64";
        }
        else if(plugin_name == "GLideN64")
        {
            section_name = "Video-GLideN64";
        }
        else if(plugin_name == "glide64mk2")
        {
            section_name = "Video-Glide64mk2";
        }
        else if(plugin_name == "rice")
        {
            section_name = "Video-Rice";
        }
    }
    else if(plugin_type == "audio")
    {
        section_name = "Audio-SDL";
    }
    else if(plugin_type == "input")
    {
        if(plugin_name == "sdl")
        {
            section_name = "Input-SDL-Control1";
        }
        else if(plugin_name == "sdl-touch")
        {
            section_name = "Input-Touch";
        }
    }
    else if(plugin_type == "rsp")
    {
        section_name = "rsp";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    qDebug(("section name: " + section_name).c_str());

    ConfigOpenSection(section_name.c_str(), &section);
    ConfigGetParameterType(section, setting_name.toStdString().c_str(), &type);
    if(M64TYPE_BOOL == type)
    {
        qDebug("saving boolean value");
        int bvalue = (value.toLower() == "true" || value == "1");
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, &bvalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else if(M64TYPE_FLOAT == type)
    {
        qDebug("saving float value");
        float fvalue = value.toFloat();
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, &fvalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else if(M64TYPE_INT == type)
    {
        qDebug("saving int value");
        int ivalue = value.toInt();
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, &ivalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else if(M64TYPE_STRING == type)
    {
        qDebug("saving string value");
        char *svalue = strdup(value.toStdString().c_str());
        ConfigSetParameter(section, setting_name.toStdString().c_str(), type, svalue);
        free(svalue);
        ConfigSaveSection(section_name.c_str());
        return 0;
    }
    else
    {
        qDebug("unknown setting type");
    }

    return -1;
}

qint32 SettingsManager::getSettingType(QString plugin_type, QString plugin_name, QString setting)
{
    m64p_handle section;
    m64p_type type;
    string section_name = "";

    if(plugin_type == "video")
    {
        if(plugin_name == "gles2n64")
        {
            section_name = "Video-gles2n64";
        }
        else if(plugin_name == "GLideN64")
        {
            section_name = "Video-GLideN64";
        }
        else if(plugin_name == "glide64mk2")
        {
            section_name = "Video-Glide64mk2";
        }
        else if(plugin_name == "rice")
        {
            section_name = "Video-Rice";
        }
    }
    else if(plugin_type == "audio")
    {
        section_name = "Audio-SDL";
    }
    else if(plugin_type == "input")
    {
        if(plugin_name == "sdl")
        {
            section_name = "Input-SDL-Control1";
        }
        else if(plugin_name == "sdl-touch")
        {
            section_name = "Input-Touch";
        }
    }
    else if(plugin_type == "rsp")
    {
        section_name = "rsp";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    ConfigOpenSection(section_name.c_str(), &section);
    ConfigGetParameterType(section, setting.toStdString().c_str(), &type);

    return type;
}

qint32 SettingsManager::getGeneralSettingType(QString plugin_type, QString setting)
{
    m64p_handle section;
    m64p_type type;
    string section_name = "";

    if(plugin_type == "video")
    {
        section_name = "Video-General";
    }
    else if(plugin_type == "audio")
    {
        section_name = "";
    }
    else if(plugin_type == "input")
    {
        section_name = "";
    }
    else if(plugin_type == "rsp")
    {
        section_name = "";
    }
    else if(plugin_type == "core")
    {
        section_name = "Core";
    }

    if(section_name == "") return 0;

    ConfigOpenSection(section_name.c_str(), &section);
    ConfigGetParameterType(section, setting.toStdString().c_str(), &type);

    return type;
}

qint32 SettingsManager::pluginCount(QString plugin_type)
{
    if(plugin_type == "video")
    {
        return 4;
    }
    else if(plugin_type == "audio")
    {
        return 1;
    }
    else if(plugin_type == "input")
    {
        return 2;
    }
    else if(plugin_type == "rsp")
    {
        return 1;
    }
    else if(plugin_type == "core")
    {
        return 1;
    }

    return 0;
}

QString SettingsManager::getPlugin(QString plugin_type, qint32 plugin_num)
{
    if(plugin_type == "video")
    {
        switch(plugin_num)
        {
        case 0:
            return "gles2n64";
        case 1:
            return "GLideN64";
        case 2:
            return "glide64mk2";
        case 3:
            return "rice";
        }
    }
    else if(plugin_type == "audio")
    {
        return "sdl-audio";
    }
    else if(plugin_type == "input")
    {
        switch(plugin_num)
        {
        case 0:
            return "sdl-touch";
        case 1:
            return "sdl";
        }
    }
    else if(plugin_type == "rsp")
    {
        return "hle";
    }
    else if(plugin_type == "Core")
    {
        return "mupen64plus";
    }

    return "unimplemented";
}

void SettingsManager::selectPlugin(QString plugin_type, qint32 plugin_num)
{
    m64p_error err;
    settings_initialized = false;

    if(plugin_type == "video")
    {
        switch(plugin_num)
        {
        case 0:
            err = ConfigSetParameter(config_section, "VideoPlugin", M64TYPE_STRING, "mupen64plus-video-n64.so");
            break;
        case 1:
            err = ConfigSetParameter(config_section, "VideoPlugin", M64TYPE_STRING, "mupen64plus-video-GLideN64.so");
            break;
        case 2:
            err = ConfigSetParameter(config_section, "VideoPlugin", M64TYPE_STRING, "mupen64plus-video-glide64mk2.so");
            break;
        case 3:
            err = ConfigSetParameter(config_section, "VideoPlugin", M64TYPE_STRING, "mupen64plus-video-rice.so");
            break;
        }
    }
    else if(plugin_type == "audio")
    {
        err = ConfigSetParameter(config_section, "AudioPlugin", M64TYPE_STRING, "mupen64plus-audio-sdl.so");
    }
    else if(plugin_type == "input")
    {
        switch(plugin_num)
        {
        case 0:
            err = ConfigSetParameter(config_section, "InputPlugin", M64TYPE_STRING, "mupen64plus-input-sdltouch.so");
            break;
        case 1:
            err = ConfigSetParameter(config_section, "InputPlugin", M64TYPE_STRING, "mupen64plus-input-sdl.so");
            break;
        }
    }
    else if(plugin_type == "rsp")
    {
        err = ConfigSetParameter(config_section, "RspPlugin", M64TYPE_STRING, "mupen64plus-rsp-hle.so");
    }
    else if(plugin_type == "core")
    {
        return;
    }

    qDebug(("changed plugin, return code: " + to_string(err)).c_str());
}
