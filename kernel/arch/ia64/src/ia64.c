/*
 * Copyright (c) 2005 Jakub Jermar
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

/** @addtogroup ia64
 * @{
 */
/** @file
 */

#include <arch.h>
#include <arch/ski/ski.h>
#include <arch/drivers/it.h>
#include <arch/interrupt.h>
#include <arch/barrier.h>
#include <arch/asm.h>
#include <arch/register.h>
#include <arch/types.h>
#include <arch/context.h>
#include <arch/stack.h>
#include <arch/mm/page.h>
#include <mm/as.h>
#include <config.h>
#include <userspace.h>
#include <console/console.h>
#include <proc/uarg.h>
#include <syscall/syscall.h>
#include <ddi/irq.h>
#include <ddi/device.h>
#include <arch/bootinfo.h>
#include <arch/drivers/ega.h>
#include <genarch/drivers/ega/ega.h>
#include <genarch/kbd/i8042.h>
#include <genarch/kbd/ns16550.h>
#include <smp/smp.h>
#include <smp/ipi.h>
#include <arch/atomic.h>
#include <panic.h>
#include <print.h>
#include <sysinfo/sysinfo.h>

/* NS16550 as a COM 1 */
#define NS16550_IRQ	(4 + LEGACY_INTERRUPT_BASE)
#define NS16550_PORT	0x3f8

bootinfo_t *bootinfo;

static uint64_t iosapic_base = 0xfec00000;

void arch_pre_main(void)
{
	/* Setup usermode init tasks. */

	unsigned int i;
	
	init.cnt = bootinfo->taskmap.count;
	
	for (i = 0; i < init.cnt; i++) {
		init.tasks[i].addr =
		    ((unsigned long) bootinfo->taskmap.tasks[i].addr) |
		    VRN_MASK;
		init.tasks[i].size = bootinfo->taskmap.tasks[i].size;
	}
}

void arch_pre_mm_init(void)
{
	/*
	 * Set Interruption Vector Address (i.e. location of interruption vector
	 * table).
	 */
	iva_write((uintptr_t) &ivt);
	srlz_d();
	
}

static void iosapic_init(void)
{
	uint64_t IOSAPIC = PA2KA((unative_t)(iosapic_base)) | FW_OFFSET;
	int i;
	
	int myid, myeid;
	
	myid = ia64_get_cpu_id();
	myeid = ia64_get_cpu_eid();

	for (i = 0; i < 16; i++) { 
		if (i == 2)
			continue;	 /* Disable Cascade interrupt */
		((uint32_t *)(IOSAPIC + 0x00))[0] = 0x10 + 2 * i;
		srlz_d();
		((uint32_t *)(IOSAPIC + 0x10))[0] = LEGACY_INTERRUPT_BASE + i;
		srlz_d();
		((uint32_t *)(IOSAPIC + 0x00))[0] = 0x10 + 2 * i + 1;
		srlz_d();
		((uint32_t *)(IOSAPIC + 0x10))[0] = myid << (56 - 32) |
		    myeid << (48 - 32);
		srlz_d();
	}

}


void arch_post_mm_init(void)
{
	if (config.cpu_active == 1) {
		iosapic_init();
		irq_init(INR_COUNT, INR_COUNT);
#ifdef SKI
		ski_init_console();
#else	
		ega_init(EGA_BASE, EGA_VIDEORAM);
#endif	
	}
	it_init();
		
}

void arch_post_cpu_init(void)
{
}

void arch_pre_smp_init(void)
{
}


#ifdef I460GX
#define POLL_INTERVAL		50000		/* 50 ms */
/** Kernel thread for polling keyboard. */
static void i8042_kkbdpoll(void *arg)
{
	while (1) {
#ifdef CONFIG_NS16550
	#ifndef CONFIG_NS16550_INTERRUPT_DRIVEN
		ns16550_poll();
	#endif	
#else
	#ifndef CONFIG_I8042_INTERRUPT_DRIVEN
		i8042_poll();
	#endif	
#endif
		thread_usleep(POLL_INTERVAL);
	}
}
#endif

void arch_post_smp_init(void)
{
	thread_t *t;

	/*
	 * Create thread that polls keyboard.
	 */
#ifdef SKI
	t = thread_create(kkbdpoll, NULL, TASK, 0, "kkbdpoll", true);
	if (!t)
		panic("cannot create kkbdpoll\n");
	thread_ready(t);
#endif		

#ifdef I460GX
	devno_t kbd = device_assign_devno();

#ifdef CONFIG_NS16550
	ns16550_init(kbd, NS16550_PORT, NS16550_IRQ, NULL, NULL);
#else
	devno_t mouse = device_assign_devno();
	i8042_init(kbd, IRQ_KBD, mouse, IRQ_MOUSE);
#endif
	t = thread_create(i8042_kkbdpoll, NULL, TASK, 0, "kkbdpoll", true);
	if (!t)
		panic("cannot create kkbdpoll\n");
	thread_ready(t);
#endif

	sysinfo_set_item_val("ia64_iospace", NULL, true);
	sysinfo_set_item_val("ia64_iospace.address", NULL, true);
	sysinfo_set_item_val("ia64_iospace.address.virtual", NULL, IO_OFFSET);
}


/** Enter userspace and never return. */
void userspace(uspace_arg_t *kernel_uarg)
{
	psr_t psr;
	rsc_t rsc;

	psr.value = psr_read();
	psr.cpl = PL_USER;
	psr.i = true;			/* start with interrupts enabled */
	psr.ic = true;
	psr.ri = 0;			/* start with instruction #0 */
	psr.bn = 1;			/* start in bank 0 */

	asm volatile ("mov %0 = ar.rsc\n" : "=r" (rsc.value));
	rsc.loadrs = 0;
	rsc.be = false;
	rsc.pl = PL_USER;
	rsc.mode = 3;			/* eager mode */

	switch_to_userspace((uintptr_t) kernel_uarg->uspace_entry,
	    ((uintptr_t) kernel_uarg->uspace_stack) + PAGE_SIZE -
	    ALIGN_UP(STACK_ITEM_SIZE, STACK_ALIGNMENT),
	    ((uintptr_t) kernel_uarg->uspace_stack) + PAGE_SIZE,
	    (uintptr_t) kernel_uarg->uspace_uarg, psr.value, rsc.value);

	while (1)
		;
}

/** Set thread-local-storage pointer.
 *
 * We use r13 (a.k.a. tp) for this purpose.
 */
unative_t sys_tls_set(unative_t addr)
{
        return 0;
}

/** Acquire console back for kernel
 *
 */
void arch_grab_console(void)
{
#ifdef SKI
	ski_kbd_grab();
#else
#ifdef CONFIG_NS16550
	ns16550_grab();
#else
	i8042_grab();
#endif	
#endif	
}

/** Return console to userspace
 *
 */
void arch_release_console(void)
{
#ifdef SKI
	ski_kbd_release();
#else	
#ifdef CONFIG_NS16550
	ns16550_release();
#else	
	i8042_release();
#endif	
#endif
}

void arch_reboot(void)
{
	outb(0x64, 0xfe);
	while (1)
		;
}

/** @}
 */
