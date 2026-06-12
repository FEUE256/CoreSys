#pragma once 

#include <types.h>
#include <status.h>

num_t tsk_ready = 0;

CS_STATUS_T status; // Status
CS_SUBSYS_STATUS_T sstatus;

uint32_t bar2;
uintptr_t fb;
volatile uint32_t* framebuffer;