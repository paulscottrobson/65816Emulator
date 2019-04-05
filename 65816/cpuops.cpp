/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"

// for "Magic WDM" features
#ifdef DEBUGGER
#include "snapshot.h"
#include "display.h"
#include "debug.h"
#include "missing.h"
#endif

#ifdef SA1_OPCODES
#define AddCycles(n)	{ SA1.Cycles += (n); }
#else
#define AddCycles(n)	{ CPU.Cycles += (n); while (CPU.Cycles >= CPU.NextEvent) S9xDoHEventProcessing(); }
#endif

#include "cpuaddr.h"
#include "cpuops.h"
#include "cpumacro.h"
// ******************************************************************************************
// 					This code is inserted after #include "cpumacro.h"
// ******************************************************************************************


struct SICPU ICPU;
struct SRegisters Registers;
struct SCPUState CPU;

uint8 IRQJunk[256];
uint8 OpenBus = 0;

void S9xDoHEventProcessing() {}
void S9xSetBSX(unsigned char x,unsigned int y) {}
void S9xSetSetaDSP(unsigned char x, unsigned int y) {}
void S9xSetST018(unsigned char x, unsigned int y) {}
void S9xSetOBC1(unsigned char x, unsigned short y) {}
void S9xSetDSP(unsigned char x, unsigned short y) {}
void S9xSetC4(unsigned char x, unsigned short y) {}
void S9xSetCPU(unsigned char x, unsigned short y) {}
void S9xSetPPU(unsigned char x, unsigned short y) {}

uint8 S9xGetPPU(uint16 x) { return 0; }
uint8 S9xGetBSX(unsigned int x) { return 0; }
uint8 S9xGetST018(unsigned int x) { return 0; }
uint8 S9xGetSetaDSP(unsigned int x) { return 0; }
uint8 S9xGetOBC1(unsigned short x) { return 0; }
uint8 S9xGetC4(unsigned short x) { return 0; }
uint8 S9xGetCPU(unsigned short x) { return 0; }
uint8 S9xGetDSP(unsigned short x) { return 0; }
uint8 S9xGetSPC7110(unsigned short x) { return 0; }
uint8 S9xGetSPC7110Byte(unsigned int x) { return 0; }

uint8 *S9xGetBasePointerC4(unsigned short x) { return &OpenBus; }
uint8 *S9xGetBasePointerOBC1(unsigned short x) { return &OpenBus; }
uint8 *S9xGetBasePointerBSX(unsigned int x) { return &OpenBus; }
uint8 *S9xGetBasePointerSPC7110(unsigned int x) { return &OpenBus; }

uint8 S9xOpLengthsM0X0[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 0
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 2
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 4
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
1, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 6
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
2, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 8
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
3, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // A
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
3, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // C
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
3, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // E
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};

uint8 S9xOpLengthsM0X1[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 0
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 2
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 4
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
1, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 6
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
2, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 8
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // A
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // C
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // E
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};

uint8 S9xOpLengthsM1X0[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 0
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 2
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 4
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 6
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
2, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 8
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
3, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // A
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // C
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // E
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};

uint8 S9xOpLengthsM1X1[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 0
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 2
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 4
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 6
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
2, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 8
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // A
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // C
2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // E
2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};

//
//	Comes from cpu.cpp
//

void Reset65816(void) {

CPU.Cycles = 0;

Registers.SL = 0xff;
Registers.P.W = 0;
Registers.A.W = 0;
Registers.X.W = 0;
Registers.Y.W = 0;
SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
ClearFlags(Decimal);

Registers.PBPC = 0;
Registers.PB = 0;
Registers.PCw = S9xGetWord(0xfffc);
OpenBus = Registers.PCh;
Registers.D.W = 0;
Registers.DB = 0;
Registers.SH = 1;
Registers.SL -= 3;
Registers.XH = 0;
Registers.YH = 0;

ClearFlags(Emulation);					// Hack into 65816 mode.
}

// ******************************************************************************************


/* ADC ********************************************************************* */

static void Op69M1 (void)
{
	ADC(Immediate8(READ));
}

static void Op69M0 (void)
{
	ADC(Immediate16(READ));
}

static void Op69Slow (void)
{
	if (CheckMemory())
		ADC(Immediate8Slow(READ));
	else
		ADC(Immediate16Slow(READ));
}

rOPM (65Slow,   DirectSlow,                       WRAP_BANK, ADC)

rOPM (75Slow,   DirectIndexedXSlow,               WRAP_BANK, ADC)

rOPM (72Slow,   DirectIndirectSlow,               WRAP_NONE, ADC)

rOPM (61Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, ADC)

rOPM (71Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, ADC)

rOPM (67Slow,   DirectIndirectLongSlow,           WRAP_NONE, ADC)

rOPM (77Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, ADC)

rOPM (6DSlow,   AbsoluteSlow,                     WRAP_NONE, ADC)

rOPM (7DSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, ADC)

rOPM (79Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, ADC)

rOPM (6FSlow,   AbsoluteLongSlow,                 WRAP_NONE, ADC)

rOPM (7FSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, ADC)

rOPM (63Slow,   StackRelativeSlow,                WRAP_NONE, ADC)

rOPM (73Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, ADC)

/* AND ********************************************************************* */

static void Op29M1 (void)
{
	Registers.AL &= Immediate8(READ);
	SetZN(Registers.AL);
}

static void Op29M0 (void)
{
	Registers.A.W &= Immediate16(READ);
	SetZN(Registers.A.W);
}

static void Op29Slow (void)
{
	if (CheckMemory())
	{
		Registers.AL &= Immediate8Slow(READ);
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W &= Immediate16Slow(READ);
		SetZN(Registers.A.W);
	}
}

rOPM (25Slow,   DirectSlow,                       WRAP_BANK, AND)

rOPM (35Slow,   DirectIndexedXSlow,               WRAP_BANK, AND)

rOPM (32Slow,   DirectIndirectSlow,               WRAP_NONE, AND)

rOPM (21Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, AND)

rOPM (31Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, AND)

rOPM (27Slow,   DirectIndirectLongSlow,           WRAP_NONE, AND)

rOPM (37Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, AND)

rOPM (2DSlow,   AbsoluteSlow,                     WRAP_NONE, AND)

rOPM (3DSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, AND)

rOPM (39Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, AND)

rOPM (2FSlow,   AbsoluteLongSlow,                 WRAP_NONE, AND)

rOPM (3FSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, AND)

rOPM (23Slow,   StackRelativeSlow,                WRAP_NONE, AND)

rOPM (33Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, AND)

/* ASL ********************************************************************* */

static void Op0AM1 (void)
{
	AddCycles(ONE_CYCLE);
	ICPU._Carry = (Registers.AL & 0x80) != 0;
	Registers.AL <<= 1;
	SetZN(Registers.AL);
}

static void Op0AM0 (void)
{
	AddCycles(ONE_CYCLE);
	ICPU._Carry = (Registers.AH & 0x80) != 0;
	Registers.A.W <<= 1;
	SetZN(Registers.A.W);
}

static void Op0ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		ICPU._Carry = (Registers.AL & 0x80) != 0;
		Registers.AL <<= 1;
		SetZN(Registers.AL);
	}
	else
	{
		ICPU._Carry = (Registers.AH & 0x80) != 0;
		Registers.A.W <<= 1;
		SetZN(Registers.A.W);
	}
}

mOPM (06Slow,   DirectSlow,                       WRAP_BANK, ASL)

mOPM (16Slow,   DirectIndexedXSlow,               WRAP_BANK, ASL)

mOPM (0ESlow,   AbsoluteSlow,                     WRAP_NONE, ASL)

mOPM (1ESlow,   AbsoluteIndexedXSlow,             WRAP_NONE, ASL)

/* BIT ********************************************************************* */

static void Op89M1 (void)
{
	ICPU._Zero = Registers.AL & Immediate8(READ);
}

static void Op89M0 (void)
{
	ICPU._Zero = (Registers.A.W & Immediate16(READ)) != 0;
}

static void Op89Slow (void)
{
	if (CheckMemory())
		ICPU._Zero = Registers.AL & Immediate8Slow(READ);
	else
		ICPU._Zero = (Registers.A.W & Immediate16Slow(READ)) != 0;
}

rOPM (24Slow,   DirectSlow,                       WRAP_BANK, BIT)

rOPM (34Slow,   DirectIndexedXSlow,               WRAP_BANK, BIT)

rOPM (2CSlow,   AbsoluteSlow,                     WRAP_NONE, BIT)

rOPM (3CSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, BIT)

/* CMP ********************************************************************* */

static void OpC9M1 (void)
{
	int16	Int16 = (int16) Registers.AL - (int16) Immediate8(READ);
	ICPU._Carry = Int16 >= 0;
	SetZN((uint8) Int16);
}

static void OpC9M0 (void)
{
	int32	Int32 = (int32) Registers.A.W - (int32) Immediate16(READ);
	ICPU._Carry = Int32 >= 0;
	SetZN((uint16) Int32);
}

static void OpC9Slow (void)
{
	if (CheckMemory())
	{
		int16	Int16 = (int16) Registers.AL - (int16) Immediate8Slow(READ);
		ICPU._Carry = Int16 >= 0;
		SetZN((uint8) Int16);
	}
	else
	{
		int32	Int32 = (int32) Registers.A.W - (int32) Immediate16Slow(READ);
		ICPU._Carry = Int32 >= 0;
		SetZN((uint16) Int32);
	}
}

rOPM (C5Slow,   DirectSlow,                       WRAP_BANK, CMP)

rOPM (D5Slow,   DirectIndexedXSlow,               WRAP_BANK, CMP)

rOPM (D2Slow,   DirectIndirectSlow,               WRAP_NONE, CMP)

rOPM (C1Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, CMP)

rOPM (D1Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, CMP)

rOPM (C7Slow,   DirectIndirectLongSlow,           WRAP_NONE, CMP)

rOPM (D7Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, CMP)

rOPM (CDSlow,   AbsoluteSlow,                     WRAP_NONE, CMP)

rOPM (DDSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, CMP)

rOPM (D9Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, CMP)

rOPM (CFSlow,   AbsoluteLongSlow,                 WRAP_NONE, CMP)

rOPM (DFSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, CMP)

rOPM (C3Slow,   StackRelativeSlow,                WRAP_NONE, CMP)

rOPM (D3Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, CMP)

/* CPX ********************************************************************* */

static void OpE0X1 (void)
{
	int16	Int16 = (int16) Registers.XL - (int16) Immediate8(READ);
	ICPU._Carry = Int16 >= 0;
	SetZN((uint8) Int16);
}

static void OpE0X0 (void)
{
	int32	Int32 = (int32) Registers.X.W - (int32) Immediate16(READ);
	ICPU._Carry = Int32 >= 0;
	SetZN((uint16) Int32);
}

static void OpE0Slow (void)
{
	if (CheckIndex())
	{
		int16	Int16 = (int16) Registers.XL - (int16) Immediate8Slow(READ);
		ICPU._Carry = Int16 >= 0;
		SetZN((uint8) Int16);
	}
	else
	{
		int32	Int32 = (int32) Registers.X.W - (int32) Immediate16Slow(READ);
		ICPU._Carry = Int32 >= 0;
		SetZN((uint16) Int32);
	}
}

rOPX (E4Slow,   DirectSlow,                       WRAP_BANK, CPX)

rOPX (ECSlow,   AbsoluteSlow,                     WRAP_NONE, CPX)

/* CPY ********************************************************************* */

static void OpC0X1 (void)
{
	int16	Int16 = (int16) Registers.YL - (int16) Immediate8(READ);
	ICPU._Carry = Int16 >= 0;
	SetZN((uint8) Int16);
}

static void OpC0X0 (void)
{
	int32	Int32 = (int32) Registers.Y.W - (int32) Immediate16(READ);
	ICPU._Carry = Int32 >= 0;
	SetZN((uint16) Int32);
}

static void OpC0Slow (void)
{
	if (CheckIndex())
	{
		int16	Int16 = (int16) Registers.YL - (int16) Immediate8Slow(READ);
		ICPU._Carry = Int16 >= 0;
		SetZN((uint8) Int16);
	}
	else
	{
		int32	Int32 = (int32) Registers.Y.W - (int32) Immediate16Slow(READ);
		ICPU._Carry = Int32 >= 0;
		SetZN((uint16) Int32);
	}
}

rOPX (C4Slow,   DirectSlow,                       WRAP_BANK, CPY)

rOPX (CCSlow,   AbsoluteSlow,                     WRAP_NONE, CPY)

/* DEC ********************************************************************* */

static void Op3AM1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.AL--;
	SetZN(Registers.AL);
}

static void Op3AM0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.A.W--;
	SetZN(Registers.A.W);
}

static void Op3ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		Registers.AL--;
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W--;
		SetZN(Registers.A.W);
	}
}

mOPM (C6Slow,   DirectSlow,                       WRAP_BANK, DEC)

mOPM (D6Slow,   DirectIndexedXSlow,               WRAP_BANK, DEC)

mOPM (CESlow,   AbsoluteSlow,                     WRAP_NONE, DEC)

mOPM (DESlow,   AbsoluteIndexedXSlow,             WRAP_NONE, DEC)

/* EOR ********************************************************************* */

static void Op49M1 (void)
{
	Registers.AL ^= Immediate8(READ);
	SetZN(Registers.AL);
}

static void Op49M0 (void)
{
	Registers.A.W ^= Immediate16(READ);
	SetZN(Registers.A.W);
}

static void Op49Slow (void)
{
	if (CheckMemory())
	{
		Registers.AL ^= Immediate8Slow(READ);
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W ^= Immediate16Slow(READ);
		SetZN(Registers.A.W);
	}
}

rOPM (45Slow,   DirectSlow,                       WRAP_BANK, EOR)

rOPM (55Slow,   DirectIndexedXSlow,               WRAP_BANK, EOR)

rOPM (52Slow,   DirectIndirectSlow,               WRAP_NONE, EOR)

rOPM (41Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, EOR)

rOPM (51Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, EOR)

rOPM (47Slow,   DirectIndirectLongSlow,           WRAP_NONE, EOR)

rOPM (57Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, EOR)

rOPM (4DSlow,   AbsoluteSlow,                     WRAP_NONE, EOR)

rOPM (5DSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, EOR)

rOPM (59Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, EOR)

rOPM (4FSlow,   AbsoluteLongSlow,                 WRAP_NONE, EOR)

rOPM (5FSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, EOR)

rOPM (43Slow,   StackRelativeSlow,                WRAP_NONE, EOR)

rOPM (53Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, EOR)

/* INC ********************************************************************* */

static void Op1AM1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.AL++;
	SetZN(Registers.AL);
}

static void Op1AM0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.A.W++;
	SetZN(Registers.A.W);
}

static void Op1ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		Registers.AL++;
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W++;
		SetZN(Registers.A.W);
	}
}

mOPM (E6Slow,   DirectSlow,                       WRAP_BANK, INC)

mOPM (F6Slow,   DirectIndexedXSlow,               WRAP_BANK, INC)

mOPM (EESlow,   AbsoluteSlow,                     WRAP_NONE, INC)

mOPM (FESlow,   AbsoluteIndexedXSlow,             WRAP_NONE, INC)

/* LDA ********************************************************************* */

static void OpA9M1 (void)
{
	Registers.AL = Immediate8(READ);
	SetZN(Registers.AL);
}

static void OpA9M0 (void)
{
	Registers.A.W = Immediate16(READ);
	SetZN(Registers.A.W);
}

static void OpA9Slow (void)
{
	if (CheckMemory())
	{
		Registers.AL = Immediate8Slow(READ);
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W = Immediate16Slow(READ);
		SetZN(Registers.A.W);
	}
}

rOPM (A5Slow,   DirectSlow,                       WRAP_BANK, LDA)

rOPM (B5Slow,   DirectIndexedXSlow,               WRAP_BANK, LDA)

rOPM (B2Slow,   DirectIndirectSlow,               WRAP_NONE, LDA)

rOPM (A1Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, LDA)

rOPM (B1Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, LDA)

rOPM (A7Slow,   DirectIndirectLongSlow,           WRAP_NONE, LDA)

rOPM (B7Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, LDA)

rOPM (ADSlow,   AbsoluteSlow,                     WRAP_NONE, LDA)

rOPM (BDSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, LDA)

rOPM (B9Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, LDA)

rOPM (AFSlow,   AbsoluteLongSlow,                 WRAP_NONE, LDA)

rOPM (BFSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, LDA)

rOPM (A3Slow,   StackRelativeSlow,                WRAP_NONE, LDA)

rOPM (B3Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, LDA)

/* LDX ********************************************************************* */

static void OpA2X1 (void)
{
	Registers.XL = Immediate8(READ);
	SetZN(Registers.XL);
}

static void OpA2X0 (void)
{
	Registers.X.W = Immediate16(READ);
	SetZN(Registers.X.W);
}

static void OpA2Slow (void)
{
	if (CheckIndex())
	{
		Registers.XL = Immediate8Slow(READ);
		SetZN(Registers.XL);
	}
	else
	{
		Registers.X.W = Immediate16Slow(READ);
		SetZN(Registers.X.W);
	}
}

rOPX (A6Slow,   DirectSlow,                       WRAP_BANK, LDX)

rOPX (B6Slow,   DirectIndexedYSlow,               WRAP_BANK, LDX)

rOPX (AESlow,   AbsoluteSlow,                     WRAP_BANK, LDX)

rOPX (BESlow,   AbsoluteIndexedYSlow,             WRAP_BANK, LDX)

/* LDY ********************************************************************* */

static void OpA0X1 (void)
{
	Registers.YL = Immediate8(READ);
	SetZN(Registers.YL);
}

static void OpA0X0 (void)
{
	Registers.Y.W = Immediate16(READ);
	SetZN(Registers.Y.W);
}

static void OpA0Slow (void)
{
	if (CheckIndex())
	{
		Registers.YL = Immediate8Slow(READ);
		SetZN(Registers.YL);
	}
	else
	{
		Registers.Y.W = Immediate16Slow(READ);
		SetZN(Registers.Y.W);
	}
}

rOPX (A4Slow,   DirectSlow,                       WRAP_BANK, LDY)

rOPX (B4Slow,   DirectIndexedXSlow,               WRAP_BANK, LDY)

rOPX (ACSlow,   AbsoluteSlow,                     WRAP_BANK, LDY)

rOPX (BCSlow,   AbsoluteIndexedXSlow,             WRAP_BANK, LDY)

/* LSR ********************************************************************* */

static void Op4AM1 (void)
{
	AddCycles(ONE_CYCLE);
	ICPU._Carry = Registers.AL & 1;
	Registers.AL >>= 1;
	SetZN(Registers.AL);
}

static void Op4AM0 (void)
{
	AddCycles(ONE_CYCLE);
	ICPU._Carry = Registers.A.W & 1;
	Registers.A.W >>= 1;
	SetZN(Registers.A.W);
}

static void Op4ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		ICPU._Carry = Registers.AL & 1;
		Registers.AL >>= 1;
		SetZN(Registers.AL);
	}
	else
	{
		ICPU._Carry = Registers.A.W & 1;
		Registers.A.W >>= 1;
		SetZN(Registers.A.W);
	}
}

mOPM (46Slow,   DirectSlow,                       WRAP_BANK, LSR)

mOPM (56Slow,   DirectIndexedXSlow,               WRAP_BANK, LSR)

mOPM (4ESlow,   AbsoluteSlow,                     WRAP_NONE, LSR)

mOPM (5ESlow,   AbsoluteIndexedXSlow,             WRAP_NONE, LSR)

/* ORA ********************************************************************* */

static void Op09M1 (void)
{
	Registers.AL |= Immediate8(READ);
	SetZN(Registers.AL);
}

static void Op09M0 (void)
{
	Registers.A.W |= Immediate16(READ);
	SetZN(Registers.A.W);
}

static void Op09Slow (void)
{
	if (CheckMemory())
	{
		Registers.AL |= Immediate8Slow(READ);
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W |= Immediate16Slow(READ);
		SetZN(Registers.A.W);
	}
}

rOPM (05Slow,   DirectSlow,                       WRAP_BANK, ORA)

rOPM (15Slow,   DirectIndexedXSlow,               WRAP_BANK, ORA)

rOPM (12Slow,   DirectIndirectSlow,               WRAP_NONE, ORA)

rOPM (01Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, ORA)

rOPM (11Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, ORA)

rOPM (07Slow,   DirectIndirectLongSlow,           WRAP_NONE, ORA)

rOPM (17Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, ORA)

rOPM (0DSlow,   AbsoluteSlow,                     WRAP_NONE, ORA)

rOPM (1DSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, ORA)

rOPM (19Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, ORA)

rOPM (0FSlow,   AbsoluteLongSlow,                 WRAP_NONE, ORA)

rOPM (1FSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, ORA)

rOPM (03Slow,   StackRelativeSlow,                WRAP_NONE, ORA)

rOPM (13Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, ORA)

/* ROL ********************************************************************* */

static void Op2AM1 (void)
{
	AddCycles(ONE_CYCLE);
	uint16	w = (((uint16) Registers.AL) << 1) | CheckCarry();
	ICPU._Carry = w >= 0x100;
	Registers.AL = (uint8) w;
	SetZN(Registers.AL);
}

static void Op2AM0 (void)
{
	AddCycles(ONE_CYCLE);
	uint32	w = (((uint32) Registers.A.W) << 1) | CheckCarry();
	ICPU._Carry = w >= 0x10000;
	Registers.A.W = (uint16) w;
	SetZN(Registers.A.W);
}

static void Op2ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		uint16	w = (((uint16) Registers.AL) << 1) | CheckCarry();
		ICPU._Carry = w >= 0x100;
		Registers.AL = (uint8) w;
		SetZN(Registers.AL);
	}
	else
	{
		uint32	w = (((uint32) Registers.A.W) << 1) | CheckCarry();
		ICPU._Carry = w >= 0x10000;
		Registers.A.W = (uint16) w;
		SetZN(Registers.A.W);
	}
}

mOPM (26Slow,   DirectSlow,                       WRAP_BANK, ROL)

mOPM (36Slow,   DirectIndexedXSlow,               WRAP_BANK, ROL)

mOPM (2ESlow,   AbsoluteSlow,                     WRAP_NONE, ROL)

mOPM (3ESlow,   AbsoluteIndexedXSlow,             WRAP_NONE, ROL)

/* ROR ********************************************************************* */

static void Op6AM1 (void)
{
	AddCycles(ONE_CYCLE);
	uint16	w = ((uint16) Registers.AL) | (((uint16) CheckCarry()) << 8);
	ICPU._Carry = w & 1;
	w >>= 1;
	Registers.AL = (uint8) w;
	SetZN(Registers.AL);
}

static void Op6AM0 (void)
{
	AddCycles(ONE_CYCLE);
	uint32	w = ((uint32) Registers.A.W) | (((uint32) CheckCarry()) << 16);
	ICPU._Carry = w & 1;
	w >>= 1;
	Registers.A.W = (uint16) w;
	SetZN(Registers.A.W);
}

static void Op6ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		uint16	w = ((uint16) Registers.AL) | (((uint16) CheckCarry()) << 8);
		ICPU._Carry = w & 1;
		w >>= 1;
		Registers.AL = (uint8) w;
		SetZN(Registers.AL);
	}
	else
	{
		uint32	w = ((uint32) Registers.A.W) | (((uint32) CheckCarry()) << 16);
		ICPU._Carry = w & 1;
		w >>= 1;
		Registers.A.W = (uint16) w;
		SetZN(Registers.A.W);
	}
}

mOPM (66Slow,   DirectSlow,                       WRAP_BANK, ROR)

mOPM (76Slow,   DirectIndexedXSlow,               WRAP_BANK, ROR)

mOPM (6ESlow,   AbsoluteSlow,                     WRAP_NONE, ROR)

mOPM (7ESlow,   AbsoluteIndexedXSlow,             WRAP_NONE, ROR)

/* SBC ********************************************************************* */

static void OpE9M1 (void)
{
	SBC(Immediate8(READ));
}

static void OpE9M0 (void)
{
	SBC(Immediate16(READ));
}

static void OpE9Slow (void)
{
	if (CheckMemory())
		SBC(Immediate8Slow(READ));
	else
		SBC(Immediate16Slow(READ));
}

rOPM (E5Slow,   DirectSlow,                       WRAP_BANK, SBC)

rOPM (F5Slow,   DirectIndexedXSlow,               WRAP_BANK, SBC)

rOPM (F2Slow,   DirectIndirectSlow,               WRAP_NONE, SBC)

rOPM (E1Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, SBC)

rOPM (F1Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, SBC)

rOPM (E7Slow,   DirectIndirectLongSlow,           WRAP_NONE, SBC)

rOPM (F7Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, SBC)

rOPM (EDSlow,   AbsoluteSlow,                     WRAP_NONE, SBC)

rOPM (FDSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, SBC)

rOPM (F9Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, SBC)

rOPM (EFSlow,   AbsoluteLongSlow,                 WRAP_NONE, SBC)

rOPM (FFSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, SBC)

rOPM (E3Slow,   StackRelativeSlow,                WRAP_NONE, SBC)

rOPM (F3Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, SBC)

/* STA ********************************************************************* */

wOPM (85Slow,   DirectSlow,                       WRAP_BANK, STA)

wOPM (95Slow,   DirectIndexedXSlow,               WRAP_BANK, STA)

wOPM (92Slow,   DirectIndirectSlow,               WRAP_NONE, STA)

wOPM (81Slow,   DirectIndexedIndirectSlow,        WRAP_NONE, STA)

wOPM (91Slow,   DirectIndirectIndexedSlow,        WRAP_NONE, STA)

wOPM (87Slow,   DirectIndirectLongSlow,           WRAP_NONE, STA)

wOPM (97Slow,   DirectIndirectIndexedLongSlow,    WRAP_NONE, STA)

wOPM (8DSlow,   AbsoluteSlow,                     WRAP_NONE, STA)

wOPM (9DSlow,   AbsoluteIndexedXSlow,             WRAP_NONE, STA)

wOPM (99Slow,   AbsoluteIndexedYSlow,             WRAP_NONE, STA)

wOPM (8FSlow,   AbsoluteLongSlow,                 WRAP_NONE, STA)

wOPM (9FSlow,   AbsoluteLongIndexedXSlow,         WRAP_NONE, STA)

wOPM (83Slow,   StackRelativeSlow,                WRAP_NONE, STA)

wOPM (93Slow,   StackRelativeIndirectIndexedSlow, WRAP_NONE, STA)

/* STX ********************************************************************* */

wOPX (86Slow,   DirectSlow,                       WRAP_BANK, STX)

wOPX (96Slow,   DirectIndexedYSlow,               WRAP_BANK, STX)

wOPX (8ESlow,   AbsoluteSlow,                     WRAP_BANK, STX)

/* STY ********************************************************************* */

wOPX (84Slow,   DirectSlow,                       WRAP_BANK, STY)

wOPX (94Slow,   DirectIndexedXSlow,               WRAP_BANK, STY)

wOPX (8CSlow,   AbsoluteSlow,                     WRAP_BANK, STY)

/* STZ ********************************************************************* */

wOPM (64Slow,   DirectSlow,                       WRAP_BANK, STZ)

wOPM (74Slow,   DirectIndexedXSlow,               WRAP_BANK, STZ)

wOPM (9CSlow,   AbsoluteSlow,                     WRAP_NONE, STZ)

wOPM (9ESlow,   AbsoluteIndexedXSlow,             WRAP_NONE, STZ)

/* TRB ********************************************************************* */

mOPM (14Slow,   DirectSlow,                       WRAP_BANK, TRB)

mOPM (1CSlow,   AbsoluteSlow,                     WRAP_BANK, TRB)

/* TSB ********************************************************************* */

mOPM (04Slow,   DirectSlow,                       WRAP_BANK, TSB)

mOPM (0CSlow,   AbsoluteSlow,                     WRAP_BANK, TSB)

/* Branch Instructions ***************************************************** */

// BCC
bOP(90Slow, RelativeSlow, !CheckCarry(),    0, CheckEmulation())

// BCS
bOP(B0Slow, RelativeSlow,  CheckCarry(),    0, CheckEmulation())

// BEQ
bOP(F0Slow, RelativeSlow,  CheckZero(),     2, CheckEmulation())

// BMI
bOP(30Slow, RelativeSlow,  CheckNegative(), 1, CheckEmulation())

// BNE
bOP(D0Slow, RelativeSlow, !CheckZero(),     1, CheckEmulation())

// BPL
bOP(10Slow, RelativeSlow, !CheckNegative(), 1, CheckEmulation())

// BRA
bOP(80Slow, RelativeSlow, 1,                X, CheckEmulation())

// BVC
bOP(50Slow, RelativeSlow, !CheckOverflow(), 0, CheckEmulation())

// BVS
bOP(70Slow, RelativeSlow,  CheckOverflow(), 0, CheckEmulation())

// BRL
static void Op82 (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + RelativeLong(JUMP));
}

static void Op82Slow (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + RelativeLongSlow(JUMP));
}

/* Flag Instructions ******************************************************* */

// CLC
static void Op18 (void)
{
	ClearCarry();
	AddCycles(ONE_CYCLE);
}

// SEC
static void Op38 (void)
{
	SetCarry();
	AddCycles(ONE_CYCLE);
}

// CLD
static void OpD8 (void)
{
	ClearDecimal();
	AddCycles(ONE_CYCLE);
}

// SED
static void OpF8 (void)
{
	SetDecimal();
	AddCycles(ONE_CYCLE);
#ifdef DEBUGGER
	missing.decimal_mode = 1;
#endif
}

// CLI
static void Op58 (void)
{
	AddCycles(ONE_CYCLE);

#ifndef SA1_OPCODES
#else
	ClearIRQ();
#endif
}

// SEI
static void Op78 (void)
{
	AddCycles(ONE_CYCLE);

#ifndef SA1_OPCODES
#else
	SetIRQ();
#endif
}

// CLV
static void OpB8 (void)
{
	ClearOverflow();
	AddCycles(ONE_CYCLE);
}

/* DEX/DEY ***************************************************************** */

static void OpCAX1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.XL--;
	SetZN(Registers.XL);
}

static void OpCAX0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.X.W--;
	SetZN(Registers.X.W);
}

static void OpCASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.XL--;
		SetZN(Registers.XL);
	}
	else
	{
		Registers.X.W--;
		SetZN(Registers.X.W);
	}
}

static void Op88X1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.YL--;
	SetZN(Registers.YL);
}

static void Op88X0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.Y.W--;
	SetZN(Registers.Y.W);
}

static void Op88Slow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.YL--;
		SetZN(Registers.YL);
	}
	else
	{
		Registers.Y.W--;
		SetZN(Registers.Y.W);
	}
}

/* INX/INY ***************************************************************** */

static void OpE8X1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.XL++;
	SetZN(Registers.XL);
}

static void OpE8X0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.X.W++;
	SetZN(Registers.X.W);
}

static void OpE8Slow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.XL++;
		SetZN(Registers.XL);
	}
	else
	{
		Registers.X.W++;
		SetZN(Registers.X.W);
	}
}

static void OpC8X1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.YL++;
	SetZN(Registers.YL);
}

static void OpC8X0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.Y.W++;
	SetZN(Registers.Y.W);
}

static void OpC8Slow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.YL++;
		SetZN(Registers.YL);
	}
	else
	{
		Registers.Y.W++;
		SetZN(Registers.Y.W);
	}
}

/* NOP ********************************************************************* */

static void OpEA (void)
{
	AddCycles(ONE_CYCLE);
}

/* PUSH Instructions ******************************************************* */

#define PushW(w) \
	S9xSetWord(w, Registers.S.W - 1, WRAP_BANK, WRITE_10); \
	Registers.S.W -= 2;

#define PushWE(w) \
	Registers.SL--; \
	S9xSetWord(w, Registers.S.W, WRAP_PAGE, WRITE_10); \
	Registers.SL--;

#define PushB(b) \
	S9xSetByte(b, Registers.S.W--);

#define PushBE(b) \
	S9xSetByte(b, Registers.S.W); \
	Registers.SL--;

// PEA
static void OpF4E0 (void)
{
	uint16	val = (uint16) Absolute(NONE);
	PushW(val);
	OpenBus = val & 0xff;
}

static void OpF4E1 (void)
{
	// Note: PEA is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	uint16	val = (uint16) Absolute(NONE);
	PushW(val);
	OpenBus = val & 0xff;
	Registers.SH = 1;
}

static void OpF4Slow (void)
{
	uint16	val = (uint16) AbsoluteSlow(NONE);
	PushW(val);
	OpenBus = val & 0xff;
	if (CheckEmulation())
		Registers.SH = 1;
}

// PEI
static void OpD4E0 (void)
{
	uint16	val = (uint16) DirectIndirectE0(NONE);
	PushW(val);
	OpenBus = val & 0xff;
}

static void OpD4E1 (void)
{
	// Note: PEI is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	uint16	val = (uint16) DirectIndirectE1(NONE);
	PushW(val);
	OpenBus = val & 0xff;
	Registers.SH = 1;
}

static void OpD4Slow (void)
{
	uint16	val = (uint16) DirectIndirectSlow(NONE);
	PushW(val);
	OpenBus = val & 0xff;
	if (CheckEmulation())
		Registers.SH = 1;
}

// PER
static void Op62E0 (void)
{
	uint16	val = (uint16) RelativeLong(NONE);
	PushW(val);
	OpenBus = val & 0xff;
}

static void Op62E1 (void)
{
	// Note: PER is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	uint16	val = (uint16) RelativeLong(NONE);
	PushW(val);
	OpenBus = val & 0xff;
	Registers.SH = 1;
}

static void Op62Slow (void)
{
	uint16	val = (uint16) RelativeLongSlow(NONE);
	PushW(val);
	OpenBus = val & 0xff;
	if (CheckEmulation())
		Registers.SH = 1;
}

// PHA
static void Op48E1 (void)
{
	AddCycles(ONE_CYCLE);
	PushBE(Registers.AL);
	OpenBus = Registers.AL;
}

static void Op48E0M1 (void)
{
	AddCycles(ONE_CYCLE);
	PushB(Registers.AL);
	OpenBus = Registers.AL;
}

static void Op48E0M0 (void)
{
	AddCycles(ONE_CYCLE);
	PushW(Registers.A.W);
	OpenBus = Registers.AL;
}

static void Op48Slow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		PushBE(Registers.AL);
	}
	else
	if (CheckMemory())
	{
		PushB(Registers.AL);
	}
	else
	{
		PushW(Registers.A.W);
	}

	OpenBus = Registers.AL;
}

// PHB
static void Op8BE1 (void)
{
	AddCycles(ONE_CYCLE);
	PushBE(Registers.DB);
	OpenBus = Registers.DB;
}

static void Op8BE0 (void)
{
	AddCycles(ONE_CYCLE);
	PushB(Registers.DB);
	OpenBus = Registers.DB;
}

static void Op8BSlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		PushBE(Registers.DB);
	}
	else
	{
		PushB(Registers.DB);
	}

	OpenBus = Registers.DB;
}

// PHD
static void Op0BE0 (void)
{
	AddCycles(ONE_CYCLE);
	PushW(Registers.D.W);
	OpenBus = Registers.DL;
}

static void Op0BE1 (void)
{
	// Note: PHD is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	AddCycles(ONE_CYCLE);
	PushW(Registers.D.W);
	OpenBus = Registers.DL;
	Registers.SH = 1;
}

static void Op0BSlow (void)
{
	AddCycles(ONE_CYCLE);
	PushW(Registers.D.W);
	OpenBus = Registers.DL;
	if (CheckEmulation())
		Registers.SH = 1;
}

// PHK
static void Op4BE1 (void)
{
	AddCycles(ONE_CYCLE);
	PushBE(Registers.PB);
	OpenBus = Registers.PB;
}

static void Op4BE0 (void)
{
	AddCycles(ONE_CYCLE);
	PushB(Registers.PB);
	OpenBus = Registers.PB;
}

static void Op4BSlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		PushBE(Registers.PB);
	}
	else
	{
		PushB(Registers.PB);
	}

	OpenBus = Registers.PB;
}

// PHP
static void Op08E0 (void)
{
	S9xPackStatus();
	AddCycles(ONE_CYCLE);
	PushB(Registers.PL);
	OpenBus = Registers.PL;
}

static void Op08E1 (void)
{
	S9xPackStatus();
	AddCycles(ONE_CYCLE);
	PushBE(Registers.PL);
	OpenBus = Registers.PL;
}

static void Op08Slow (void)
{
	S9xPackStatus();
	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		PushBE(Registers.PL);
	}
	else
	{
		PushB(Registers.PL);
	}

	OpenBus = Registers.PL;
}

// PHX
static void OpDAE1 (void)
{
	AddCycles(ONE_CYCLE);
	PushBE(Registers.XL);
	OpenBus = Registers.XL;
}

static void OpDAE0X1 (void)
{
	AddCycles(ONE_CYCLE);
	PushB(Registers.XL);
	OpenBus = Registers.XL;
}

static void OpDAE0X0 (void)
{
	AddCycles(ONE_CYCLE);
	PushW(Registers.X.W);
	OpenBus = Registers.XL;
}

static void OpDASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		PushBE(Registers.XL);
	}
	else
	if (CheckIndex())
	{
		PushB(Registers.XL);
	}
	else
	{
		PushW(Registers.X.W);
	}

	OpenBus = Registers.XL;
}

// PHY
static void Op5AE1 (void)
{
	AddCycles(ONE_CYCLE);
	PushBE(Registers.YL);
	OpenBus = Registers.YL;
}

static void Op5AE0X1 (void)
{
	AddCycles(ONE_CYCLE);
	PushB(Registers.YL);
	OpenBus = Registers.YL;
}

static void Op5AE0X0 (void)
{
	AddCycles(ONE_CYCLE);
	PushW(Registers.Y.W);
	OpenBus = Registers.YL;
}

static void Op5ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		PushBE(Registers.YL);
	}
	else
	if (CheckIndex())
	{
		PushB(Registers.YL);
	}
	else
	{
		PushW(Registers.Y.W);
	}

	OpenBus = Registers.YL;
}

/* PULL Instructions ******************************************************* */

#define PullW(w) \
	w = S9xGetWord(Registers.S.W + 1, WRAP_BANK); \
	Registers.S.W += 2;

#define PullWE(w) \
	Registers.SL++; \
	w = S9xGetWord(Registers.S.W, WRAP_PAGE); \
	Registers.SL++;

#define PullB(b) \
	b = S9xGetByte(++Registers.S.W);

#define PullBE(b) \
	Registers.SL++; \
	b = S9xGetByte(Registers.S.W);

// PLA
static void Op68E1 (void)
{
	AddCycles(TWO_CYCLES);
	PullBE(Registers.AL);
	SetZN(Registers.AL);
	OpenBus = Registers.AL;
}

static void Op68E0M1 (void)
{
	AddCycles(TWO_CYCLES);
	PullB(Registers.AL);
	SetZN(Registers.AL);
	OpenBus = Registers.AL;
}

static void Op68E0M0 (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.A.W);
	SetZN(Registers.A.W);
	OpenBus = Registers.AH;
}

static void Op68Slow (void)
{
	AddCycles(TWO_CYCLES);

	if (CheckEmulation())
	{
		PullBE(Registers.AL);
		SetZN(Registers.AL);
		OpenBus = Registers.AL;
	}
	else
	if (CheckMemory())
	{
		PullB(Registers.AL);
		SetZN(Registers.AL);
		OpenBus = Registers.AL;
	}
	else
	{
		PullW(Registers.A.W);
		SetZN(Registers.A.W);
		OpenBus = Registers.AH;
	}
}

// PLB
static void OpABE1 (void)
{
	AddCycles(TWO_CYCLES);
	PullBE(Registers.DB);
	SetZN(Registers.DB);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = Registers.DB;
}

static void OpABE0 (void)
{
	AddCycles(TWO_CYCLES);
	PullB(Registers.DB);
	SetZN(Registers.DB);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = Registers.DB;
}

static void OpABSlow (void)
{
	AddCycles(TWO_CYCLES);

	if (CheckEmulation())
	{
		PullBE(Registers.DB);
	}
	else
	{
		PullB(Registers.DB);
	}

	SetZN(Registers.DB);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = Registers.DB;
}

// PLD
static void Op2BE0 (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.D.W);
	SetZN(Registers.D.W);
	OpenBus = Registers.DH;
}

static void Op2BE1 (void)
{
	// Note: PLD is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	AddCycles(TWO_CYCLES);
	PullW(Registers.D.W);
	SetZN(Registers.D.W);
	OpenBus = Registers.DH;
	Registers.SH = 1;
}

static void Op2BSlow (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.D.W);
	SetZN(Registers.D.W);
	OpenBus = Registers.DH;
	if (CheckEmulation())
		Registers.SH = 1;
}

// PLP
static void Op28E1 (void)
{
	AddCycles(TWO_CYCLES);
	PullBE(Registers.PL);
	OpenBus = Registers.PL;
	SetFlags(MemoryFlag | IndexFlag);
	S9xUnpackStatus();
	S9xFixCycles();
	CHECK_FOR_IRQ();
}

static void Op28E0 (void)
{
	AddCycles(TWO_CYCLES);
	PullB(Registers.PL);
	OpenBus = Registers.PL;
	S9xUnpackStatus();

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
	CHECK_FOR_IRQ();
}

static void Op28Slow (void)
{
	AddCycles(TWO_CYCLES);

	if (CheckEmulation())
	{
		PullBE(Registers.PL);
		OpenBus = Registers.PL;
		SetFlags(MemoryFlag | IndexFlag);
	}
	else
	{
		PullB(Registers.PL);
		OpenBus = Registers.PL;
	}

	S9xUnpackStatus();

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
	CHECK_FOR_IRQ();
}

// PLX
static void OpFAE1 (void)
{
	AddCycles(TWO_CYCLES);
	PullBE(Registers.XL);
	SetZN(Registers.XL);
	OpenBus = Registers.XL;
}

static void OpFAE0X1 (void)
{
	AddCycles(TWO_CYCLES);
	PullB(Registers.XL);
	SetZN(Registers.XL);
	OpenBus = Registers.XL;
}

static void OpFAE0X0 (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.X.W);
	SetZN(Registers.X.W);
	OpenBus = Registers.XH;
}

static void OpFASlow (void)
{
	AddCycles(TWO_CYCLES);

	if (CheckEmulation())
	{
		PullBE(Registers.XL);
		SetZN(Registers.XL);
		OpenBus = Registers.XL;
	}
	else
	if (CheckIndex())
	{
		PullB(Registers.XL);
		SetZN(Registers.XL);
		OpenBus = Registers.XL;
	}
	else
	{
		PullW(Registers.X.W);
		SetZN(Registers.X.W);
		OpenBus = Registers.XH;
	}
}

// PLY
static void Op7AE1 (void)
{
	AddCycles(TWO_CYCLES);
	PullBE(Registers.YL);
	SetZN(Registers.YL);
	OpenBus = Registers.YL;
}

static void Op7AE0X1 (void)
{
	AddCycles(TWO_CYCLES);
	PullB(Registers.YL);
	SetZN(Registers.YL);
	OpenBus = Registers.YL;
}

static void Op7AE0X0 (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.Y.W);
	SetZN(Registers.Y.W);
	OpenBus = Registers.YH;
}

static void Op7ASlow (void)
{
	AddCycles(TWO_CYCLES);

	if (CheckEmulation())
	{
		PullBE(Registers.YL);
		SetZN(Registers.YL);
		OpenBus = Registers.YL;
	}
	else
	if (CheckIndex())
	{
		PullB(Registers.YL);
		SetZN(Registers.YL);
		OpenBus = Registers.YL;
	}
	else
	{
		PullW(Registers.Y.W);
		SetZN(Registers.Y.W);
		OpenBus = Registers.YH;
	}
}

/* Transfer Instructions *************************************************** */

// TAX
static void OpAAX1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.XL = Registers.AL;
	SetZN(Registers.XL);
}

static void OpAAX0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.X.W = Registers.A.W;
	SetZN(Registers.X.W);
}

static void OpAASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.XL = Registers.AL;
		SetZN(Registers.XL);
	}
	else
	{
		Registers.X.W = Registers.A.W;
		SetZN(Registers.X.W);
	}
}

// TAY
static void OpA8X1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.YL = Registers.AL;
	SetZN(Registers.YL);
}

static void OpA8X0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.Y.W = Registers.A.W;
	SetZN(Registers.Y.W);
}

static void OpA8Slow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.YL = Registers.AL;
		SetZN(Registers.YL);
	}
	else
	{
		Registers.Y.W = Registers.A.W;
		SetZN(Registers.Y.W);
	}
}

// TCD
static void Op5B (void)
{
	AddCycles(ONE_CYCLE);
	Registers.D.W = Registers.A.W;
	SetZN(Registers.D.W);
}

// TCS
static void Op1B (void)
{
	AddCycles(ONE_CYCLE);
	Registers.S.W = Registers.A.W;
	if (CheckEmulation())
		Registers.SH = 1;
}

// TDC
static void Op7B (void)
{
	AddCycles(ONE_CYCLE);
	Registers.A.W = Registers.D.W;
	SetZN(Registers.A.W);
}

// TSC
static void Op3B (void)
{
	AddCycles(ONE_CYCLE);
	Registers.A.W = Registers.S.W;
	SetZN(Registers.A.W);
}

// TSX
static void OpBAX1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.XL = Registers.SL;
	SetZN(Registers.XL);
}

static void OpBAX0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.X.W = Registers.S.W;
	SetZN(Registers.X.W);
}

static void OpBASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.XL = Registers.SL;
		SetZN(Registers.XL);
	}
	else
	{
		Registers.X.W = Registers.S.W;
		SetZN(Registers.X.W);
	}
}

// TXA
static void Op8AM1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.AL = Registers.XL;
	SetZN(Registers.AL);
}

static void Op8AM0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.A.W = Registers.X.W;
	SetZN(Registers.A.W);
}

static void Op8ASlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		Registers.AL = Registers.XL;
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W = Registers.X.W;
		SetZN(Registers.A.W);
	}
}

// TXS
static void Op9A (void)
{
	AddCycles(ONE_CYCLE);
	Registers.S.W = Registers.X.W;
	if (CheckEmulation())
		Registers.SH = 1;
}

// TXY
static void Op9BX1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.YL = Registers.XL;
	SetZN(Registers.YL);
}

static void Op9BX0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.Y.W = Registers.X.W;
	SetZN(Registers.Y.W);
}

static void Op9BSlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.YL = Registers.XL;
		SetZN(Registers.YL);
	}
	else
	{
		Registers.Y.W = Registers.X.W;
		SetZN(Registers.Y.W);
	}
}

// TYA
static void Op98M1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.AL = Registers.YL;
	SetZN(Registers.AL);
}

static void Op98M0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.A.W = Registers.Y.W;
	SetZN(Registers.A.W);
}

static void Op98Slow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckMemory())
	{
		Registers.AL = Registers.YL;
		SetZN(Registers.AL);
	}
	else
	{
		Registers.A.W = Registers.Y.W;
		SetZN(Registers.A.W);
	}
}

// TYX
static void OpBBX1 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.XL = Registers.YL;
	SetZN(Registers.XL);
}

static void OpBBX0 (void)
{
	AddCycles(ONE_CYCLE);
	Registers.X.W = Registers.Y.W;
	SetZN(Registers.X.W);
}

static void OpBBSlow (void)
{
	AddCycles(ONE_CYCLE);

	if (CheckIndex())
	{
		Registers.XL = Registers.YL;
		SetZN(Registers.XL);
	}
	else
	{
		Registers.X.W = Registers.Y.W;
		SetZN(Registers.X.W);
	}
}

/* XCE ********************************************************************* */

static void OpFB (void)
{
	AddCycles(ONE_CYCLE);

	uint8	A1 = ICPU._Carry;
	uint8	A2 = Registers.PH;

	ICPU._Carry = A2 & 1;
	Registers.PH = A1;

	if (CheckEmulation())
	{
		SetFlags(MemoryFlag | IndexFlag);
		Registers.SH = 1;
	#ifdef DEBUGGER
		missing.emulate6502 = 1;
	#endif
	}

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
}

/* BRK ********************************************************************* */

static void Op00 (void)
{
#ifdef DEBUGGER
	if (CPU.Flags & TRACE_FLAG)
		S9xTraceMessage("*** BRK");
#endif

	AddCycles(CPU.MemSpeed);

	uint16	addr;

	if (!CheckEmulation())
	{
		PushB(Registers.PB);
		PushW(Registers.PCw + 1);
		S9xPackStatus();
		PushB(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

		addr = S9xGetWord(0xFFE6);
	}
	else
	{
		PushWE(Registers.PCw + 1);
		S9xPackStatus();
		PushBE(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

		addr = S9xGetWord(0xFFFE);
	}

	S9xSetPCBase(addr);
	OpenBus = addr >> 8;
}

/* IRQ ********************************************************************* */

void S9xOpcode_IRQ (void)
{
#ifdef DEBUGGER
	if (CPU.Flags & TRACE_FLAG)
	#ifdef SA1_OPCODES
		S9xTraceMessage("*** SA1 IRQ");
	#else
		S9xTraceMessage("*** IRQ");
	#endif
#endif

	// IRQ and NMI do an opcode fetch as their first "IO" cycle.
	AddCycles(CPU.MemSpeed + ONE_CYCLE);

	if (!CheckEmulation())
	{
		PushB(Registers.PB);
		PushW(Registers.PCw);
		S9xPackStatus();
		PushB(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

	#ifdef SA1_OPCODES
		OpenBus = IRQJunk[0x08];
		AddCycles(2 * ONE_CYCLE);
		S9xSA1SetPCBase(IRQJunk[0x07] | (IRQJunk[0x08] << 8));
	#else
		if ((-1) && (IRQJunk[0x09] & 0x40))
		{
			OpenBus = IRQJunk[0x0f];
			AddCycles(2 * ONE_CYCLE);
			S9xSetPCBase(IRQJunk[0x0e] | (IRQJunk[0x0f] << 8));
		}
		else
		{
			uint16	addr = S9xGetWord(0xFFEE);
			OpenBus = addr >> 8;
			S9xSetPCBase(addr);
		}
	#endif
	}
	else
	{
		PushWE(Registers.PCw);
		S9xPackStatus();
		PushBE(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

	#ifdef SA1_OPCODES
		OpenBus = IRQJunk[0x08];
		AddCycles(2 * ONE_CYCLE);
		S9xSA1SetPCBase(IRQJunk[0x07] | (IRQJunk[0x08] << 8));
	#else
		if ((-1) && (IRQJunk[0x09] & 0x40))
		{
			OpenBus = IRQJunk[0x0f];
			AddCycles(2 * ONE_CYCLE);
			S9xSetPCBase(IRQJunk[0x0e] | (IRQJunk[0x0f] << 8));
		}
		else
		{
			uint16	addr = S9xGetWord(0xFFFE);
			OpenBus = addr >> 8;
			S9xSetPCBase(addr);
		}
	#endif
	}
}

/* NMI ********************************************************************* */

void S9xOpcode_NMI (void)
{
#ifdef DEBUGGER
	if (CPU.Flags & TRACE_FLAG)
	#ifdef SA1_OPCODES
		S9xTraceMessage("*** SA1 NMI");
	#else
		S9xTraceMessage("*** NMI");
	#endif
#endif

	// IRQ and NMI do an opcode fetch as their first "IO" cycle.
	AddCycles(CPU.MemSpeed + ONE_CYCLE);

	if (!CheckEmulation())
	{
		PushB(Registers.PB);
		PushW(Registers.PCw);
		S9xPackStatus();
		PushB(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

	#ifdef SA1_OPCODES
		OpenBus = IRQJunk[0x06];
		AddCycles(2 * ONE_CYCLE);
		S9xSA1SetPCBase(IRQJunk[0x05] | (IRQJunk[0x06] << 8));
	#else
		if ((-1) && (IRQJunk[0x09] & 0x10))
		{
			OpenBus = IRQJunk[0x0d];
			AddCycles(2 * ONE_CYCLE);
			S9xSetPCBase(IRQJunk[0x0c] | (IRQJunk[0x0d] << 8));
		}
		else
		{
			uint16	addr = S9xGetWord(0xFFEA);
			OpenBus = addr >> 8;
			S9xSetPCBase(addr);
		}
	#endif
	}
	else
	{
		PushWE(Registers.PCw);
		S9xPackStatus();
		PushBE(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

	#ifdef SA1_OPCODES
		OpenBus = IRQJunk[0x06];
		AddCycles(2 * ONE_CYCLE);
		S9xSA1SetPCBase(IRQJunk[0x05] | (IRQJunk[0x06] << 8));
	#else
		if ((-1) && (IRQJunk[0x09] & 0x10))
		{
			OpenBus = IRQJunk[0x0d];
			AddCycles(2 * ONE_CYCLE);
			S9xSetPCBase(IRQJunk[0x0c] | (IRQJunk[0x0d] << 8));
		}
		else
		{
			uint16	addr = S9xGetWord(0xFFFA);
			OpenBus = addr >> 8;
			S9xSetPCBase(addr);
		}
	#endif
	}
}

/* COP ********************************************************************* */

static void Op02 (void)
{
#ifdef DEBUGGER
	if (CPU.Flags & TRACE_FLAG)
		S9xTraceMessage("*** COP");
#endif

	AddCycles(CPU.MemSpeed);

	uint16	addr;

	if (!CheckEmulation())
	{
		PushB(Registers.PB);
		PushW(Registers.PCw + 1);
		S9xPackStatus();
		PushB(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

		addr = S9xGetWord(0xFFE4);
	}
	else
	{
		PushWE(Registers.PCw + 1);
		S9xPackStatus();
		PushBE(Registers.PL);
		OpenBus = Registers.PL;
		ClearDecimal();
		SetIRQ();

		addr = S9xGetWord(0xFFF4);
	}

	S9xSetPCBase(addr);
	OpenBus = addr >> 8;
}

/* JML ********************************************************************* */

static void OpDC (void)
{
	S9xSetPCBase(AbsoluteIndirectLong(JUMP));
#ifdef SA1_OPCODES
	AddCycles(ONE_CYCLE);
#endif
}

static void OpDCSlow (void)
{
	S9xSetPCBase(AbsoluteIndirectLongSlow(JUMP));
#ifdef SA1_OPCODES
	AddCycles(ONE_CYCLE);
#endif
}

static void Op5C (void)
{
	S9xSetPCBase(AbsoluteLong(JUMP));
#ifdef SA1_OPCODES
	AddCycles(ONE_CYCLE);
#endif
}

static void Op5CSlow (void)
{
	S9xSetPCBase(AbsoluteLongSlow(JUMP));
#ifdef SA1_OPCODES
	AddCycles(ONE_CYCLE);
#endif
}

/* JMP ********************************************************************* */

static void Op4C (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + ((uint16) Absolute(JUMP)));
}

static void Op4CSlow (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + ((uint16) AbsoluteSlow(JUMP)));
}

static void Op6C (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + ((uint16) AbsoluteIndirect(JUMP)));
}

static void Op6CSlow (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + ((uint16) AbsoluteIndirectSlow(JUMP)));
}

static void Op7C (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + ((uint16) AbsoluteIndexedIndirect(JUMP)));
}

static void Op7CSlow (void)
{
	S9xSetPCBase(ICPU.ShiftedPB + ((uint16) AbsoluteIndexedIndirectSlow(JUMP)));
}

/* JSL/RTL ***************************************************************** */

static void Op22E1 (void)
{
	// Note: JSL is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	uint32	addr = AbsoluteLong(JSR);
	PushB(Registers.PB);
	PushW(Registers.PCw - 1);
	Registers.SH = 1;
	S9xSetPCBase(addr);
}

static void Op22E0 (void)
{
	uint32	addr = AbsoluteLong(JSR);
	PushB(Registers.PB);
	PushW(Registers.PCw - 1);
	S9xSetPCBase(addr);
}

static void Op22Slow (void)
{
	uint32	addr = AbsoluteLongSlow(JSR);
	PushB(Registers.PB);
	PushW(Registers.PCw - 1);
	if (CheckEmulation())
		Registers.SH = 1;
	S9xSetPCBase(addr);
}

static void Op6BE1 (void)
{
	// Note: RTL is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	AddCycles(TWO_CYCLES);
	PullW(Registers.PCw);
	PullB(Registers.PB);
	Registers.SH = 1;
	Registers.PCw++;
	S9xSetPCBase(Registers.PBPC);
}

static void Op6BE0 (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.PCw);
	PullB(Registers.PB);
	Registers.PCw++;
	S9xSetPCBase(Registers.PBPC);
}

static void Op6BSlow (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.PCw);
	PullB(Registers.PB);
	if (CheckEmulation())
		Registers.SH = 1;
	Registers.PCw++;
	S9xSetPCBase(Registers.PBPC);
}

/* JSR/RTS ***************************************************************** */

static void Op20E1 (void)
{
	uint16	addr = Absolute(JSR);
	AddCycles(ONE_CYCLE);
	PushWE(Registers.PCw - 1);
	S9xSetPCBase(ICPU.ShiftedPB + addr);
}

static void Op20E0 (void)
{
	uint16	addr = Absolute(JSR);
	AddCycles(ONE_CYCLE);
	PushW(Registers.PCw - 1);
	S9xSetPCBase(ICPU.ShiftedPB + addr);
}

static void Op20Slow (void)
{
	uint16	addr = AbsoluteSlow(JSR);

	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		PushWE(Registers.PCw - 1);
	}
	else
	{
		PushW(Registers.PCw - 1);
	}

	S9xSetPCBase(ICPU.ShiftedPB + addr);
}

static void OpFCE1 (void)
{
	// Note: JSR (a,X) is a new instruction,
	// and so doesn't respect the emu-mode stack bounds.
	uint16	addr = AbsoluteIndexedIndirect(JSR);
	PushW(Registers.PCw - 1);
	Registers.SH = 1;
	S9xSetPCBase(ICPU.ShiftedPB + addr);
}

static void OpFCE0 (void)
{
	uint16	addr = AbsoluteIndexedIndirect(JSR);
	PushW(Registers.PCw - 1);
	S9xSetPCBase(ICPU.ShiftedPB + addr);
}

static void OpFCSlow (void)
{
	uint16	addr = AbsoluteIndexedIndirectSlow(JSR);
	PushW(Registers.PCw - 1);
	if (CheckEmulation())
		Registers.SH = 1;
	S9xSetPCBase(ICPU.ShiftedPB + addr);
}

static void Op60E1 (void)
{
	AddCycles(TWO_CYCLES);
	PullWE(Registers.PCw);
	AddCycles(ONE_CYCLE);
	Registers.PCw++;
	S9xSetPCBase(Registers.PBPC);
}

static void Op60E0 (void)
{
	AddCycles(TWO_CYCLES);
	PullW(Registers.PCw);
	AddCycles(ONE_CYCLE);
	Registers.PCw++;
	S9xSetPCBase(Registers.PBPC);
}

static void Op60Slow (void)
{
	AddCycles(TWO_CYCLES);

	if (CheckEmulation())
	{
		PullWE(Registers.PCw);
	}
	else
	{
		PullW(Registers.PCw);
	}

	AddCycles(ONE_CYCLE);
	Registers.PCw++;
	S9xSetPCBase(Registers.PBPC);
}

/* MVN/MVP ***************************************************************** */

static void Op54X1 (void)
{
	uint32	SrcBank;

	Registers.DB = Immediate8(NONE);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = SrcBank = Immediate8(NONE);

	S9xSetByte(OpenBus = S9xGetByte((SrcBank << 16) + Registers.X.W), ICPU.ShiftedDB + Registers.Y.W);

	Registers.XL++;
	Registers.YL++;
	Registers.A.W--;
	if (Registers.A.W != 0xffff)
		Registers.PCw -= 3;

	AddCycles(TWO_CYCLES);
}

static void Op54X0 (void)
{
	uint32	SrcBank;

	Registers.DB = Immediate8(NONE);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = SrcBank = Immediate8(NONE);

	S9xSetByte(OpenBus = S9xGetByte((SrcBank << 16) + Registers.X.W), ICPU.ShiftedDB + Registers.Y.W);

	Registers.X.W++;
	Registers.Y.W++;
	Registers.A.W--;
	if (Registers.A.W != 0xffff)
		Registers.PCw -= 3;

	AddCycles(TWO_CYCLES);
}

static void Op54Slow (void)
{
	uint32	SrcBank;

	OpenBus = Registers.DB = Immediate8Slow(NONE);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = SrcBank = Immediate8Slow(NONE);

	S9xSetByte(OpenBus = S9xGetByte((SrcBank << 16) + Registers.X.W), ICPU.ShiftedDB + Registers.Y.W);

	if (CheckIndex())
	{
		Registers.XL++;
		Registers.YL++;
	}
	else
	{
		Registers.X.W++;
		Registers.Y.W++;
	}

	Registers.A.W--;
	if (Registers.A.W != 0xffff)
		Registers.PCw -= 3;

	AddCycles(TWO_CYCLES);
}

static void Op44X1 (void)
{
	uint32	SrcBank;

	Registers.DB = Immediate8(NONE);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = SrcBank = Immediate8(NONE);

	S9xSetByte(OpenBus = S9xGetByte((SrcBank << 16) + Registers.X.W), ICPU.ShiftedDB + Registers.Y.W);

	Registers.XL--;
	Registers.YL--;
	Registers.A.W--;
	if (Registers.A.W != 0xffff)
		Registers.PCw -= 3;

	AddCycles(TWO_CYCLES);
}

static void Op44X0 (void)
{
	uint32	SrcBank;

	Registers.DB = Immediate8(NONE);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = SrcBank = Immediate8(NONE);

	S9xSetByte(OpenBus = S9xGetByte((SrcBank << 16) + Registers.X.W), ICPU.ShiftedDB + Registers.Y.W);

	Registers.X.W--;
	Registers.Y.W--;
	Registers.A.W--;
	if (Registers.A.W != 0xffff)
		Registers.PCw -= 3;

	AddCycles(TWO_CYCLES);
}

static void Op44Slow (void)
{
	uint32	SrcBank;

	OpenBus = Registers.DB = Immediate8Slow(NONE);
	ICPU.ShiftedDB = Registers.DB << 16;
	OpenBus = SrcBank = Immediate8Slow(NONE);

	S9xSetByte(OpenBus = S9xGetByte((SrcBank << 16) + Registers.X.W), ICPU.ShiftedDB + Registers.Y.W);

	if (CheckIndex())
	{
		Registers.XL--;
		Registers.YL--;
	}
	else
	{
		Registers.X.W--;
		Registers.Y.W--;
	}

	Registers.A.W--;
	if (Registers.A.W != 0xffff)
		Registers.PCw -= 3;

	AddCycles(TWO_CYCLES);
}

/* REP/SEP ***************************************************************** */

static void OpC2 (void)
{
	uint8	Work8 = ~Immediate8(READ);
	Registers.PL &= Work8;
	ICPU._Carry &= Work8;
	ICPU._Overflow &= (Work8 >> 6);
	ICPU._Negative &= Work8;
	ICPU._Zero |= ~Work8 & Zero;

	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		SetFlags(MemoryFlag | IndexFlag);
	#ifdef DEBUGGER
		missing.emulate6502 = 1;
	#endif
	}

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
	CHECK_FOR_IRQ();
}

static void OpC2Slow (void)
{
	uint8	Work8 = ~Immediate8Slow(READ);
	Registers.PL &= Work8;
	ICPU._Carry &= Work8;
	ICPU._Overflow &= (Work8 >> 6);
	ICPU._Negative &= Work8;
	ICPU._Zero |= ~Work8 & Zero;

	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		SetFlags(MemoryFlag | IndexFlag);
	#ifdef DEBUGGER
		missing.emulate6502 = 1;
	#endif
	}

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
	CHECK_FOR_IRQ();
}

static void OpE2 (void)
{
	uint8	Work8 = Immediate8(READ);
	Registers.PL |= Work8;
	ICPU._Carry |= Work8 & 1;
	ICPU._Overflow |= (Work8 >> 6) & 1;
	ICPU._Negative |= Work8;
	if (Work8 & Zero)
		ICPU._Zero = 0;

	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		SetFlags(MemoryFlag | IndexFlag);
	#ifdef DEBUGGER
		missing.emulate6502 = 1;
	#endif
	}

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
}

static void OpE2Slow (void)
{
	uint8	Work8 = Immediate8Slow(READ);
	Registers.PL |= Work8;
	ICPU._Carry |= Work8 & 1;
	ICPU._Overflow |= (Work8 >> 6) & 1;
	ICPU._Negative |= Work8;
	if (Work8 & Zero)
		ICPU._Zero = 0;

	AddCycles(ONE_CYCLE);

	if (CheckEmulation())
	{
		SetFlags(MemoryFlag | IndexFlag);
	#ifdef DEBUGGER
		missing.emulate6502 = 1;
	#endif
	}

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
}

/* XBA ********************************************************************* */

static void OpEB (void)
{
	uint8	Work8 = Registers.AL;
	Registers.AL = Registers.AH;
	Registers.AH = Work8;
	SetZN(Registers.AL);
	AddCycles(TWO_CYCLES);
}

/* RTI ********************************************************************* */

static void Op40Slow (void)
{
	AddCycles(TWO_CYCLES);

	if (!CheckEmulation())
	{
		PullB(Registers.PL);
		S9xUnpackStatus();
		PullW(Registers.PCw);
		PullB(Registers.PB);
		OpenBus = Registers.PB;
		ICPU.ShiftedPB = Registers.PB << 16;
	}
	else
	{
		PullBE(Registers.PL);
		S9xUnpackStatus();
		PullWE(Registers.PCw);
		OpenBus = Registers.PCh;
		SetFlags(MemoryFlag | IndexFlag);
	#ifdef DEBUGGER
		missing.emulate6502 = 1;
	#endif
	}

	S9xSetPCBase(Registers.PBPC);

	if (CheckIndex())
	{
		Registers.XH = 0;
		Registers.YH = 0;
	}

	S9xFixCycles();
	CHECK_FOR_IRQ();
}

/* STP/WAI ***************************************************************** */

// WAI
static void OpCB (void)
{
#ifdef SA1_OPCODES
	SA1.WaitingForInterrupt = TRUE;
	Registers.PCw--;
	AddCycles(TWO_CYCLES);
#else
	CPU.WaitingForInterrupt = TRUE;

	Registers.PCw--;
	AddCycles(ONE_CYCLE);
#endif
}

// STP
static void OpDB (void)
{
	Registers.PCw--;
	CPU.Flags |= DEBUG_MODE_FLAG | HALTED_FLAG;
}

/* WDM (Reserved S9xOpcode) ************************************************ */

#ifdef DEBUGGER
extern FILE	*trace, *trace2;
#endif

static void Op42 (void)
{
#ifdef DEBUGGER
	uint8	byte = (uint8) S9xGetWord(Registers.PBPC);
#else
	S9xGetWord(Registers.PBPC);
#endif
	Registers.PCw++;

#ifdef DEBUGGER
	// Hey, let's use this to trigger debug modes.
	switch (byte)
	{
		case 0xdb: // "STP" = Enter debug mode
			CPU.Flags |= DEBUG_MODE_FLAG;
			break;

	#ifndef SA1_OPCODES
		case 0xe2: // "SEP" = Trace on
			if (!(CPU.Flags & TRACE_FLAG))
			{
				char	buf[25];
				CPU.Flags |= TRACE_FLAG;
				snprintf(buf, 25, "WDM trace on at $%02X:%04X", Registers.PB, Registers.PCw);
				S9xMessage(S9X_DEBUG, S9X_DEBUG_OUTPUT, buf);
				if (trace != NULL)
					fclose(trace);
				ENSURE_TRACE_OPEN(trace, "WDMtrace.log", "ab")
			}

			break;

		case 0xc2: // "REP" = Trace off
			if (CPU.Flags & TRACE_FLAG)
			{
				char	buf[26];
				CPU.Flags &= ~TRACE_FLAG;
				snprintf(buf, 26, "WDM trace off at $%02X:%04X", Registers.PB, Registers.PCw);
				S9xMessage(S9X_DEBUG, S9X_DEBUG_OUTPUT, buf);
				if (trace != NULL)
					fclose(trace);
				trace = NULL;
			}

			break;
	#endif

		case 0x42: // "WDM" = Snapshot
			char	filename[PATH_MAX + 1], drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], def[PATH_MAX + 1], ext[_MAX_EXT + 1];

			_splitpath(Memory.ROMFilename, drive, dir, def, ext);
			snprintf(filename, PATH_MAX, "%s%s%s-%06X.wdm", S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, def, Registers.PBPC & 0xffffff);
			sprintf(def, "WDM Snapshot at $%02X:%04X: %s", Registers.PB, Registers.PCw, filename);
			S9xMessage(S9X_DEBUG, S9X_DEBUG_OUTPUT, def);
			S9xFreezeGame(filename);

			break;

		default:
			break;
	}
#endif
}

/* CPU-S9xOpcodes Definitions ************************************************/

struct SOpcodes S9xOpcodesM1X1[256] =
{
};

struct SOpcodes S9xOpcodesE1[256] =
{
};

struct SOpcodes S9xOpcodesM1X0[256] =
{
};

struct SOpcodes S9xOpcodesM0X0[256] =
{
};

struct SOpcodes S9xOpcodesM0X1[256] =
{
};

struct SOpcodes S9xOpcodesSlow[256] =
{
	{ Op00 },        { Op01Slow },    { Op02 },        { Op03Slow },    { Op04Slow },
	{ Op05Slow },    { Op06Slow },    { Op07Slow },    { Op08Slow },    { Op09Slow },
	{ Op0ASlow },    { Op0BSlow },    { Op0CSlow },    { Op0DSlow },    { Op0ESlow },
	{ Op0FSlow },    { Op10Slow },    { Op11Slow },    { Op12Slow },    { Op13Slow },
	{ Op14Slow },    { Op15Slow },    { Op16Slow },    { Op17Slow },    { Op18 },
	{ Op19Slow },    { Op1ASlow },    { Op1B },        { Op1CSlow },    { Op1DSlow },
	{ Op1ESlow },    { Op1FSlow },    { Op20Slow },    { Op21Slow },    { Op22Slow },
	{ Op23Slow },    { Op24Slow },    { Op25Slow },    { Op26Slow },    { Op27Slow },
	{ Op28Slow },    { Op29Slow },    { Op2ASlow },    { Op2BSlow },    { Op2CSlow },
	{ Op2DSlow },    { Op2ESlow },    { Op2FSlow },    { Op30Slow },    { Op31Slow },
	{ Op32Slow },    { Op33Slow },    { Op34Slow },    { Op35Slow },    { Op36Slow },
	{ Op37Slow },    { Op38 },        { Op39Slow },    { Op3ASlow },    { Op3B },
	{ Op3CSlow },    { Op3DSlow },    { Op3ESlow },    { Op3FSlow },    { Op40Slow },
	{ Op41Slow },    { Op42 },        { Op43Slow },    { Op44Slow },    { Op45Slow },
	{ Op46Slow },    { Op47Slow },    { Op48Slow },    { Op49Slow },    { Op4ASlow },
	{ Op4BSlow },    { Op4CSlow },    { Op4DSlow },    { Op4ESlow },    { Op4FSlow },
	{ Op50Slow },    { Op51Slow },    { Op52Slow },    { Op53Slow },    { Op54Slow },
	{ Op55Slow },    { Op56Slow },    { Op57Slow },    { Op58 },        { Op59Slow },
	{ Op5ASlow },    { Op5B },        { Op5CSlow },    { Op5DSlow },    { Op5ESlow },
	{ Op5FSlow },    { Op60Slow },    { Op61Slow },    { Op62Slow },    { Op63Slow },
	{ Op64Slow },    { Op65Slow },    { Op66Slow },    { Op67Slow },    { Op68Slow },
	{ Op69Slow },    { Op6ASlow },    { Op6BSlow },    { Op6CSlow },    { Op6DSlow },
	{ Op6ESlow },    { Op6FSlow },    { Op70Slow },    { Op71Slow },    { Op72Slow },
	{ Op73Slow },    { Op74Slow },    { Op75Slow },    { Op76Slow },    { Op77Slow },
	{ Op78 },        { Op79Slow },    { Op7ASlow },    { Op7B },        { Op7CSlow },
	{ Op7DSlow },    { Op7ESlow },    { Op7FSlow },    { Op80Slow },    { Op81Slow },
	{ Op82Slow },    { Op83Slow },    { Op84Slow },    { Op85Slow },    { Op86Slow },
	{ Op87Slow },    { Op88Slow },    { Op89Slow },    { Op8ASlow },    { Op8BSlow },
	{ Op8CSlow },    { Op8DSlow },    { Op8ESlow },    { Op8FSlow },    { Op90Slow },
	{ Op91Slow },    { Op92Slow },    { Op93Slow },    { Op94Slow },    { Op95Slow },
	{ Op96Slow },    { Op97Slow },    { Op98Slow },    { Op99Slow },    { Op9A },
	{ Op9BSlow },    { Op9CSlow },    { Op9DSlow },    { Op9ESlow },    { Op9FSlow },
	{ OpA0Slow },    { OpA1Slow },    { OpA2Slow },    { OpA3Slow },    { OpA4Slow },
	{ OpA5Slow },    { OpA6Slow },    { OpA7Slow },    { OpA8Slow },    { OpA9Slow },
	{ OpAASlow },    { OpABSlow },    { OpACSlow },    { OpADSlow },    { OpAESlow },
	{ OpAFSlow },    { OpB0Slow },    { OpB1Slow },    { OpB2Slow },    { OpB3Slow },
	{ OpB4Slow },    { OpB5Slow },    { OpB6Slow },    { OpB7Slow },    { OpB8 },
	{ OpB9Slow },    { OpBASlow },    { OpBBSlow },    { OpBCSlow },    { OpBDSlow },
	{ OpBESlow },    { OpBFSlow },    { OpC0Slow },    { OpC1Slow },    { OpC2Slow },
	{ OpC3Slow },    { OpC4Slow },    { OpC5Slow },    { OpC6Slow },    { OpC7Slow },
	{ OpC8Slow },    { OpC9Slow },    { OpCASlow },    { OpCB },        { OpCCSlow },
	{ OpCDSlow },    { OpCESlow },    { OpCFSlow },    { OpD0Slow },    { OpD1Slow },
	{ OpD2Slow },    { OpD3Slow },    { OpD4Slow },    { OpD5Slow },    { OpD6Slow },
	{ OpD7Slow },    { OpD8 },        { OpD9Slow },    { OpDASlow },    { OpDB },
	{ OpDCSlow },    { OpDDSlow },    { OpDESlow },    { OpDFSlow },    { OpE0Slow },
	{ OpE1Slow },    { OpE2Slow },    { OpE3Slow },    { OpE4Slow },    { OpE5Slow },
	{ OpE6Slow },    { OpE7Slow },    { OpE8Slow },    { OpE9Slow },    { OpEA },
	{ OpEB },        { OpECSlow },    { OpEDSlow },    { OpEESlow },    { OpEFSlow },
	{ OpF0Slow },    { OpF1Slow },    { OpF2Slow },    { OpF3Slow },    { OpF4Slow },
	{ OpF5Slow },    { OpF6Slow },    { OpF7Slow },    { OpF8 },        { OpF9Slow },
	{ OpFASlow },    { OpFB },        { OpFCSlow },    { OpFDSlow },    { OpFESlow },
	{ OpFFSlow }
};
