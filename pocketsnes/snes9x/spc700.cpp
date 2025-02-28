/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman
 
  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se) 

 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/
#include "snes9x.h"
#include "spc700.h"
#include "memmap.h"
#include "display.h"
#include "cpuexec.h"
#include "apu.h"
#include "apumem.h"

int8   Int8 = 0;
int16  Int16 = 0;
int32  Int32 = 0;
uint8  W1;
uint8  W2;
uint8  Work8 = 0;
uint16 Work16 = 0;
uint32 Work32 = 0;

#define OP1 IAPU.PC[1]
#define OP2 IAPU.PC[2]

#define APUShutdown() \
    if (Settings.Shutdown && (IAPU.PC == IAPU.WaitAddress1 || IAPU.PC == IAPU.WaitAddress2)) \
    { \
       if (IAPU.WaitCounter == 0) \
       { \
          if (!ICPU.CPUExecuting) \
             APU.Cycles = CPU.Cycles = CPU.NextEvent; \
          else \
             IAPU.APUExecuting = false; \
       } \
       else if (IAPU.WaitCounter >= 2) \
          IAPU.WaitCounter = 1; \
       else \
          IAPU.WaitCounter--; \
    }

#define APUSetZN8(b) \
    IAPU._Zero = (b)

#define APUSetZN16(w) \
    IAPU._Zero = ((w) != 0) | ((w) >> 8)

#define TCALL(n) \
{\
    PushW (IAPU.PC - IAPU.RAM + 1); \
    IAPU.PC = IAPU.RAM + S9xAPUGetByte(0xffc0 + ((15 - n) << 1)) + \
       (S9xAPUGetByte(0xffc1 + ((15 - n) << 1)) << 8); \
}

#define SBC(a, b) \
    Int16 = (int16) (a) - (int16) (b) + (int16) (APUCheckCarry ()) - 1; \
    IAPU._Carry = Int16 >= 0; \
    if ((((a) ^ (b)) & 0x80) && (((a) ^ (uint8) Int16) & 0x80)) \
       APUSetOverflow (); \
    else \
       APUClearOverflow (); \
    APUSetHalfCarry (); \
    if(((a) ^ (b) ^ (uint8) Int16) & 0x10) \
       APUClearHalfCarry (); \
    (a) = (uint8) Int16; \
    APUSetZN8 ((uint8) Int16)

#define ADC(a,b) \
    Work16 = (a) + (b) + APUCheckCarry(); \
    IAPU._Carry = Work16 >= 0x100; \
    if (~((a) ^ (b)) & ((b) ^ (uint8) Work16) & 0x80) \
       APUSetOverflow (); \
    else \
       APUClearOverflow (); \
    APUClearHalfCarry (); \
    if(((a) ^ (b) ^ (uint8) Work16) & 0x10) \
       APUSetHalfCarry (); \
    (a) = (uint8) Work16; \
    APUSetZN8 ((uint8) Work16)

#define CMP(a,b) \
    Int16 = (int16) (a) - (int16) (b); \
    IAPU._Carry = Int16 >= 0; \
    APUSetZN8((uint8) Int16)

#define ASL(b) \
    IAPU._Carry = ((b) & 0x80) != 0; \
    (b) <<= 1; \
    APUSetZN8 (b)

#define LSR(b) \
    IAPU._Carry = (b) & 1; \
    (b) >>= 1; \
    APUSetZN8 (b)

#define ROL(b) \
    Work16 = ((b) << 1) | APUCheckCarry (); \
    IAPU._Carry = Work16 >= 0x100; \
    (b) = (uint8) Work16; \
    APUSetZN8 (b)

#define ROR(b) \
    Work16 = (b) | ((uint16) APUCheckCarry () << 8); \
    IAPU._Carry = (uint8) Work16 & 1; \
    Work16 >>= 1; \
    (b) = (uint8) Work16; \
    APUSetZN8 (b)

#define Push(b) \
    IAPU.RAM[0x100 + IAPU.Registers.S] = b; \
    IAPU.Registers.S--

#define Pop(b) \
    IAPU.Registers.S++; \
    (b) = IAPU.RAM[0x100 + IAPU.Registers.S]

#ifdef FAST_LSB_WORD_ACCESS
#define PushW(w) \
    if (IAPU.Registers.S == 0) \
    {\
       IAPU.RAM[0x1ff] = (w); \
       IAPU.RAM[0x100] = ((w) >> 8); \
    } \
    else \
       *(uint16 *) (IAPU.RAM + 0xff + IAPU.Registers.S) = w; \
    IAPU.Registers.S -= 2

#define PopW(w) \
    IAPU.Registers.S += 2; \
    if (IAPU.Registers.S == 0) \
       (w) = IAPU.RAM[0x1ff] | (IAPU.RAM[0x100] << 8); \
    else \
       (w) = *(uint16 *) (IAPU.RAM + 0xff + IAPU.Registers.S)
#else
#define PushW(w) \
    IAPU.RAM[0xff + IAPU.Registers.S] = w; \
    IAPU.RAM[0x100 + IAPU.Registers.S] = ((w) >> 8); \
    IAPU.Registers.S -= 2

#define PopW(w) \
    IAPU.Registers.S += 2; \
    if(IAPU.Registers.S == 0) \
       (w) = IAPU.RAM[0x1ff] | (IAPU.RAM[0x100] << 8); \
    else \
       (w) = IAPU.RAM[0xff + IAPU.Registers.S] + (IAPU.RAM[0x100 + IAPU.Registers.S] << 8)
#endif

#define Relative() \
    Int8 = OP1; \
    Int16 = (int16) (IAPU.PC + 2 - IAPU.RAM) + Int8;

#define Relative2() \
    Int8 = OP2; \
    Int16 = (int16) (IAPU.PC + 3 - IAPU.RAM) + Int8;

#ifdef FAST_LSB_WORD_ACCESS
#define IndexedXIndirect() \
    IAPU.Address = *(uint16 *) (IAPU.DirectPage + ((OP1 + IAPU.Registers.X) & 0xff));

#define Absolute() \
    IAPU.Address = *(uint16 *) (IAPU.PC + 1);

#define AbsoluteX() \
    IAPU.Address = *(uint16 *) (IAPU.PC + 1) + IAPU.Registers.X;

#define AbsoluteY() \
    IAPU.Address = *(uint16 *) (IAPU.PC + 1) + IAPU.Registers.YA.B.Y;

#define MemBit() \
    IAPU.Address = *(uint16 *) (IAPU.PC + 1); \
    IAPU.Bit = (uint8)(IAPU.Address >> 13); \
    IAPU.Address &= 0x1fff;

#define IndirectIndexedY() \
    IAPU.Address = *(uint16 *) (IAPU.DirectPage + OP1) + IAPU.Registers.YA.B.Y;
#else
#define IndexedXIndirect() \
    IAPU.Address = IAPU.DirectPage[(OP1 + IAPU.Registers.X) & 0xff] + \
       (IAPU.DirectPage[(OP1 + IAPU.Registers.X + 1) & 0xff] << 8);

#define Absolute() \
    IAPU.Address = OP1 + (OP2 << 8);

#define AbsoluteX() \
    IAPU.Address = OP1 + (OP2 << 8) + IAPU.Registers.X;

#define AbsoluteY() \
    IAPU.Address = OP1 + (OP2 << 8) + IAPU.Registers.YA.B.Y;

#define MemBit() \
    IAPU.Address = OP1 + (OP2 << 8); \
    IAPU.Bit = (int8) (IAPU.Address >> 13); \
    IAPU.Address &= 0x1fff;

#define IndirectIndexedY() \
    IAPU.Address = IAPU.DirectPage[OP1] + \
       (IAPU.DirectPage[OP1 + 1] << 8) + \
       IAPU.Registers.YA.B.Y;
#endif

void Apu00(void) /* NOP */
{
   IAPU.PC++;
}

void Apu01(void)
{
   TCALL(0);
}

void Apu11(void)
{
   TCALL(1);
}

void Apu21(void)
{
   TCALL(2);
}

void Apu31(void)
{
   TCALL(3);
}

void Apu41(void)
{
   TCALL(4);
}

void Apu51(void)
{
   TCALL(5);
}

void Apu61(void)
{
   TCALL(6);
}

void Apu71(void)
{
   TCALL(7);
}

void Apu81(void)
{
   TCALL(8);
}

void Apu91(void)
{
   TCALL(9);
}

void ApuA1(void)
{
   TCALL(10);
}

void ApuB1(void)
{
   TCALL(11);
}

void ApuC1(void)
{
   TCALL(12);
}

void ApuD1(void)
{
   TCALL(13);
}

void ApuE1(void)
{
   TCALL(14);
}

void ApuF1(void)
{
   TCALL(15);
}

void Apu3F(void) /* CALL absolute */
{
   Absolute();
   /* 0xB6f for Star Fox 2 */
   PushW(IAPU.PC + 3 - IAPU.RAM);
   IAPU.PC = IAPU.RAM + IAPU.Address;
}

void Apu4F(void) /* PCALL $XX */
{
   Work8 = OP1;
   PushW(IAPU.PC + 2 - IAPU.RAM);
   IAPU.PC = IAPU.RAM + 0xff00 + Work8;
}

#define SET(b) \
S9xAPUSetByteZ ((uint8) (S9xAPUGetByteZ (OP1 ) | (1 << (b))), OP1); \
IAPU.PC += 2

void Apu02(void)
{
   SET(0);
}

void Apu22(void)
{
   SET(1);
}

void Apu42(void)
{
   SET(2);
}

void Apu62(void)
{
   SET(3);
}

void Apu82(void)
{
   SET(4);
}

void ApuA2(void)
{
   SET(5);
}

void ApuC2(void)
{
   SET(6);
}

void ApuE2(void)
{
   SET(7);
}

#define CLR(b) \
S9xAPUSetByteZ ((uint8) (S9xAPUGetByteZ (OP1) & ~(1 << (b))), OP1); \
IAPU.PC += 2;

void Apu12(void)
{
   CLR(0);
}

void Apu32(void)
{
   CLR(1);
}

void Apu52(void)
{
   CLR(2);
}

void Apu72(void)
{
   CLR(3);
}

void Apu92(void)
{
   CLR(4);
}

void ApuB2(void)
{
   CLR(5);
}

void ApuD2(void)
{
   CLR(6);
}

void ApuF2(void)
{
   CLR(7);
}

#define BBS(b) \
Work8 = OP1; \
Relative2 (); \
if (S9xAPUGetByteZ (Work8) & (1 << (b))) \
{ \
    IAPU.PC = IAPU.RAM + (uint16) Int16; \
    APU.Cycles += IAPU.TwoCycles; \
} \
else \
    IAPU.PC += 3

void Apu03(void)
{
   BBS(0);
}

void Apu23(void)
{
   BBS(1);
}

void Apu43(void)
{
   BBS(2);
}

void Apu63(void)
{
   BBS(3);
}

void Apu83(void)
{
   BBS(4);
}

void ApuA3(void)
{
   BBS(5);
}

void ApuC3(void)
{
   BBS(6);
}

void ApuE3(void)
{
   BBS(7);
}

#define BBC(b) \
Work8 = OP1; \
Relative2 (); \
if (!(S9xAPUGetByteZ (Work8) & (1 << (b)))) \
{ \
    IAPU.PC = IAPU.RAM + (uint16) Int16; \
    APU.Cycles += IAPU.TwoCycles; \
} \
else \
    IAPU.PC += 3

void Apu13(void)
{
   BBC(0);
}

void Apu33(void)
{
   BBC(1);
}

void Apu53(void)
{
   BBC(2);
}

void Apu73(void)
{
   BBC(3);
}

void Apu93(void)
{
   BBC(4);
}

void ApuB3(void)
{
   BBC(5);
}

void ApuD3(void)
{
   BBC(6);
}

void ApuF3(void)
{
   BBC(7);
}

void Apu04(void)
{
   /* OR A,dp */
   IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu05(void)
{
   /* OR A,abs */
   Absolute();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu06(void)
{
   /* OR A,(X) */
   IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu07(void)
{
   /* OR A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu08(void)
{
   /* OR A,#00 */
   IAPU.Registers.YA.B.A |= OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu09(void)
{
   /* OR dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   Work8 |= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu14(void)
{
   /* OR A,dp+X */
   IAPU.Registers.YA.B.A |= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu15(void)
{
   /* OR A,abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu16(void)
{
   /* OR A,abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu17(void)
{
   /* OR A,(dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A |= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu18(void)
{
   /* OR dp,#00 */
   Work8 = OP1;
   Work8 |= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu19(void)
{
   /* OR (X),(Y) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X) | S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   APUSetZN8(Work8);
   S9xAPUSetByteZ(Work8, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu0A(void)
{
   /* OR1 C,membit */
   MemBit();
   if (!APUCheckCarry())
      if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
         APUSetCarry();
   IAPU.PC += 3;
}

void Apu2A(void)
{
   /* OR1 C,not membit */
   MemBit();
   if (!APUCheckCarry())
      if (!(S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
         APUSetCarry();
   IAPU.PC += 3;
}

void Apu4A(void)
{
   /* AND1 C,membit */
   MemBit();
   if (APUCheckCarry())
      if (!(S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
         APUClearCarry();
   IAPU.PC += 3;
}

void Apu6A(void)
{
   /* AND1 C, not membit */
   MemBit();
   if (APUCheckCarry())
      if ((S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit)))
         APUClearCarry();
   IAPU.PC += 3;
}

void Apu8A(void)
{
   /* EOR1 C, membit */
   MemBit();
   if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
   {
      if (APUCheckCarry())
         APUClearCarry();
      else
         APUSetCarry();
   }
   IAPU.PC += 3;
}

void ApuAA(void)
{
   /* MOV1 C,membit */
   MemBit();
   if (S9xAPUGetByte(IAPU.Address) & (1 << IAPU.Bit))
      APUSetCarry();
   else
      APUClearCarry();
   IAPU.PC += 3;
}

void ApuCA(void)
{
   /* MOV1 membit,C */
   MemBit();
   if (APUCheckCarry())
      S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) | (1 << IAPU.Bit), IAPU.Address);
   else
      S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) & ~(1 << IAPU.Bit), IAPU.Address);
   IAPU.PC += 3;
}

void ApuEA(void)
{
   /* NOT1 membit */
   MemBit();
   S9xAPUSetByte(S9xAPUGetByte(IAPU.Address) ^ (1 << IAPU.Bit), IAPU.Address);
   IAPU.PC += 3;
}

void Apu0B(void)
{
   /* ASL dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ASL(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu0C(void)
{
   /* ASL abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ASL(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu1B(void)
{
   /* ASL dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ASL(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu1C(void)
{
   /* ASL A */
   ASL(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu0D(void)
{
   /* PUSH PSW */
   S9xAPUPackStatus();
   Push(IAPU.Registers.P);
   IAPU.PC++;
}

void Apu2D(void)
{
   /* PUSH A */
   Push(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu4D(void)
{
   /* PUSH X */
   Push(IAPU.Registers.X);
   IAPU.PC++;
}

void Apu6D(void)
{
   /* PUSH Y */
   Push(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void Apu8E(void)
{
   /* POP PSW */
   Pop(IAPU.Registers.P);
   S9xAPUUnpackStatus();
   if (APUCheckDirectPage())
      IAPU.DirectPage = IAPU.RAM + 0x100;
   else
      IAPU.DirectPage = IAPU.RAM;
   IAPU.PC++;
}

void ApuAE(void)
{
   /* POP A */
   Pop(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuCE(void)
{
   /* POP X */
   Pop(IAPU.Registers.X);
   IAPU.PC++;
}

void ApuEE(void)
{
   /* POP Y */
   Pop(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void Apu0E(void)
{
   /* TSET1 abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   S9xAPUSetByte(Work8 | IAPU.Registers.YA.B.A, IAPU.Address);
   Work8 = IAPU.Registers.YA.B.A - Work8;
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu4E(void)
{
   /* TCLR1 abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   S9xAPUSetByte(Work8 & ~IAPU.Registers.YA.B.A, IAPU.Address);
   Work8 = IAPU.Registers.YA.B.A - Work8;
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu0F(void)
{
   /* BRK */
   PushW(IAPU.PC + 1 - IAPU.RAM);
   S9xAPUPackStatus();
   Push(IAPU.Registers.P);
   APUSetBreak();
   APUClearInterrupt();
   IAPU.PC = IAPU.RAM + S9xAPUGetByte(0xffde) + (S9xAPUGetByte(0xffdf) << 8);
}

void ApuEF(void)
{
   /* SLEEP */
   APU.TimerEnabled[0] = APU.TimerEnabled[1] = APU.TimerEnabled[2] = false;
   IAPU.APUExecuting = false;
}

void ApuFF(void)
{
   /* STOP */
   APU.TimerEnabled[0] = APU.TimerEnabled[1] = APU.TimerEnabled[2] = false;
   IAPU.APUExecuting = false;
   Settings.APUEnabled = false; /* re-enabled on next APU reset */
}

void Apu10(void)
{
   /* BPL */
   Relative();
   if (!APUCheckNegative())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void Apu30(void)
{
   /* BMI */
   Relative();
   if (APUCheckNegative())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void Apu90(void)
{
   /* BCC */
   Relative();
   if (!APUCheckCarry())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void ApuB0(void)
{
   /* BCS */
   Relative();
   if (APUCheckCarry())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void ApuD0(void)
{
   /* BNE */
   Relative();
   if (!APUCheckZero())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void ApuF0(void)
{
   /* BEQ */
   Relative();
   if (APUCheckZero())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 2;
}

void Apu50(void)
{
   /* BVC */
   Relative();
   if (!APUCheckOverflow())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 2;
}

void Apu70(void)
{
   /* BVS */
   Relative();
   if (APUCheckOverflow())
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 2;
}

void Apu2F(void)
{
   /* BRA */
   Relative();
   IAPU.PC = IAPU.RAM + (uint16) Int16;
}

void Apu80(void)
{
   /* SETC */
   APUSetCarry();
   IAPU.PC++;
}

void ApuED(void)
{
   /* NOTC */
   IAPU._Carry ^= 1;
   IAPU.PC++;
}

void Apu40(void)
{
   /* SETP */
   APUSetDirectPage();
   IAPU.DirectPage = IAPU.RAM + 0x100;
   IAPU.PC++;
}

void Apu1A(void)
{
   /* DECW dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8) - 1;
   S9xAPUSetByteZ((uint8) Work16, OP1);
   S9xAPUSetByteZ(Work16 >> 8, OP1 + 1);
   APUSetZN16(Work16);
   IAPU.PC += 2;
}

void Apu5A(void)
{
   /* CMPW YA,dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8);
   Int32 = (int32) IAPU.Registers.YA.W - (int32) Work16;
   IAPU._Carry = Int32 >= 0;
   APUSetZN16((uint16) Int32);
   IAPU.PC += 2;
}

void Apu3A(void)
{
   /* INCW dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8) + 1;
   S9xAPUSetByteZ((uint8) Work16, OP1);
   S9xAPUSetByteZ(Work16 >> 8, OP1 + 1);
   APUSetZN16(Work16);
   IAPU.PC += 2;
}

void Apu7A(void)
{
   /* ADDW YA,dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8);
   Work32 = (uint32) IAPU.Registers.YA.W + Work16;
   IAPU._Carry = Work32 >= 0x10000;
   if (~(IAPU.Registers.YA.W ^ Work16) & (Work16 ^ (uint16) Work32) & 0x8000)
      APUSetOverflow();
   else
      APUClearOverflow();
   APUClearHalfCarry();
   if ((IAPU.Registers.YA.W ^ Work16 ^ (uint16) Work32) & 0x1000)
      APUSetHalfCarry();
   IAPU.Registers.YA.W = (uint16) Work32;
   APUSetZN16(IAPU.Registers.YA.W);
   IAPU.PC += 2;
}

void Apu9A(void)
{
   /* SUBW YA,dp */
   Work16 = S9xAPUGetByteZ(OP1) + (S9xAPUGetByteZ(OP1 + 1) << 8);
   Int32 = (int32) IAPU.Registers.YA.W - (int32) Work16;
   APUClearHalfCarry();
   IAPU._Carry = Int32 >= 0;
   if (((IAPU.Registers.YA.W ^ Work16) & 0x8000) && ((IAPU.Registers.YA.W ^ (uint16) Int32) & 0x8000))
      APUSetOverflow();
   else
      APUClearOverflow();
   APUSetHalfCarry();
   if ((IAPU.Registers.YA.W ^ Work16 ^ (uint16) Int32) & 0x1000)
      APUClearHalfCarry();
   IAPU.Registers.YA.W = (uint16) Int32;
   APUSetZN16(IAPU.Registers.YA.W);
   IAPU.PC += 2;
}

void ApuBA(void)
{
   /* MOVW YA,dp */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1);
   IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1 + 1);
   APUSetZN16(IAPU.Registers.YA.W);
   IAPU.PC += 2;
}

void ApuDA(void)
{
   /* MOVW dp,YA */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1);
   S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1 + 1);
   IAPU.PC += 2;
}

void Apu64(void)
{
   /* CMP A,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu65(void)
{
   /* CMP A,abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu66(void)
{
   /* CMP A,(X) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC++;
}

void Apu67(void)
{
   /* CMP A,(dp+X) */
   IndexedXIndirect();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu68(void)
{
   /* CMP A,#00 */
   Work8 = OP1;
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu69(void)
{
   /* CMP dp(dest), dp(src) */
   W1 = S9xAPUGetByteZ(OP1);
   Work8 = S9xAPUGetByteZ(OP2);
   CMP(Work8, W1);
   IAPU.PC += 3;
}

void Apu74(void)
{
   /* CMP A, dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu75(void)
{
   /* CMP A,abs+X */
   AbsoluteX();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu76(void)
{
   /* CMP A, abs+Y */
   AbsoluteY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu77(void)
{
   /* CMP A,(dp)+Y */
   IndirectIndexedY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu78(void)
{
   /* CMP dp,#00 */
   Work8 = OP1;
   W1 = S9xAPUGetByteZ(OP2);
   CMP(W1, Work8);
   IAPU.PC += 3;
}

void Apu79(void)
{
   /* CMP (X),(Y) */
   W1 = S9xAPUGetByteZ(IAPU.Registers.X);
   Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   CMP(W1, Work8);
   IAPU.PC++;
}

void Apu1E(void)
{
   /* CMP X,abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.X, Work8);
   IAPU.PC += 3;
}

void Apu3E(void)
{
   /* CMP X,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   CMP(IAPU.Registers.X, Work8);
   IAPU.PC += 2;
}

void ApuC8(void)
{
   /* CMP X,#00 */
   CMP(IAPU.Registers.X, OP1);
   IAPU.PC += 2;
}

void Apu5E(void)
{
   /* CMP Y,abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   CMP(IAPU.Registers.YA.B.Y, Work8);
   IAPU.PC += 3;
}

void Apu7E(void)
{
   /* CMP Y,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   CMP(IAPU.Registers.YA.B.Y, Work8);
   IAPU.PC += 2;
}

void ApuAD(void)
{
   /* CMP Y,#00 */
   Work8 = OP1;
   CMP(IAPU.Registers.YA.B.Y, Work8);
   IAPU.PC += 2;
}

void Apu1F(void)
{
   /* JMP (abs+X) */
   Absolute();
   IAPU.PC = IAPU.RAM + S9xAPUGetByte(IAPU.Address + IAPU.Registers.X) + (S9xAPUGetByte(IAPU.Address + IAPU.Registers.X + 1) << 8);
}

void Apu5F(void)
{
   /* JMP abs */
   Absolute();
   IAPU.PC = IAPU.RAM + IAPU.Address;
}

void Apu20(void)
{
   /* CLRP */
   APUClearDirectPage();
   IAPU.DirectPage = IAPU.RAM;
   IAPU.PC++;
}

void Apu60(void)
{
   /* CLRC */
   APUClearCarry();
   IAPU.PC++;
}

void ApuE0(void)
{
   /* CLRV */
   APUClearHalfCarry();
   APUClearOverflow();
   IAPU.PC++;
}

void Apu24(void)
{
   /* AND A,dp */
   IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu25(void)
{
   /* AND A,abs */
   Absolute();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu26(void)
{
   /* AND A,(X) */
   IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu27(void)
{
   /* AND A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu28(void)
{
   /* AND A,#00 */
   IAPU.Registers.YA.B.A &= OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu29(void)
{
   /* AND dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   Work8 &= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu34(void)
{
   /* AND A,dp+X */
   IAPU.Registers.YA.B.A &= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu35(void)
{
   /* AND A,abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu36(void)
{
   /* AND A,abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu37(void)
{
   /* AND A,(dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A &= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu38(void)
{
   /* AND dp,#00 */
   Work8 = OP1;
   Work8 &= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu39(void)
{
   /* AND (X),(Y) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X) & S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   APUSetZN8(Work8);
   S9xAPUSetByteZ(Work8, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu2B(void)
{
   /* ROL dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ROL(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu2C(void)
{
   /* ROL abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ROL(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu3B(void)
{
   /* ROL dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ROL(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu3C(void)
{
   /* ROL A */
   ROL(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu2E(void)
{
   /* CBNE dp,rel */
   Work8 = OP1;
   Relative2();

   if (S9xAPUGetByteZ(Work8) != IAPU.Registers.YA.B.A)
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 3;
}

void ApuDE(void)
{
   /* CBNE dp+X,rel */
   Work8 = OP1 + IAPU.Registers.X;
   Relative2();

   if (S9xAPUGetByteZ(Work8) != IAPU.Registers.YA.B.A)
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
      APUShutdown();
   }
   else
      IAPU.PC += 3;
}

void Apu3D(void)
{
   /* INC X */
   IAPU.Registers.X++;
   APUSetZN8(IAPU.Registers.X);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void ApuFC(void)
{
   /* INC Y */
   IAPU.Registers.YA.B.Y++;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void Apu1D(void)
{
   /* DEC X */
   IAPU.Registers.X--;
   APUSetZN8(IAPU.Registers.X);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void ApuDC(void)
{
   /* DEC Y */
   IAPU.Registers.YA.B.Y--;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void ApuAB(void)
{
   /* INC dp */
   Work8 = S9xAPUGetByteZ(OP1) + 1;
   S9xAPUSetByteZ(Work8, OP1);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void ApuAC(void)
{
   /* INC abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address) + 1;
   S9xAPUSetByte(Work8, IAPU.Address);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 3;
}

void ApuBB(void)
{
   /* INC dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X) + 1;
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void ApuBC(void)
{
   /* INC A */
   IAPU.Registers.YA.B.A++;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void Apu8B(void)
{
   /* DEC dp */
   Work8 = S9xAPUGetByteZ(OP1) - 1;
   S9xAPUSetByteZ(Work8, OP1);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void Apu8C(void)
{
   /* DEC abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address) - 1;
   S9xAPUSetByte(Work8, IAPU.Address);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 3;
}

void Apu9B(void)
{
   /* DEC dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X) - 1;
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   APUSetZN8(Work8);
   IAPU.WaitCounter++;
   IAPU.PC += 2;
}

void Apu9C(void)
{
   /* DEC A */
   IAPU.Registers.YA.B.A--;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.WaitCounter++;
   IAPU.PC++;
}

void Apu44(void)
{
   /* EOR A,dp */
   IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu45(void)
{
   /* EOR A,abs */
   Absolute();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu46(void)
{
   /* EOR A,(X) */
   IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu47(void)
{
   /* EOR A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu48(void)
{
   /* EOR A,#00 */
   IAPU.Registers.YA.B.A ^= OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu49(void)
{
   /* EOR dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   Work8 ^= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu54(void)
{
   /* EOR A,dp+X */
   IAPU.Registers.YA.B.A ^= S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu55(void)
{
   /* EOR A,abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu56(void)
{
   /* EOR A,abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void Apu57(void)
{
   /* EOR A,(dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A ^= S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void Apu58(void)
{
   /* EOR dp,#00 */
   Work8 = OP1;
   Work8 ^= S9xAPUGetByteZ(OP2);
   S9xAPUSetByteZ(Work8, OP2);
   APUSetZN8(Work8);
   IAPU.PC += 3;
}

void Apu59(void)
{
   /* EOR (X),(Y) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X) ^ S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   APUSetZN8(Work8);
   S9xAPUSetByteZ(Work8, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu4B(void)
{
   /* LSR dp */
   Work8 = S9xAPUGetByteZ(OP1);
   LSR(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu4C(void)
{
   /* LSR abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   LSR(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu5B(void)
{
   /* LSR dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   LSR(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu5C(void)
{
   /* LSR A */
   LSR(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu7D(void)
{
   /* MOV A,X */
   IAPU.Registers.YA.B.A = IAPU.Registers.X;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuDD(void)
{
   /* MOV A,Y */
   IAPU.Registers.YA.B.A = IAPU.Registers.YA.B.Y;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu5D(void)
{
   /* MOV X,A */
   IAPU.Registers.X = IAPU.Registers.YA.B.A;
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC++;
}

void ApuFD(void)
{
   /* MOV Y,A */
   IAPU.Registers.YA.B.Y = IAPU.Registers.YA.B.A;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void Apu9D(void)
{
   /* MOV X,SP */
   IAPU.Registers.X = IAPU.Registers.S;
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC++;
}

void ApuBD(void)
{
   /* MOV SP,X */
   IAPU.Registers.S = IAPU.Registers.X;
   IAPU.PC++;
}

void Apu6B(void)
{
   /* ROR dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ROR(Work8);
   S9xAPUSetByteZ(Work8, OP1);
   IAPU.PC += 2;
}

void Apu6C(void)
{
   /* ROR abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ROR(Work8);
   S9xAPUSetByte(Work8, IAPU.Address);
   IAPU.PC += 3;
}

void Apu7B(void)
{
   /* ROR dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ROR(Work8);
   S9xAPUSetByteZ(Work8, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void Apu7C(void)
{
   /* ROR A */
   ROR(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu6E(void)
{
   /* DBNZ dp,rel */
   Work8 = OP1;
   Relative2();
   W1 = S9xAPUGetByteZ(Work8) - 1;
   S9xAPUSetByteZ(W1, Work8);
   if (W1 != 0)
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 3;
}

void ApuFE(void)
{
   /* DBNZ Y,rel */
   Relative();
   IAPU.Registers.YA.B.Y--;
   if (IAPU.Registers.YA.B.Y != 0)
   {
      IAPU.PC = IAPU.RAM + (uint16) Int16;
      APU.Cycles += IAPU.TwoCycles;
   }
   else
      IAPU.PC += 2;
}

void Apu6F(void)
{
   /* RET */
   PopW(IAPU.Registers.PC);
   IAPU.PC = IAPU.RAM + IAPU.Registers.PC;
}

void Apu7F(void)
{
   /* RETI */
   Pop(IAPU.Registers.P);
   S9xAPUUnpackStatus();
   PopW(IAPU.Registers.PC);
   IAPU.PC = IAPU.RAM + IAPU.Registers.PC;
}

void Apu84(void)
{
   /* ADC A,dp */
   Work8 = S9xAPUGetByteZ(OP1);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu85(void)
{
   /* ADC A, abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu86(void)
{
   /* ADC A,(X) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC++;
}

void Apu87(void)
{
   /* ADC A,(dp+X) */
   IndexedXIndirect();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu88(void)
{
   /* ADC A,#00 */
   Work8 = OP1;
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu89(void)
{
   /* ADC dp(dest),dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   W1 = S9xAPUGetByteZ(OP2);
   ADC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void Apu94(void)
{
   /* ADC A,dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu95(void)
{
   /* ADC A, abs+X */
   AbsoluteX();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu96(void)
{
   /* ADC A, abs+Y */
   AbsoluteY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void Apu97(void)
{
   /* ADC A, (dp)+Y */
   IndirectIndexedY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   ADC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void Apu98(void)
{
   /* ADC dp,#00 */
   Work8 = OP1;
   W1 = S9xAPUGetByteZ(OP2);
   ADC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void Apu99(void)
{
   /* ADC (X),(Y) */
   W1 = S9xAPUGetByteZ(IAPU.Registers.X);
   Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   ADC(W1, Work8);
   S9xAPUSetByteZ(W1, IAPU.Registers.X);
   IAPU.PC++;
}

void Apu8D(void)
{
   /* MOV Y,#00 */
   IAPU.Registers.YA.B.Y = OP1;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void Apu8F(void)
{
   /* MOV dp,#00 */
   Work8 = OP1;
   S9xAPUSetByteZ(Work8, OP2);
   IAPU.PC += 3;
}

void Apu9E(void)
{
   uint32 i;
   uint32 yva;
   uint32 x;

   /* DIV YA,X */
   if ((IAPU.Registers.X & 0x0f) <= (IAPU.Registers.YA.B.Y & 0x0f))
      APUSetHalfCarry();
   else
      APUClearHalfCarry();

   yva = IAPU.Registers.YA.W;
   x   = IAPU.Registers.X << 9;

   for (i = 0 ; i < 9 ; ++i)
   {
      yva <<= 1;
      if (yva & 0x20000)
         yva = (yva & 0x1ffff) | 1;
      if (yva >= x)
         yva ^= 1;
      if (yva & 1)
         yva = (yva - x) & 0x1ffff;
   }

   if (yva & 0x100)
       APUSetOverflow();
   else
       APUClearOverflow();

   IAPU.Registers.YA.B.Y = (yva >> 9) & 0xff;
   IAPU.Registers.YA.B.A = yva & 0xff;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void Apu9F(void)
{
   /* XCN A */
   IAPU.Registers.YA.B.A = (IAPU.Registers.YA.B.A >> 4) | (IAPU.Registers.YA.B.A << 4);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuA4(void)
{
   /* SBC A, dp */
   Work8 = S9xAPUGetByteZ(OP1);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuA5(void)
{
   /* SBC A, abs */
   Absolute();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void ApuA6(void)
{
   /* SBC A, (X) */
   Work8 = S9xAPUGetByteZ(IAPU.Registers.X);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC++;
}

void ApuA7(void)
{
   /* SBC A,(dp+X) */
   IndexedXIndirect();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuA8(void)
{
   /* SBC A,#00 */
   Work8 = OP1;
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuA9(void)
{
   /* SBC dp(dest), dp(src) */
   Work8 = S9xAPUGetByteZ(OP1);
   W1 = S9xAPUGetByteZ(OP2);
   SBC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void ApuB4(void)
{
   /* SBC A, dp+X */
   Work8 = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuB5(void)
{
   /* SBC A,abs+X */
   AbsoluteX();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void ApuB6(void)
{
   /* SBC A,abs+Y */
   AbsoluteY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 3;
}

void ApuB7(void)
{
   /* SBC A,(dp)+Y */
   IndirectIndexedY();
   Work8 = S9xAPUGetByte(IAPU.Address);
   SBC(IAPU.Registers.YA.B.A, Work8);
   IAPU.PC += 2;
}

void ApuB8(void)
{
   /* SBC dp,#00 */
   Work8 = OP1;
   W1 = S9xAPUGetByteZ(OP2);
   SBC(W1, Work8);
   S9xAPUSetByteZ(W1, OP2);
   IAPU.PC += 3;
}

void ApuB9(void)
{
   /* SBC (X),(Y) */
   W1 = S9xAPUGetByteZ(IAPU.Registers.X);
   Work8 = S9xAPUGetByteZ(IAPU.Registers.YA.B.Y);
   SBC(W1, Work8);
   S9xAPUSetByteZ(W1, IAPU.Registers.X);
   IAPU.PC++;
}

void ApuAF(void)
{
   /* MOV (X)+, A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, IAPU.Registers.X++);
   IAPU.PC++;
}

void ApuBE(void)
{
   /* DAS */
   if (IAPU.Registers.YA.B.A > 0x99 || !IAPU._Carry)
   {
      IAPU.Registers.YA.B.A -= 0x60;
      APUClearCarry();
   }
   else
      APUSetCarry();

   if ((IAPU.Registers.YA.B.A & 0x0f) > 9 || !APUCheckHalfCarry())
      IAPU.Registers.YA.B.A -= 6;

   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuBF(void)
{
   /* MOV A,(X)+ */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(IAPU.Registers.X++);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuC0(void)
{
   /* DI */
   APUClearInterrupt();
   IAPU.PC++;
}

void ApuA0(void)
{
   /* EI */
   APUSetInterrupt();
   IAPU.PC++;
}

void ApuC4(void)
{
   /* MOV dp,A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1);
   IAPU.PC += 2;
}

void ApuC5(void)
{
   /* MOV abs,A */
   Absolute();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 3;
}

void ApuC6(void)
{
   /* MOV (X), A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, IAPU.Registers.X);
   IAPU.PC++;
}

void ApuC7(void)
{
   /* MOV (dp+X),A */
   IndexedXIndirect();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 2;
}

void ApuC9(void)
{
   /* MOV abs,X */
   Absolute();
   S9xAPUSetByte(IAPU.Registers.X, IAPU.Address);
   IAPU.PC += 3;
}

void ApuCB(void)
{
   /* MOV dp,Y */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1);
   IAPU.PC += 2;
}

void ApuCC(void)
{
   /* MOV abs,Y */
   Absolute();
   S9xAPUSetByte(IAPU.Registers.YA.B.Y, IAPU.Address);
   IAPU.PC += 3;
}

void ApuCD(void)
{
   /* MOV X,#00 */
   IAPU.Registers.X = OP1;
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuCF(void)
{
   /* MUL YA */
   IAPU.Registers.YA.W = (uint16) IAPU.Registers.YA.B.A * IAPU.Registers.YA.B.Y;
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC++;
}

void ApuD4(void)
{
   /* MOV dp+X, A */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.A, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuD5(void)
{
   /* MOV abs+X,A */
   AbsoluteX();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 3;
}

void ApuD6(void)
{
   /* MOV abs+Y,A */
   AbsoluteY();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 3;
}

void ApuD7(void)
{
   /* MOV (dp)+Y,A */
   IndirectIndexedY();
   S9xAPUSetByte(IAPU.Registers.YA.B.A, IAPU.Address);
   IAPU.PC += 2;
}

void ApuD8(void)
{
   /* MOV dp,X */
   S9xAPUSetByteZ(IAPU.Registers.X, OP1);
   IAPU.PC += 2;
}

void ApuD9(void)
{
   /* MOV dp+Y,X */
   S9xAPUSetByteZ(IAPU.Registers.X, OP1 + IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void ApuDB(void)
{
   /* MOV dp+X,Y */
   S9xAPUSetByteZ(IAPU.Registers.YA.B.Y, OP1 + IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuDF(void)
{
   /* DAA */
   if (IAPU.Registers.YA.B.A > 0x99 || IAPU._Carry)
   {
      IAPU.Registers.YA.B.A += 0x60;
      APUSetCarry();
   }
   else
      APUClearCarry();

   if ((IAPU.Registers.YA.B.A & 0x0f) > 9 || APUCheckHalfCarry())
      IAPU.Registers.YA.B.A += 6;

   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuE4(void)
{
   /* MOV A, dp */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuE5(void)
{
   /* MOV A,abs */
   Absolute();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void ApuE6(void)
{
   /* MOV A,(X) */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC++;
}

void ApuE7(void)
{
   /* MOV A,(dp+X) */
   IndexedXIndirect();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuE8(void)
{
   /* MOV A,#00 */
   IAPU.Registers.YA.B.A = OP1;
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuE9(void)
{
   /* MOV X, abs */
   Absolute();
   IAPU.Registers.X = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 3;
}

void ApuEB(void)
{
   /* MOV Y,dp */
   IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void ApuEC(void)
{
   /* MOV Y,abs */
   Absolute();
   IAPU.Registers.YA.B.Y = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 3;
}

void ApuF4(void)
{
   /* MOV A, dp+X */
   IAPU.Registers.YA.B.A = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuF5(void)
{
   /* MOV A, abs+X */
   AbsoluteX();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void ApuF6(void)
{
   /* MOV A, abs+Y */
   AbsoluteY();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 3;
}

void ApuF7(void)
{
   /* MOV A, (dp)+Y */
   IndirectIndexedY();
   IAPU.Registers.YA.B.A = S9xAPUGetByte(IAPU.Address);
   APUSetZN8(IAPU.Registers.YA.B.A);
   IAPU.PC += 2;
}

void ApuF8(void)
{
   /* MOV X,dp */
   IAPU.Registers.X = S9xAPUGetByteZ(OP1);
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuF9(void)
{
   /* MOV X,dp+Y */
   IAPU.Registers.X = S9xAPUGetByteZ(OP1 + IAPU.Registers.YA.B.Y);
   APUSetZN8(IAPU.Registers.X);
   IAPU.PC += 2;
}

void ApuFA(void)
{
   /* MOV dp(dest),dp(src) */
   S9xAPUSetByteZ(S9xAPUGetByteZ(OP1), OP2);
   IAPU.PC += 3;
}

void ApuFB(void)
{
   /* MOV Y,dp+X */
   IAPU.Registers.YA.B.Y = S9xAPUGetByteZ(OP1 + IAPU.Registers.X);
   APUSetZN8(IAPU.Registers.YA.B.Y);
   IAPU.PC += 2;
}

void (*S9xApuOpcodes[256])(void) =
{
   Apu00, Apu01, Apu02, Apu03, Apu04, Apu05, Apu06, Apu07,
   Apu08, Apu09, Apu0A, Apu0B, Apu0C, Apu0D, Apu0E, Apu0F,
   Apu10, Apu11, Apu12, Apu13, Apu14, Apu15, Apu16, Apu17,
   Apu18, Apu19, Apu1A, Apu1B, Apu1C, Apu1D, Apu1E, Apu1F,
   Apu20, Apu21, Apu22, Apu23, Apu24, Apu25, Apu26, Apu27,
   Apu28, Apu29, Apu2A, Apu2B, Apu2C, Apu2D, Apu2E, Apu2F,
   Apu30, Apu31, Apu32, Apu33, Apu34, Apu35, Apu36, Apu37,
   Apu38, Apu39, Apu3A, Apu3B, Apu3C, Apu3D, Apu3E, Apu3F,
   Apu40, Apu41, Apu42, Apu43, Apu44, Apu45, Apu46, Apu47,
   Apu48, Apu49, Apu4A, Apu4B, Apu4C, Apu4D, Apu4E, Apu4F,
   Apu50, Apu51, Apu52, Apu53, Apu54, Apu55, Apu56, Apu57,
   Apu58, Apu59, Apu5A, Apu5B, Apu5C, Apu5D, Apu5E, Apu5F,
   Apu60, Apu61, Apu62, Apu63, Apu64, Apu65, Apu66, Apu67,
   Apu68, Apu69, Apu6A, Apu6B, Apu6C, Apu6D, Apu6E, Apu6F,
   Apu70, Apu71, Apu72, Apu73, Apu74, Apu75, Apu76, Apu77,
   Apu78, Apu79, Apu7A, Apu7B, Apu7C, Apu7D, Apu7E, Apu7F,
   Apu80, Apu81, Apu82, Apu83, Apu84, Apu85, Apu86, Apu87,
   Apu88, Apu89, Apu8A, Apu8B, Apu8C, Apu8D, Apu8E, Apu8F,
   Apu90, Apu91, Apu92, Apu93, Apu94, Apu95, Apu96, Apu97,
   Apu98, Apu99, Apu9A, Apu9B, Apu9C, Apu9D, Apu9E, Apu9F,
   ApuA0, ApuA1, ApuA2, ApuA3, ApuA4, ApuA5, ApuA6, ApuA7,
   ApuA8, ApuA9, ApuAA, ApuAB, ApuAC, ApuAD, ApuAE, ApuAF,
   ApuB0, ApuB1, ApuB2, ApuB3, ApuB4, ApuB5, ApuB6, ApuB7,
   ApuB8, ApuB9, ApuBA, ApuBB, ApuBC, ApuBD, ApuBE, ApuBF,
   ApuC0, ApuC1, ApuC2, ApuC3, ApuC4, ApuC5, ApuC6, ApuC7,
   ApuC8, ApuC9, ApuCA, ApuCB, ApuCC, ApuCD, ApuCE, ApuCF,
   ApuD0, ApuD1, ApuD2, ApuD3, ApuD4, ApuD5, ApuD6, ApuD7,
   ApuD8, ApuD9, ApuDA, ApuDB, ApuDC, ApuDD, ApuDE, ApuDF,
   ApuE0, ApuE1, ApuE2, ApuE3, ApuE4, ApuE5, ApuE6, ApuE7,
   ApuE8, ApuE9, ApuEA, ApuEB, ApuEC, ApuED, ApuEE, ApuEF,
   ApuF0, ApuF1, ApuF2, ApuF3, ApuF4, ApuF5, ApuF6, ApuF7,
   ApuF8, ApuF9, ApuFA, ApuFB, ApuFC, ApuFD, ApuFE, ApuFF
};
