#include "emulator_thread.h"
#include "core_interface.h"
#include "debug.h"
#include "version.h"
#include "osal_dynamiclib.h"

#include <QWindow>
#include <QGuiApplication>
#include <QScreen>

#include <cstdlib>
#include <fstream>

emulator_thread the_emulator_thread;

using namespace std;

extern "C" void DebugMessage(int level, const char *message, ...);

emulator_thread::emulator_thread() : stopped(true)
#if ATTACH_TO_CORE
    , audio_resource_acquired(false), m_audio_resource(this, AudioResourceQt::AudioResource::MediaType)
#endif
{

}

void emulator_thread::stop()
{
    stopped = true;
    if(game)
    {
        game->kill();
    }
    this->wait();
}

void emulator_thread::set_game(std::string tgame)
{
    my_game = tgame;
}

void emulator_thread::acquire_audio()
{
#if ATTACH_TO_CORE
    QObject::connect(&m_audio_resource, SIGNAL(acquiredChanged()),
                         this, SLOT(onAcquiredChanged()));
    m_audio_resource.acquire();
#endif
}

void emulator_thread::run()
{
#if !ATTACH_TO_CORE
    game = new QProcess;
    stopped = false;

    game->start(QString::fromStdString((std::string)"/usr/bin/mupen64plus " + "\"" + my_game + "\""));
    while(game->waitForFinished(-1))
    {
        sleep(1);
    }

    stopped = true;
    delete game;
    game = 0;
#else
    stopped = false;

    m64p_error err;
    int length;
    char *rom_buffer;
    m64p_handle config_section;

    AttachCoreLib(NULL);
    qDebug(("attached core lib, return code: " + to_string(err)).c_str());

    err = CoreStartup(CORE_API_VERSION, NULL, NULL, (void*)"Core", DebugCallback, NULL, NULL);

    qDebug("Launching Game");

    ifstream rom(my_game.c_str(), ifstream::binary);

    rom.seekg (0, ios::end);
    length = rom.tellg();
    rom.seekg (0, ios::beg);
    // allocate memory:
    rom_buffer = (char*)malloc(sizeof(char)*length);
    // read data as a block:
    rom.read(rom_buffer, length);
    rom.close();

    err = CoreDoCommand(M64CMD_ROM_OPEN, (int)length, rom_buffer);

    m64p_handle video_plugin, audio_plugin, input_plugin, rsp_plugin;

    ptr_PluginStartup PluginStartup;
    ptr_PluginShutdown PluginShutdown;

    err = ConfigOpenSection("UI-Console", &config_section);

    qDebug(("config section opened: " + to_string(err)).c_str());

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

    CoreAttachPlugin(M64PLUGIN_GFX, video_plugin);
    CoreAttachPlugin(M64PLUGIN_AUDIO, audio_plugin);
    CoreAttachPlugin(M64PLUGIN_INPUT, input_plugin);
    CoreAttachPlugin(M64PLUGIN_RSP, rsp_plugin);

    qDebug("acquiring audioresource");

    while(!audio_resource_acquired)
    {
        sleep(1);
    }

    CoreDoCommand(M64CMD_EXECUTE, 0, NULL);

    CoreDetachPlugin(M64PLUGIN_GFX);
    CoreDetachPlugin(M64PLUGIN_AUDIO);
    CoreDetachPlugin(M64PLUGIN_INPUT);
    CoreDetachPlugin(M64PLUGIN_RSP);

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

    qDebug("detaching core");

    CoreShutdown();
    DetachCoreLib();
#endif
}

#if ATTACH_TO_CORE
void emulator_thread::onAcquiredChanged()
{
    qDebug("audio resource acquired");
    audio_resource_acquired = m_audio_resource.isAcquired();
}
#endif
