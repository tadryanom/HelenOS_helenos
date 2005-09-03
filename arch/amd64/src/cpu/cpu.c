/*
 * Copyright (C) 2001-2004 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch/cpu.h>
#include <arch/cpuid.h>
#include <arch/pm.h>

#include <arch.h>
#include <arch/types.h>
#include <print.h>
#include <typedefs.h>

/*
 * Identification of CPUs.
 * Contains only non-MP-Specification specific SMP code.
 */
#define AMD_CPUID_EBX	0x68747541
#define AMD_CPUID_ECX 	0x444d4163
#define AMD_CPUID_EDX 	0x69746e65

#define INTEL_CPUID_EBX	0x756e6547
#define INTEL_CPUID_ECX 0x6c65746e
#define INTEL_CPUID_EDX 0x49656e69


enum vendor {
	VendorUnknown=0,
	VendorAMD,
	VendorIntel
};

static char *vendor_str[] = {
	"Unknown Vendor",
	"AuthenticAMD",
	"GenuineIntel"
};

void set_TS_flag(void)
{
	asm
	(
		"mov %%cr0,%%rax;"
		"or $8,%%rax;"
		"mov %%rax,%%cr0;"
		:
		:
		:"%rax"
	);
}

void reset_TS_flag(void)
{
	asm
	(
		"mov %%cr0,%%rax;"
		"btc $4,%%rax;"
		"mov %%rax,%%cr0;"
		:
		:
		:"%rax"
	);	
}
