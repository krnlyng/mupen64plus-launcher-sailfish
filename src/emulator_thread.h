#ifndef EMULATOR_THREAD_H
#define EMULATOR_THREAD_H

#include <QObject>
#include <QThread>
#include <AudioResourceQt>

#include <QWindow>
#include <QOpenGLContext>

#include <string>
#include <vector>

#include "core_interface.h"

class emulator_thread : public QThread
{
    Q_OBJECT
public:
    emulator_thread();
    void stop();
    void set_game(std::string game);
private:
    std::string my_game;
    bool stopped;
    bool audio_resource_acquired;
    AudioResourceQt::AudioResource m_audio_resource;
    QOpenGLContext *my_render_context;
    QWindow *my_window;
    QSurfaceFormat fmt;
    static m64p_error VidExtFuncInit(void);
    static m64p_error VidExtFuncQuit(void);
    static m64p_error VidExtFuncListModes(m64p_2d_size *, int *);
    static m64p_error VidExtFuncSetMode(int, int, int, int, int);
    static void *     VidExtFuncGLGetProc(const char*);
    static m64p_error VidExtFuncGLSetAttr(m64p_GLattr, int);
    static m64p_error VidExtFuncGLGetAttr(m64p_GLattr, int *);
    static m64p_error VidExtFuncGLSwapBuf(void);
    static m64p_error VidExtFuncSetCaption(const char *);
    static m64p_error VidExtFuncToggleFS(void);
    static m64p_error VidExtFuncResizeWindow(int, int);
    void run();
public slots:
    void onAcquiredChanged(void);
};

extern emulator_thread the_emulator_thread;

#endif // EMULATOR_THREAD_H
