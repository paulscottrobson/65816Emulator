/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _GETSET_H_
#define _GETSET_H_

#include "cpuexec.h"
#include "dsp.h"
#include "sa1.h"
#include "spc7110.h"
#include "c4.h"
#include "obc1.h"
#include "seta.h"
#include "bsx.h"
#include "msu1.h"

unsigned char ReadByte(unsigned int address);
unsigned short ReadWord(unsigned int address);
void WriteByte(unsigned int address,unsigned char data);
void WriteWord(unsigned int address,unsigned short data);

#define addCyclesInMemoryAccess {}

#define addCyclesInMemoryAccess_x2 {}

extern uint8	OpenBus;

static inline int32 memory_speed (uint32 address)
{
	return (ONE_CYCLE);
}

inline uint8 S9xGetByte (uint32 Address)
{
	return ReadByte(Address);
}

inline uint16 S9xGetWord (uint32 Address, enum s9xwrap_t w = WRAP_NONE)
{
	return ReadWord(Address);
}

inline void S9xSetByte (uint8 Byte, uint32 Address)
{
	WriteByte(Address,Byte);
}

inline void S9xSetWord (uint16 Word, uint32 Address, enum s9xwrap_t w = WRAP_NONE, enum s9xwriteorder_t o = WRITE_01)
{
	WriteWord(Address,Word);
}

inline void S9xSetPCBase (uint32 Address)
{
}

uint8 dummy;
inline uint8 * S9xGetBasePointer (uint32 Address)
{
	return &dummy;
}

#endif
