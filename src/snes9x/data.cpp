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

uint8 add32_32 [32][32] = {
{ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
  0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,
  0x1e,0x1f},
{ 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,
  0x1f,0x1f},
{ 0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
  0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
  0x1f,0x1f},
{ 0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,
  0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,
  0x1f,0x1f},
{ 0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,
  0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,
  0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,
  0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,
  0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
  0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
  0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
  0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
  0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,
  0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
  0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,
  0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,
  0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x1b,0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f},
{ 0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
  0x1f,0x1f}
};

uint8 add32_32_half [32][32] = {
{ 0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,
  0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,
  0x0f,0x0f},
{ 0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,
  0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,
  0x0f,0x10},
{ 0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,
  0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,
  0x10,0x10},
{ 0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,
  0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,
  0x10,0x11},
{ 0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,
  0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,
  0x11,0x11},
{ 0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,
  0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,
  0x11,0x12},
{ 0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,
  0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,
  0x12,0x12},
{ 0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,
  0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,
  0x12,0x13},
{ 0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,
  0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,
  0x13,0x13},
{ 0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,
  0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,
  0x13,0x14},
{ 0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,
  0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,
  0x14,0x14},
{ 0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,
  0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,
  0x14,0x15},
{ 0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,
  0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,
  0x15,0x15},
{ 0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,
  0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,
  0x15,0x16},
{ 0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,
  0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,
  0x16,0x16},
{ 0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,
  0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,
  0x16,0x17},
{ 0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,
  0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,
  0x17,0x17},
{ 0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,
  0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,
  0x17,0x18},
{ 0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,
  0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,
  0x18,0x18},
{ 0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,
  0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,
  0x18,0x19},
{ 0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,
  0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,
  0x19,0x19},
{ 0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,
  0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,
  0x19,0x1a},
{ 0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,
  0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,
  0x1a,0x1a},
{ 0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,
  0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,
  0x1a,0x1b},
{ 0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,
  0x13,0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,
  0x1b,0x1b},
{ 0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,
  0x14,0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,
  0x1b,0x1c},
{ 0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,
  0x14,0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,0x1b,
  0x1c,0x1c},
{ 0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,
  0x15,0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1c,
  0x1c,0x1d},
{ 0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,
  0x15,0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1c,0x1c,
  0x1d,0x1d},
{ 0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,
  0x16,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1c,0x1c,0x1d,
  0x1d,0x1e},
{ 0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,
  0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1c,0x1c,0x1d,0x1d,
  0x1e,0x1e},
{ 0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,0x14,0x15,0x15,0x16,0x16,
  0x17,0x17,0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1c,0x1c,0x1d,0x1d,0x1e,
  0x1e,0x1f}
};
uint8 sub32_32 [32][32] = {
{ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
  0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,
  0x1e,0x1f},
{ 0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,
  0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,
  0x1d,0x1e},
{ 0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
  0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
  0x1c,0x1d},
{ 0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,
  0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,
  0x1b,0x1c},
{ 0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
  0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
  0x1a,0x1b},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
  0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
  0x19,0x1a},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
  0x18,0x19},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
  0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
  0x17,0x18},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
  0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,
  0x16,0x17},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,
  0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,
  0x15,0x16},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
  0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,
  0x14,0x15},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,
  0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,
  0x13,0x14},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,
  0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,
  0x12,0x13},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
  0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
  0x11,0x12},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
  0x10,0x11},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
  0x0f,0x10},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,
  0x0e,0x0f},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
  0x0d,0x0e},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,
  0x0c,0x0d},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
  0x0b,0x0c},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,
  0x0a,0x0b},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0a},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
  0x08,0x09},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
  0x07,0x08},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,
  0x06,0x07},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
  0x05,0x06},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,
  0x04,0x05},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,
  0x03,0x04},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
  0x02,0x03},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x01,0x02},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00}
};

uint8 sub32_32_half [32][32] = {
{ 0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,
  0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,
  0x0f,0x0f},
{ 0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,
  0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,
  0x0e,0x0f},
{ 0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,
  0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,
  0x0e,0x0e},
{ 0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,
  0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,
  0x0d,0x0e},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,
  0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,
  0x0d,0x0d},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,
  0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,
  0x0c,0x0d},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,
  0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,
  0x0c,0x0c},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,
  0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,
  0x0b,0x0c},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,
  0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,
  0x0b,0x0b},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,
  0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,
  0x0a,0x0b},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,
  0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,
  0x0a,0x0a},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,
  0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,
  0x09,0x0a},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
  0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,
  0x09,0x09},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,
  0x08,0x09},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,
  0x08,0x08},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,
  0x07,0x08},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,
  0x07,0x07},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,
  0x06,0x07},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,
  0x06,0x06},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,
  0x05,0x06},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,
  0x05,0x05},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,
  0x04,0x05},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,
  0x04,0x04},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,0x03,
  0x03,0x04},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x02,
  0x03,0x03},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,
  0x02,0x03},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,
  0x02,0x02},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
  0x01,0x02},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x01,0x01},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x01},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00}
};


uint8 mul_brightness [16][32] = {
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00},
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
  0x02,0x02},
{ 0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,
  0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x04,0x04,0x04,
  0x04,0x04},
{ 0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x03,0x03,
  0x03,0x03,0x03,0x04,0x04,0x04,0x04,0x04,0x05,0x05,0x05,0x05,0x05,0x06,0x06,
  0x06,0x06},
{ 0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x04,
  0x04,0x04,0x05,0x05,0x05,0x05,0x06,0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x08,
  0x08,0x08},
{ 0x00,0x00,0x01,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,0x05,
  0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x07,0x08,0x08,0x08,0x09,0x09,0x09,0x0a,
  0x0a,0x0a},
{ 0x00,0x00,0x01,0x01,0x02,0x02,0x02,0x03,0x03,0x04,0x04,0x04,0x05,0x05,0x06,
  0x06,0x06,0x07,0x07,0x08,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0a,0x0b,0x0b,0x0c,
  0x0c,0x0c},
{ 0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,
  0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,
  0x0e,0x0e},
{ 0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,
  0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,
  0x10,0x11},
{ 0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x04,0x05,0x05,0x06,0x07,0x07,0x08,0x08,
  0x09,0x0a,0x0a,0x0b,0x0b,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x10,0x10,0x11,0x11,
  0x12,0x13},
{ 0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x05,0x05,0x06,0x07,0x07,0x08,0x09,0x09,
  0x0a,0x0b,0x0b,0x0c,0x0d,0x0d,0x0e,0x0f,0x0f,0x10,0x11,0x11,0x12,0x13,0x13,
  0x14,0x15},
{ 0x00,0x01,0x01,0x02,0x03,0x04,0x04,0x05,0x06,0x07,0x07,0x08,0x09,0x0a,0x0a,
  0x0b,0x0c,0x0c,0x0d,0x0e,0x0f,0x0f,0x10,0x11,0x12,0x12,0x13,0x14,0x15,0x15,
  0x16,0x17},
{ 0x00,0x01,0x02,0x02,0x03,0x04,0x05,0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,
  0x0c,0x0d,0x0e,0x0e,0x0f,0x10,0x11,0x12,0x12,0x13,0x14,0x15,0x16,0x16,0x17,
  0x18,0x19},
{ 0x00,0x01,0x02,0x03,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,0x0c,
  0x0d,0x0e,0x0f,0x10,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x17,0x18,0x19,
  0x1a,0x1b},
{ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,
  0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
  0x1c,0x1d},
{ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
  0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,
  0x1e,0x1f}
};


