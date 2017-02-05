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

emulator_thread::emulator_thread() : stopped(true), audio_resource_acquired(false), m_audio_resource(this, AudioResourceQt::AudioResource::MediaType)
{

}

void emulator_thread::stop()
{
    stopped = true;
    this->wait();
}

void emulator_thread::set_game(std::string game)
{
    QObject::connect(&m_audio_resource, SIGNAL(acquiredChanged()),
                         this, SLOT(onAcquiredChanged()));
    m_audio_resource.acquire();
    my_game = game;
}

m64p_error emulator_thread::VidExtFuncInit(void)
{
    return M64ERR_SUCCESS;
}

m64p_error emulator_thread::VidExtFuncQuit(void)
{
    return M64ERR_SUCCESS;
}

m64p_error emulator_thread::VidExtFuncListModes(m64p_2d_size *, int *)
{
}

m64p_error emulator_thread::VidExtFuncSetMode(int a, int b, int c, int d, int e)
{
    the_emulator_thread.fmt.setMajorVersion(2);
    the_emulator_thread.fmt.setMinorVersion(0);
    the_emulator_thread.fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    the_emulator_thread.fmt.setRenderableType(QSurfaceFormat::OpenGLES);

    the_emulator_thread.my_window = new QWindow();
    the_emulator_thread.my_window->setFormat(the_emulator_thread.fmt);
    the_emulator_thread.my_window->setSurfaceType(QSurface::OpenGLSurface);
    the_emulator_thread.my_window->create();
    the_emulator_thread.my_window->resize(a, b);
    the_emulator_thread.my_window->show();

    the_emulator_thread.my_render_context = new QOpenGLContext(the_emulator_thread.my_window);
    the_emulator_thread.my_render_context->setFormat(the_emulator_thread.fmt);
    the_emulator_thread.my_render_context->create();
    the_emulator_thread.my_render_context->makeCurrent(the_emulator_thread.my_window);

    qDebug(("Setting mode " + to_string(a) + " " + to_string(b) + " " + to_string(c) + " " + to_string(d) + " " + to_string(e)).c_str());
    the_emulator_thread.my_window->resize(a, b);

    return M64ERR_SUCCESS;
}

void *     emulator_thread::VidExtFuncGLGetProc(const char* procName)
{
    return (void*)the_emulator_thread.my_render_context->getProcAddress(QString(procName).toLatin1());
}

m64p_error emulator_thread::VidExtFuncGLSetAttr(m64p_GLattr attr, int value)
{
    switch(attr)
    {
    case M64P_GL_DEPTH_SIZE:
        the_emulator_thread.fmt.setDepthBufferSize(value);
        break;
    case M64P_GL_RED_SIZE:
        the_emulator_thread.fmt.setRedBufferSize(value);
        break;
    case M64P_GL_GREEN_SIZE:
        the_emulator_thread.fmt.setGreenBufferSize(value);
        break;
    case M64P_GL_BLUE_SIZE:
        the_emulator_thread.fmt.setBlueBufferSize(value);
        break;
    case M64P_GL_ALPHA_SIZE:
        the_emulator_thread.fmt.setAlphaBufferSize(value);
        break;
    case M64P_GL_DOUBLEBUFFER:
        if(value)
        {
            the_emulator_thread.fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
        }
        else
        {
            the_emulator_thread.fmt.setSwapBehavior(QSurfaceFormat::SingleBuffer);
        }
        break;
    default:
        qDebug(("set attr unimplemented for: " + to_string(attr) + " value: " + to_string(value)).c_str());
        return M64ERR_INCOMPATIBLE;
    }
    return M64ERR_SUCCESS;
}

m64p_error emulator_thread::VidExtFuncGLGetAttr(m64p_GLattr attr, int *value)
{
    switch(attr)
    {
    case M64P_GL_DEPTH_SIZE:
        *value = the_emulator_thread.fmt.depthBufferSize();
        break;
    case M64P_GL_RED_SIZE:
        *value = the_emulator_thread.fmt.redBufferSize();
        break;
    case M64P_GL_GREEN_SIZE:
        *value = the_emulator_thread.fmt.greenBufferSize();
        break;
    case M64P_GL_BLUE_SIZE:
        *value = the_emulator_thread.fmt.blueBufferSize();
        break;
    case M64P_GL_ALPHA_SIZE:
        *value = the_emulator_thread.fmt.alphaBufferSize();
        break;
    default:
        qDebug(("get attr unimplemented for: " + to_string(attr)).c_str());
        return M64ERR_INCOMPATIBLE;
    }
    return M64ERR_SUCCESS;
}

m64p_error emulator_thread::VidExtFuncGLSwapBuf(void)
{
    if(the_emulator_thread.stopped)
    {
        return CoreDoCommand(M64CMD_STOP, 0, NULL);
    }
    the_emulator_thread.my_render_context->swapBuffers(the_emulator_thread.my_render_context->surface());
    return M64ERR_SUCCESS;
}

m64p_error emulator_thread::VidExtFuncSetCaption(const char *)
{
}

m64p_error emulator_thread::VidExtFuncToggleFS(void)
{
}

m64p_error emulator_thread::VidExtFuncResizeWindow(int a, int b)
{
    the_emulator_thread.my_window->resize(a, b);
}

void emulator_thread::run()
{
    stopped = false;

    char *rom_buffer;
    unsigned int length;
    m64p_error err;
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

    m64p_video_extension_functions vidext = {
        11,
        emulator_thread::VidExtFuncInit,
        emulator_thread::VidExtFuncQuit,
        emulator_thread::VidExtFuncListModes,
        emulator_thread::VidExtFuncSetMode,
        emulator_thread::VidExtFuncGLGetProc,
        emulator_thread::VidExtFuncGLSetAttr,
        emulator_thread::VidExtFuncGLGetAttr,
        emulator_thread::VidExtFuncGLSwapBuf,
        emulator_thread::VidExtFuncSetCaption,
        emulator_thread::VidExtFuncToggleFS,
        emulator_thread::VidExtFuncResizeWindow,
    };

    CoreOverrideVidExt(&vidext);

    err = CoreDoCommand(M64CMD_ROM_OPEN, (int)length, rom_buffer);

    m64p_handle video_plugin, audio_plugin, input_plugin, rsp_plugin;

    ptr_PluginStartup PluginStartup;
    ptr_PluginShutdown PluginShutdown;

    err = ConfigOpenSection("mupen64plus-launcher-sailfish", &config_section);

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

    delete my_render_context;
    delete my_window;
    qDebug("game stopped");
    stopped = true;
}

void emulator_thread::onAcquiredChanged()
{
    qDebug("audio resource acquired");
    audio_resource_acquired = m_audio_resource.isAcquired();
}
