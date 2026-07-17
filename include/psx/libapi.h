/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/include/psx/libapi.h
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#ifndef LIBAPI_H
#define LIBAPI_H

#include <stdint.h>

#ifndef KERNEL_H
#include "kernel.h"
#endif


extern int SetRCnt(int spec, unsigned short target, int mode);
extern int GetRCnt(int spec);
extern int ResetRCnt(int spec);
extern int StartRCnt(int spec);
extern int StopRCnt(int spec);

extern int OpenEvent(unsigned int desc, int spec, int mode, int32_t (*func)());
extern int CloseEvent(unsigned int event);
extern int WaitEvent(unsigned int event);
extern int TestEvent(unsigned int event);
extern int EnableEvent(unsigned int event);
extern int DisableEvent(unsigned int event);
extern void DeliverEvent(unsigned int ev1, int ev2);
extern void UnDeliverEvent(unsigned int ev1, int ev2);

extern int OpenTh(int (*func)(), unsigned int, unsigned int);
extern int CloseTh(int unk00);
extern int ChangeTh(int unk00);

#if !defined(CTR_NATIVE)
extern int open(char *unk00, unsigned int);
extern int close(int unk00);
extern int lseek(int unk00, int, int);
extern int read(int unk00, void *, int);
extern int write(int unk00, void *, int);
#endif

extern struct DIRENTRY *firstfile(char *unk00, struct DIRENTRY *);
extern struct DIRENTRY *nextfile(struct DIRENTRY *unk00);

extern int erase(char *unk00);
extern int format(char *unk00);

extern int LoadTest(char *unk00, struct EXEC *);
extern int Load(char *unk00, struct EXEC *);
extern int Exec(struct EXEC *unk00, int, char **);
extern int LoadExec(char *unk00, unsigned int, unsigned int);

extern int InitPAD(char *unk00, int, char *, int);
extern int StartPAD();
extern void StopPAD();
extern void EnablePAD();
extern void DisablePAD();

extern void FlushCache();
extern void ReturnFromException();
#if !defined(CTR_NATIVE)
extern int EnterCriticalSection();
extern void ExitCriticalSection();
#endif
extern void Exception();
extern void SwEnterCriticalSection();
extern void SwExitCriticalSection();

extern uint32_t SetSp(uint32_t newsp);
extern uint32_t GetSp();
extern uint32_t GetGp();
extern uint32_t GetCr();
extern uint32_t GetSr();
extern uint32_t GetSysSp();

extern int SetConf(unsigned int, unsigned int, unsigned int);
extern void GetConf(unsigned int *, unsigned int *, unsigned int *);

/*
extern int _get_errno(void);
extern int _get_error(int);
*/
extern void SystemError(char, int);
extern void SetMem(int);

extern int Krom2RawAdd(unsigned int);
extern int Krom2RawAdd2(unsigned short);

extern void _96_init(void);
extern void _96_remove(void);
extern void _boot(void);

extern void ChangeClearPAD(int);

/* prototypes added by shino 96/05/22 */
extern void InitCARD(int val);
extern int StartCARD();
extern int StopCARD();
extern void _bu_init();
extern int _card_info(int chan);
extern int _card_clear(int chan);
extern int _card_load(int chan);
extern int _card_auto(int val);
extern void _new_card();
extern int _card_status(int drv);
extern int _card_wait(int drv);
extern unsigned int _card_chan(void);
extern int _card_write(int chan, int block, unsigned char *buf);
extern int _card_read(int chan, int block, unsigned char *buf);
extern int _card_format(int chan);


#endif
