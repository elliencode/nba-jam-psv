#include <kubridge.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "utils/init.h"
#include "utils/glutil.h"

#include <psp2/kernel/threadmgr.h>

#include <falso_jni/FalsoJNI.h>
#include <psp2/appmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <so_util/so_util.h>
#include <sys/_pthreadtypes.h>
#include <utils/utils.h>

#include "reimpl/controls.h"
#include "utils/logger.h"

int _newlib_heap_size_user = 256 * 1024 * 1024;

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif

so_module so_mod;

void (* nativeKeyDown)(JNIEnv * env, jclass clazz, int keycode);
void (* nativeKeyUp)(JNIEnv * env, jclass clazz, int keycode);

void (* nativeTouchBegan)(JNIEnv * env, jclass clazz, int x, int y, int pointerIdx);
void (* nativeTouchMoved)(JNIEnv * env, jclass clazz, int x, int y, int pointerIdx);
void (* nativeTouchEnded)(JNIEnv * env, jclass clazz, int x, int y, int pointerIdx);

void (* nativeMotion)(JNIEnv * env, jclass clazz, int unk, float x, float y);

void * controls_thread() {
    // Move to 4th core if available
    sceKernelChangeThreadCpuAffinityMask(sceKernelGetThreadId(), 0x80000);

    while (1) {
        controls_poll();
        sceKernelDelayThread(8335); // half a frame assuming 60fps
    }
}

void * game_thread();

volatile int doMorrowind = 0;
volatile int doingMorrowind = 0;

int main(int argc, char * argv[]) {
    soloader_init_all();
    l_success("soloader_init_all() passed");

    if (!file_exists("ux0:data/com.eamobile.nbajam_row_wf/Language")) {
        file_save("ux0:data/com.eamobile.nbajam_row_wf/Language", "en", 3);
    }

    for (int i = 0; i < argc; ++i)
        if (strstr(argv[i], "-silent"))
            doingMorrowind = 1;

    int (* JNI_OnLoad)(void *jvm) = (void *)so_symbol(&so_mod, "JNI_OnLoad");
    JNI_OnLoad(&jvm);
    l_success("JNI_OnLoad() passed");

    gl_init();
    l_success("gl_init() passed");

    nativeKeyDown = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeKeyDown");
    nativeKeyUp = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeKeyUp");
    nativeTouchBegan = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeTouchBegan");
    nativeTouchMoved = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeTouchMoved");
    nativeTouchEnded = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeTouchEnded");
    nativeMotion = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeMotion");

    void (* nativeOnCreate)() = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeOnCreate");
    nativeOnCreate();
    l_success("nativeOnCreate() passed");

    void (* nativeOnResume)() = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeOnResume");
    nativeOnResume();
    l_success("nativeOnResume() passed");

    void (* nativeOnSurfaceCreated)() = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeOnSurfaceCreated");
    nativeOnSurfaceCreated();
    l_success("nativeOnSurfaceCreated() passed");

    void (* nativeOnSurfaceChanged)(JNIEnv * env, jclass clazz, int width, int height, int safeInsetLeft, int safeInsetRight, int safeInsetTop, int safeInsetBottom) = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeOnSurfaceChanged");
    nativeOnSurfaceChanged(&jni, (jclass)0x42424242, 960, 544, 0, 0, 0, 0);
    l_success("nativeOnSurfaceChanged() passed");

    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 256 * 1024);
    pthread_create(&t, &attr, controls_thread, NULL);
    pthread_detach(t);
    l_success("Controls thread detached");

    // Running the .so in a thread with enlarged stack size.
    pthread_t t3;
    pthread_attr_t attr3;
    pthread_attr_init(&attr3);
    pthread_attr_setstacksize(&attr3, 4 * 1024 * 1024);
    pthread_create(&t3, &attr3, game_thread, NULL);
    pthread_detach(t3);
    l_success("Game thread detached");

    l_success("Main thread 0x%x exiting", sceKernelGetThreadId());

    sceKernelExitDeleteThread(0);
}

KuKernelExceptionHandler nextHandler;

void UndefInstrHandler(KuKernelExceptionContext *exceptionContext) {
    sceClibPrintf("<ExceptionHandler> Caught Prefetch Abort!\n");
    if (exceptionContext->pc == 0) {
        sceClibPrintf("PC is 0x0, jumping to LR (0x%x)\n", exceptionContext->lr);
        if (exceptionContext->lr & 0x1) {
            sceClibPrintf("LR is &0x1, setting SPSR to |= 0x20, current value: 0x%x\n", exceptionContext->SPSR);
            exceptionContext->SPSR |= 0x20;
        }
        exceptionContext->pc = exceptionContext->lr;
    }
}


void DabtHandler(KuKernelExceptionContext *exceptionContext) {
    sceClibPrintf("<ExceptionHandler> Caught Data Abort!\n");
    sceClibPrintf("<ExceptionHandler> PC: 0x%x\n", exceptionContext->pc);
    sceClibPrintf("<ExceptionHandler> LR: 0x%x\n", exceptionContext->lr);
    if (exceptionContext->pc != 0) {
        exceptionContext->pc += 4;
    }
}

void * game_thread() {
    kuKernelRegisterExceptionHandler(KU_KERNEL_EXCEPTION_TYPE_DATA_ABORT, DabtHandler, &nextHandler, NULL);
    kuKernelRegisterExceptionHandler(KU_KERNEL_EXCEPTION_TYPE_PREFETCH_ABORT, UndefInstrHandler, &nextHandler, NULL);

    uint32_t startedAt = sceKernelGetProcessTimeLow();

    void (* nativeOnDrawFrame)() = (void *)so_symbol(&so_mod, "Java_com_eamobile_nbajam_NBAJamActivity_nativeOnDrawFrame");
    while (1) {
        if (doingMorrowind && sceKernelGetProcessTimeLow() - startedAt < 20000000) {
            drawFakeLoadingScreen();
            gl_swap();
        }

        nativeOnDrawFrame();

        if (doingMorrowind && sceKernelGetProcessTimeLow() - startedAt < 20000000) {
            continue;
        }

        if (doMorrowind) {
            for (int i = 0; i < 3; ++i) {
                drawFakeLoadingScreen();
                gl_swap();
            }
            char argp[512];
            sprintf(argp, "-silent");
            sceAppMgrLoadExec("app0:/eboot.bin", (char * const*)((const char*[]){argp, 0}), NULL);
            while(1) sceKernelDelayThread(10000);
        }

        gl_swap();
    }
}

void controls_handler_key(int32_t keycode, ControlsAction action) {
    switch (action) {
        case CONTROLS_ACTION_DOWN:
            nativeKeyDown(&jni, (jclass)0x42424242, keycode);
            break;
        case CONTROLS_ACTION_UP:
            nativeKeyUp(&jni, (jclass)0x42424242, keycode);
            break;
    }
}

void controls_handler_touch(int32_t id, float x, float y, ControlsAction action) {
    switch (action) {
        case CONTROLS_ACTION_DOWN:
            nativeTouchBegan(&jni, (jclass)0x42424242, (int)x, (int)y, id);
            break;
        case CONTROLS_ACTION_UP:
            nativeTouchEnded(&jni, (jclass)0x42424242, (int)x, (int)y, id);
            break;
        case CONTROLS_ACTION_MOVE:
            nativeTouchMoved(&jni, (jclass)0x42424242, (int)x, (int)y, id);
            break;
    }
}

void controls_handler_analog(ControlsStickId which, float x, float y, ControlsAction action) {
    if (which == CONTROLS_STICK_LEFT)
        nativeMotion(&jni, (jclass)0x42424242, 0, x, y);
}
