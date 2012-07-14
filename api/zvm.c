/*
 * ZeroVM API. contain zerovm api functions
 * notice: this is an untrusted code
 *
 *  Created on: Dec 2, 2011
 *      Author: d'b
 */

#include <string.h>
#include <stdio.h>

#define USER_SIDE
#include "zvm.h"

/* pointer to trap() through the trampoline */
static int32_t (*_trap)(uint64_t *in) = (int32_t (*)(uint64_t*))
    0x10000 /* start of trampoline */ +
    0x20 /* size of trampoline record */ *
    0 /* onering syscall number */;

/* holds all information which could get via api */
static struct UserManifest setup;

/* initialization of zerovm api */
struct UserManifest *zvm_init()
{
  struct UserManifest *result = &setup;

  /* system */
  result->syscallback = zvm_syscallback(1);
  result->heap_ptr = zvm_heap_ptr();
  result->mem_size = zvm_mem_size();

  /* get channels count */
  result->channels_count = zvm_channels(NULL);
  result->channels = calloc(result->channels_count, sizeof(*result->channels));
  zvm_channels(result->channels);

  /* limits, counters */
  result->syscalls_limit = zvm_syscalls_limit();
  result->syscalls_count = zvm_syscalls_count();

  return result;
}

/* wrapper for zerovm "TrapRead" */
int32_t zvm_pread(int desc, char *buffer, int32_t size, int64_t offset)
{
  uint64_t request[] = {TrapRead, 0, desc, (uint32_t)buffer, size, offset};
  return _trap(request);
}

/* wrapper for zerovm "TrapWrite" */
int32_t zvm_pwrite(int desc, char *buffer, int32_t size, int64_t offset)
{
  uint64_t request[] = {TrapWrite, 0, desc, (uint32_t)buffer, size, offset};
  return _trap(request);
}

/* wrapper for zerovm "TrapExit" */
int32_t zvm_exit(int32_t code)
{
  uint64_t request[] = {TrapExit, 0, code};
  return _trap(request);
}

/*
 * set syscallback address. return active syscallback
 * hint: to get syscallback w/o redefining it any invalid pointer
 * could be used, for example 1
 */
int32_t zvm_syscallback(intptr_t addr)
{
  uint64_t request[] = {TrapSyscallback, 0, addr};
  return _trap(request);
}

/* return user heap starting address */
void* zvm_heap_ptr()
{
  uint64_t request[] = {TrapHeapPtr};
  return (void*)_trap(request);
}

/*
 * return user memory size. note that this is not the heap
 * size, but whole memory available for user
 */
uint32_t zvm_mem_size()
{
  uint64_t request[] = {TrapMemSize};
  return _trap(request);
}

/*
 * called with NULL return channels number, otherwise copy
 * channels information into provided space
 */
int32_t zvm_channels(struct ZVMChannel *channels)
{
  uint64_t request[] = {TrapChannels, 0, (intptr_t)channels};
  return _trap(request);
}

/* return syscalls count */
int64_t zvm_syscalls_count()
{
  int64_t result;
  uint64_t request[] = {TrapSyscallsCount, 0, (intptr_t)&result};
  _trap(request); /* ignore the returnings, real value passed by reference */
  return result;
}

/* return syscalls limit */
int64_t zvm_syscalls_limit()
{
  int64_t result;
  uint64_t request[] = {TrapSyscallsLimit, 0, (intptr_t)&result};
  _trap(request); /* ignore the returnings, real value passed by reference */
  return result;
}

#undef USER_SIDE
