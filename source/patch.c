/*
 * Copyright (C) 2023 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

/**
 * @file  patch.c
 * @brief Patching some of the .so internal functions or bridging them to native
 *        for better compatibility.
 */

#include <AL/al.h>
#include <physfs.h>
#include <AL/alc.h>
#include <utils/utils.h>

#include <kubridge.h>
#include <string.h>
#include <psp2/appmgr.h>
#include <psp2/kernel/clib.h>
#include <so_util/so_util.h>

extern so_module so_mod;

uint64_t BGAppDelegate__getFreeDiskSpace() {
    uint64_t free_size = 0, max_size = 0;
    sceAppMgrGetDevInfo("ux0:", &max_size, &free_size);
    return free_size;
}

static uint8_t MenuManagerInstance[0x2b8] __attribute__((aligned(8)));
static bool MenuManagerInstance_initialized = false;

void * _ZN11BGSingletonI11MenuManagerE8InstanceEv() {
    if (!MenuManagerInstance_initialized) {
        memset(MenuManagerInstance, 0, sizeof(MenuManagerInstance));
        void * (* _ZN11MenuManagerC1Ev)(void *this) = (void *)so_symbol(&so_mod, "_ZN11MenuManagerC1Ev");
        _ZN11MenuManagerC1Ev(MenuManagerInstance);
        MenuManagerInstance_initialized = true;
    }
    return (void *) MenuManagerInstance;
}


so_hook _ZN16BGMenuController16setFocusedObjectEP12BGMenuObject_hook;

void * _ZN16BGMenuController16setFocusedObjectEP12BGMenuObject(void * this, void * p1) {
    //sceClibPrintf("_ZN16BGMenuController16setFocusedObjectEP12BGMenuObject called with %p / %p\n", this, p1);
    SO_CONTINUE(void *, _ZN16BGMenuController16setFocusedObjectEP12BGMenuObject_hook, this, p1);
    //sceClibPrintf("_ZN16BGMenuController16setFocusedObjectEP12BGMenuObject returning\n");
}

volatile int menuQuitButtonPressed = 0;

so_hook _ZN14GameState_Quit21MenuManagerLoadedMenuEi_hook;
void _ZN14GameState_Quit21MenuManagerLoadedMenuEi(int p1) {
    //sceClibPrintf("_ZN14GameState_Quit21MenuManagerLoadedMenuEi called.\n");
    void *menuAnimMgr;
    void *this;
    void * iVar1;
    int iVar2;
    void *bgMenuObj;

    void * (* BGSingleton__MenuAnimationManager__Instance)() = (void *)so_symbol(&so_mod, "_ZN11BGSingletonI20MenuAnimationManagerE8InstanceEv");
    void * (* MenuAnimationManager__BeginAnimation)(void * this, unsigned int p1) = (void *)so_symbol(&so_mod, "_ZN20MenuAnimationManager14BeginAnimationE10eMenuAnims");
    void * (* MenuManager__GetMenu)(void * this, int p1) = (void *)so_symbol(&so_mod, "_ZN11MenuManager7GetMenuEi");
    int (* KazaamNetworking__IsConnected)() = (void *)so_symbol(&so_mod, "_ZN16KazaamNetworking11IsConnectedEv");

    menuAnimMgr = (void *)BGSingleton__MenuAnimationManager__Instance();
    MenuAnimationManager__BeginAnimation(menuAnimMgr,0x17);
    this = _ZN11BGSingletonI11MenuManagerE8InstanceEv();
    iVar1 = MenuManager__GetMenu(this,0x26);

    iVar2 = KazaamNetworking__IsConnected();
    if (iVar2 == 0) {
        bgMenuObj = *(void **)(iVar1 + 0x84);
    }
    else {
        bgMenuObj = *(void **)(iVar1 + 0x90);
    }
    _ZN16BGMenuController16setFocusedObjectEP12BGMenuObject(*(void **)(this + 0x260),bgMenuObj);
    //sceClibPrintf("_ZN14GameState_Quit21MenuManagerLoadedMenuEi finished.\n");

    menuQuitButtonPressed = 1;
    return;
}

extern volatile int doMorrowind;

so_hook StateMachineClear_hook;
void StateMachineClear(void * this) {
    if (menuQuitButtonPressed == 0) {
        sceClibPrintf("StateMachineClear: Business as usual.\n");
        SO_CONTINUE(void *, StateMachineClear_hook, this);
        return;
    }

    sceClibPrintf("\n\n***\n");
    sceClibPrintf("They have taken you from the Imperial City's prison, first by carriage and now by boat,\n");
    sceClibPrintf("to the east to Morrowind. Fear not, for I am watchful. You have been chosen.\n");
    sceClibPrintf("***\n\n\n");

    doMorrowind = 1;
}



extern void *__cxa_guard_abort;
extern void *__cxa_guard_acquire;
extern void *__cxa_guard_release;

void so_patch(void) {
    //_ZN14QuitToMainMenuC1Ev_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN14QuitToMainMenuC1Ev"), (uintptr_t)&_ZN14QuitToMainMenuC1Ev);
    //_ZN14QuitToMainMenuC1Ev_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN14QuitToMainMenuC2Ev"), (uintptr_t)&_ZN14QuitToMainMenuC1Ev);

    hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN11BGSingletonI11MenuManagerE8InstanceEv"), (uintptr_t)&_ZN11BGSingletonI11MenuManagerE8InstanceEv);

    hook_addr((uintptr_t)so_symbol(&so_mod, "__cxa_guard_abort"), (uintptr_t)&__cxa_guard_abort);
    hook_addr((uintptr_t)so_symbol(&so_mod, "__cxa_guard_acquire"), (uintptr_t)&__cxa_guard_acquire);
    hook_addr((uintptr_t)so_symbol(&so_mod, "__cxa_guard_release"), (uintptr_t)&__cxa_guard_release);


    _ZN16BGMenuController16setFocusedObjectEP12BGMenuObject_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN16BGMenuController16setFocusedObjectEP12BGMenuObject"), (uintptr_t)&_ZN16BGMenuController16setFocusedObjectEP12BGMenuObject);

    //_ZN14GameState_Quit13DrawInterfaceEv_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN14GameState_Quit13DrawInterfaceEv"), (uintptr_t)&_ZN14GameState_Quit13DrawInterfaceEv);
    _ZN14GameState_Quit21MenuManagerLoadedMenuEi_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN14GameState_Quit21MenuManagerLoadedMenuEi"), (uintptr_t)&_ZN14GameState_Quit21MenuManagerLoadedMenuEi);
    //_ZN14GameState_Quit6UpdateEf_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN14GameState_Quit6UpdateEf"), (uintptr_t)&_ZN14GameState_Quit6UpdateEf);


    hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN13BGAppDelegate16getFreeDiskSpaceEv"), (uintptr_t)&BGAppDelegate__getFreeDiskSpace);
    StateMachineClear_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN12StateMachine5ClearEv"), (uintptr_t)&StateMachineClear);


    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_addToSearchPath"), (uintptr_t)&PHYSFS_addToSearchPath);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_caseFold"), (uintptr_t)&PHYSFS_caseFold);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_close"), (uintptr_t)&PHYSFS_close);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_deinit"), (uintptr_t)&PHYSFS_deinit);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_delete"), (uintptr_t)&PHYSFS_delete);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_deregisterArchiver"), (uintptr_t)&PHYSFS_deregisterArchiver);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_enumerate"), (uintptr_t)&PHYSFS_enumerate);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_enumerateFiles"), (uintptr_t)&PHYSFS_enumerateFiles);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_enumerateFilesCallback"), (uintptr_t)&PHYSFS_enumerateFilesCallback);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_eof"), (uintptr_t)&PHYSFS_eof);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_exists"), (uintptr_t)&PHYSFS_exists);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_fileLength"), (uintptr_t)&PHYSFS_fileLength);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_flush"), (uintptr_t)&PHYSFS_flush);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_freeList"), (uintptr_t)&PHYSFS_freeList);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getAllocator"), (uintptr_t)&PHYSFS_getAllocator);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getBaseDir"), (uintptr_t)&PHYSFS_getBaseDir);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getCdRomDirs"), (uintptr_t)&PHYSFS_getCdRomDirs);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getCdRomDirsCallback"), (uintptr_t)&PHYSFS_getCdRomDirsCallback);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getDirSeparator"), (uintptr_t)&PHYSFS_getDirSeparator);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getErrorByCode"), (uintptr_t)&PHYSFS_getErrorByCode);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getLastError"), (uintptr_t)&PHYSFS_getLastError);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getLastErrorCode"), (uintptr_t)&PHYSFS_getLastErrorCode);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getLastModTime"), (uintptr_t)&PHYSFS_getLastModTime);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getLinkedVersion"), (uintptr_t)&PHYSFS_getLinkedVersion);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getMountPoint"), (uintptr_t)&PHYSFS_getMountPoint);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getPrefDir"), (uintptr_t)&PHYSFS_getPrefDir);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getRealDir"), (uintptr_t)&PHYSFS_getRealDir);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getSearchPath"), (uintptr_t)&PHYSFS_getSearchPath);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getSearchPathCallback"), (uintptr_t)&PHYSFS_getSearchPathCallback);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getUserDir"), (uintptr_t)&PHYSFS_getUserDir);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_getWriteDir"), (uintptr_t)&PHYSFS_getWriteDir);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_init"), (uintptr_t)&PHYSFS_init);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_isDirectory"), (uintptr_t)&PHYSFS_isDirectory);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_isInit"), (uintptr_t)&PHYSFS_isInit);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_isSymbolicLink"), (uintptr_t)&PHYSFS_isSymbolicLink);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_mkdir"), (uintptr_t)&PHYSFS_mkdir);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_mount"), (uintptr_t)&PHYSFS_mount);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_mountHandle"), (uintptr_t)&PHYSFS_mountHandle);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_mountIo"), (uintptr_t)&PHYSFS_mountIo);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_mountMemory"), (uintptr_t)&PHYSFS_mountMemory);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_openAppend"), (uintptr_t)&PHYSFS_openAppend);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_openRead"), (uintptr_t)&PHYSFS_openRead);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_openWrite"), (uintptr_t)&PHYSFS_openWrite);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_permitSymbolicLinks"), (uintptr_t)&PHYSFS_permitSymbolicLinks);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_read"), (uintptr_t)&PHYSFS_read);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readBytes"), (uintptr_t)&PHYSFS_readBytes);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readSBE16"), (uintptr_t)&PHYSFS_readSBE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readSBE32"), (uintptr_t)&PHYSFS_readSBE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readSBE64"), (uintptr_t)&PHYSFS_readSBE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readSLE16"), (uintptr_t)&PHYSFS_readSLE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readSLE32"), (uintptr_t)&PHYSFS_readSLE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readSLE64"), (uintptr_t)&PHYSFS_readSLE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readUBE16"), (uintptr_t)&PHYSFS_readUBE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readUBE32"), (uintptr_t)&PHYSFS_readUBE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readUBE64"), (uintptr_t)&PHYSFS_readUBE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readULE16"), (uintptr_t)&PHYSFS_readULE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readULE32"), (uintptr_t)&PHYSFS_readULE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_readULE64"), (uintptr_t)&PHYSFS_readULE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_registerArchiver"), (uintptr_t)&PHYSFS_registerArchiver);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_removeFromSearchPath"), (uintptr_t)&PHYSFS_removeFromSearchPath);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_seek"), (uintptr_t)&PHYSFS_seek);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_setAllocator"), (uintptr_t)&PHYSFS_setAllocator);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_setBuffer"), (uintptr_t)&PHYSFS_setBuffer);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_setErrorCode"), (uintptr_t)&PHYSFS_setErrorCode);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_setSaneConfig"), (uintptr_t)&PHYSFS_setSaneConfig);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_setWriteDir"), (uintptr_t)&PHYSFS_setWriteDir);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_stat"), (uintptr_t)&PHYSFS_stat);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_supportedArchiveTypes"), (uintptr_t)&PHYSFS_supportedArchiveTypes);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapSBE16"), (uintptr_t)&PHYSFS_swapSBE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapSBE32"), (uintptr_t)&PHYSFS_swapSBE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapSBE64"), (uintptr_t)&PHYSFS_swapSBE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapSLE16"), (uintptr_t)&PHYSFS_swapSLE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapSLE32"), (uintptr_t)&PHYSFS_swapSLE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapSLE64"), (uintptr_t)&PHYSFS_swapSLE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapUBE16"), (uintptr_t)&PHYSFS_swapUBE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapUBE32"), (uintptr_t)&PHYSFS_swapUBE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapUBE64"), (uintptr_t)&PHYSFS_swapUBE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapULE16"), (uintptr_t)&PHYSFS_swapULE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapULE32"), (uintptr_t)&PHYSFS_swapULE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_swapULE64"), (uintptr_t)&PHYSFS_swapULE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_symbolicLinksPermitted"), (uintptr_t)&PHYSFS_symbolicLinksPermitted);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_tell"), (uintptr_t)&PHYSFS_tell);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_ucs4stricmp"), (uintptr_t)&PHYSFS_ucs4stricmp);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_unmount"), (uintptr_t)&PHYSFS_unmount);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf16stricmp"), (uintptr_t)&PHYSFS_utf16stricmp);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8FromLatin1"), (uintptr_t)&PHYSFS_utf8FromLatin1);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8FromUcs2"), (uintptr_t)&PHYSFS_utf8FromUcs2);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8FromUcs4"), (uintptr_t)&PHYSFS_utf8FromUcs4);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8FromUtf16"), (uintptr_t)&PHYSFS_utf8FromUtf16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8stricmp"), (uintptr_t)&PHYSFS_utf8stricmp);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8ToUcs2"), (uintptr_t)&PHYSFS_utf8ToUcs2);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8ToUcs4"), (uintptr_t)&PHYSFS_utf8ToUcs4);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_utf8ToUtf16"), (uintptr_t)&PHYSFS_utf8ToUtf16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_write"), (uintptr_t)&PHYSFS_write);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeBytes"), (uintptr_t)&PHYSFS_writeBytes);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeSBE16"), (uintptr_t)&PHYSFS_writeSBE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeSBE32"), (uintptr_t)&PHYSFS_writeSBE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeSBE64"), (uintptr_t)&PHYSFS_writeSBE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeSLE16"), (uintptr_t)&PHYSFS_writeSLE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeSLE32"), (uintptr_t)&PHYSFS_writeSLE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeSLE64"), (uintptr_t)&PHYSFS_writeSLE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeUBE16"), (uintptr_t)&PHYSFS_writeUBE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeUBE32"), (uintptr_t)&PHYSFS_writeUBE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeUBE64"), (uintptr_t)&PHYSFS_writeUBE64);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeULE16"), (uintptr_t)&PHYSFS_writeULE16);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeULE32"), (uintptr_t)&PHYSFS_writeULE32);
    hook_addr((uintptr_t)so_symbol(&so_mod, "PHYSFS_writeULE64"), (uintptr_t)&PHYSFS_writeULE64);

    // OpenAL bundled with the game is going to try to use OpenSLES and fail.
    // We patch it to our Vita-native OpenAL.
    hook_addr((uintptr_t)so_symbol(&so_mod, "alc_opensles_init"), (uintptr_t)&ret0);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alc_opensles_probe"), (uintptr_t)&ret0);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alBuffer3f"), (uintptr_t)&alBuffer3f);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alBuffer3i"), (uintptr_t)&alBuffer3i);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alBufferData"), (uintptr_t)&alBufferData);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alBufferf"), (uintptr_t)&alBufferf);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alBufferfv"), (uintptr_t)&alBufferfv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alBufferi"), (uintptr_t)&alBufferi);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alBufferiv"), (uintptr_t)&alBufferiv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcCaptureCloseDevice"), (uintptr_t)&alcCaptureCloseDevice);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcCaptureOpenDevice"), (uintptr_t)&alcCaptureOpenDevice);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcCaptureSamples"), (uintptr_t)&alcCaptureSamples);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcCaptureStart"), (uintptr_t)&alcCaptureStart);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcCaptureStop"), (uintptr_t)&alcCaptureStop);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcCloseDevice"), (uintptr_t)&alcCloseDevice);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcCreateContext"), (uintptr_t)&alcCreateContext);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcDestroyContext"), (uintptr_t)&alcDestroyContext);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcGetContextsDevice"), (uintptr_t)&alcGetContextsDevice);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcGetCurrentContext"), (uintptr_t)&alcGetCurrentContext);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcGetEnumValue"), (uintptr_t)&alcGetEnumValue);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcGetError"), (uintptr_t)&alcGetError);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcGetIntegerv"), (uintptr_t)&alcGetIntegerv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcGetProcAddress"), (uintptr_t)&alcGetProcAddress);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcGetString"), (uintptr_t)&alcGetString);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcIsExtensionPresent"), (uintptr_t)&alcIsExtensionPresent);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcMakeContextCurrent"), (uintptr_t)&alcMakeContextCurrent);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcOpenDevice"), (uintptr_t)&alcOpenDevice);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcProcessContext"), (uintptr_t)&alcProcessContext);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alcSuspendContext"), (uintptr_t)&alcSuspendContext);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alDeleteBuffers"), (uintptr_t)&alDeleteBuffers);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alDeleteSources"), (uintptr_t)&alDeleteSources);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alDisable"), (uintptr_t)&alDisable);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alDistanceModel"), (uintptr_t)&alDistanceModel);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alDopplerFactor"), (uintptr_t)&alDopplerFactor);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alDopplerVelocity"), (uintptr_t)&alDopplerVelocity);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alEnable"), (uintptr_t)&alEnable);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGenBuffers"), (uintptr_t)&alGenBuffers);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGenSources"), (uintptr_t)&alGenSources);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBoolean"), (uintptr_t)&alGetBoolean);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBooleanv"), (uintptr_t)&alGetBooleanv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBuffer3f"), (uintptr_t)&alGetBuffer3f);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBuffer3i"), (uintptr_t)&alGetBuffer3i);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBufferf"), (uintptr_t)&alGetBufferf);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBufferfv"), (uintptr_t)&alGetBufferfv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBufferi"), (uintptr_t)&alGetBufferi);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetBufferiv"), (uintptr_t)&alGetBufferiv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetDouble"), (uintptr_t)&alGetDouble);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetDoublev"), (uintptr_t)&alGetDoublev);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetEnumValue"), (uintptr_t)&alGetEnumValue);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetError"), (uintptr_t)&alGetError);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetFloat"), (uintptr_t)&alGetFloat);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetFloatv"), (uintptr_t)&alGetFloatv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetInteger"), (uintptr_t)&alGetInteger);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetIntegerv"), (uintptr_t)&alGetIntegerv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetListener3f"), (uintptr_t)&alGetListener3f);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetListener3i"), (uintptr_t)&alGetListener3i);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetListenerf"), (uintptr_t)&alGetListenerf);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetListenerfv"), (uintptr_t)&alGetListenerfv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetListeneri"), (uintptr_t)&alGetListeneri);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetListeneriv"), (uintptr_t)&alGetListeneriv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetProcAddress"), (uintptr_t)&alGetProcAddress);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetSource3f"), (uintptr_t)&alGetSource3f);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetSource3i"), (uintptr_t)&alGetSource3i);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetSourcef"), (uintptr_t)&alGetSourcef);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetSourcefv"), (uintptr_t)&alGetSourcefv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetSourcei"), (uintptr_t)&alGetSourcei);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetSourceiv"), (uintptr_t)&alGetSourceiv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alGetString"), (uintptr_t)&alGetString);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alIsBuffer"), (uintptr_t)&alIsBuffer);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alIsEnabled"), (uintptr_t)&alIsEnabled);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alIsExtensionPresent"), (uintptr_t)&alIsExtensionPresent);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alIsSource"), (uintptr_t)&alIsSource);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alListener3f"), (uintptr_t)&alListener3f);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alListener3i"), (uintptr_t)&alListener3i);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alListenerf"), (uintptr_t)&alListenerf);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alListenerfv"), (uintptr_t)&alListenerfv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alListeneri"), (uintptr_t)&alListeneri);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alListeneriv"), (uintptr_t)&alListeneriv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSource3f"), (uintptr_t)&alSource3f);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSource3i"), (uintptr_t)&alSource3i);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourcef"), (uintptr_t)&alSourcef);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourcefv"), (uintptr_t)&alSourcefv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourcei"), (uintptr_t)&alSourcei);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourceiv"), (uintptr_t)&alSourceiv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourcePause"), (uintptr_t)&alSourcePause);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourcePausev"), (uintptr_t)&alSourcePausev);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourcePlay"), (uintptr_t)&alSourcePlay);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourcePlayv"), (uintptr_t)&alSourcePlayv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourceQueueBuffers"), (uintptr_t)&alSourceQueueBuffers);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourceRewind"), (uintptr_t)&alSourceRewind);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourceRewindv"), (uintptr_t)&alSourceRewindv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourceStop"), (uintptr_t)&alSourceStop);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourceStopv"), (uintptr_t)&alSourceStopv);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSourceUnqueueBuffers"), (uintptr_t)&alSourceUnqueueBuffers);
    hook_addr((uintptr_t)so_symbol(&so_mod, "alSpeedOfSound"), (uintptr_t)&alSpeedOfSound);
}
