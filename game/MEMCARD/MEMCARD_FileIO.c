#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003db98-0x8003dbf8.
int MEMCARD_NewTask(int slotIdx, char *name, u8 *ptrMemcard, int memcardFileSize, int unused)
{
	(void)unused;
	sdata->memcardSlot = slotIdx;

	MEMCARD_StringSet(sdata->s_memcardFileCurr, slotIdx, name);

	sdata->memcard_ptrStart = ptrMemcard;
	sdata->memcard_remainingAttempts = 8;
	sdata->memcardFileSize = memcardFileSize;

	return 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003dbf8-0x8003dc30.
void MEMCARD_CloseFile(void)
{
	int fd = sdata->memcard_fd;

	if (fd != -1)
	{
		close(fd);
		sdata->memcard_fd = -1;
	}

	sdata->memcard_stage = MC_STAGE_IDLE;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003dc30-0x8003dc9c.
int MEMCARD_ReadFile(int start_offset, int size)
{
	if ((lseek(sdata->memcard_fd, start_offset, 0) >= 0) && (read(sdata->memcard_fd, sdata->memcard_ptrStart, size) >= 0))
	{
		// The read has started, the result will be found
		// the next time we wait for an event result
		return MC_RETURN_PENDING;
	}

	MEMCARD_CloseFile();
	return MC_RETURN_TIMEOUT;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003dc9c-0x8003dd10.
u8 MEMCARD_WriteFile(int start_offset, const u8 *data, int size)
{
	if ((lseek(sdata->memcard_fd, start_offset, 0) >= 0) && (write(sdata->memcard_fd, data, size) >= 0))
	{
		// The write has started, the result will be found
		// the next time we wait for an event result
		return MC_RETURN_PENDING;
	}

	MEMCARD_CloseFile();
	return MC_RETURN_TIMEOUT;
}
