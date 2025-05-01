#pragma once
#include <cstdint>

struct LoadData
{
    char const *exp;
    void *param;
    char const *path;

    void *loadlib;
    void *getprocaddr;
};

/* Pseudo-C of the following code:

DWORD WINAPI LoadDllAndSetParameters(LoadData *data)
{
    auto lib = (decltype(&LoadLibraryA)(data->loadlib))(data->path);
    if(!lib) return 0;
    auto proc = decltype(&GetProcAddress)(data->getprocaddr)(lib, data->exp);
    if(!proc) return 0;
    return ((DWORD(__stdcall*)(void*))proc)(data->param);
}
*/

static uint8_t const load_dll_and_set_parameters[] = {
/*0:*/  0x53,                      /*push   rbx ; nonvolatile backup */
/*1:*/  0x48, 0x83, 0xec, 0x20,    /*sub    rsp, 0x20 */

/*5:*/  0x48, 0x89, 0xcb,          /*mov    rbx, rcx */
/*8:*/  0x48, 0x8b, 0x49, 0x10,    /*mov    rcx, QWORD PTR [rcx+0x10] */
/*c:*/  0xff, 0x53, 0x18,          /*call   QWORD PTR [rbx+0x18] ; r = data->loadlib(data->path); */

/*f:*/  0x48, 0x85, 0xc0,          /*test   rax, rax */
/*12:*/ 0x74, 0x19,                /*je     0x2d, <failure_exit> ; if(!r) return r; */

/*14:*/ 0x48, 0x8b, 0x13,          /*mov    rdx, QWORD PTR [rbx] */
/*17:*/ 0x48, 0x89, 0xc1,          /*mov    rcx, rax */
/*1a:*/ 0xff, 0x53, 0x20,          /*call   QWORD PTR [rbx+0x20] ; r = data->getprocaddr(data->exp); */

/*1d:*/ 0x48, 0x85, 0xc0,          /*test   rax, rax */
/*20:*/ 0x74, 0x0b,                /*je     0x2d, <failure_exit> ; if(!r) return r; */

/*22:*/ 0x48, 0x8b, 0x4b, 0x08,    /*mov    rcx, QWORD PTR [rbx+0x8] ; a1 = data->param*/
/*26:*/ 0x48, 0x83, 0xc4, 0x20,    /*add    rsp, 0x20 */
/*2a:*/ 0x5b,                      /*pop    rbx */
/*2b:*/ 0xff, 0xe0,                /*jmp    rax ; tailcall return r(a1); */

/*@0x2d, <failure_exit>:*/
/*2d:*/ 0x48, 0x83, 0xc4, 0x20,    /*add    rsp, 0x20 ; return r */
/*31:*/ 0x5b,                      /*pop    rbx */
/*32:*/ 0xc2, 0x00, 0x00,          /*ret    0x0 */
};
