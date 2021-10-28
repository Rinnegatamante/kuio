#define _PSP2_KERNEL_SYSMEM_H_
#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/threadmgr/semaphores.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem/data_transfers.h>
#include <psp2kern/kernel/threadmgr/thread.h>
#include <taihen.h>
#include <libk/string.h>
#include <libk/stdio.h>
#include <libk/stdarg.h>
#include "include/kuio.h"

#define CHUNK_SIZE 2048

static uint8_t chunk[CHUNK_SIZE];
static SceUID kuio_thd_id, io_request_mutex, io_result_mutex;
static char fname[256];

#define OPEN_FILE   0
#define WRITE_FILE  1
#define READ_FILE   2
#define CLOSE_FILE  3
#define SEEK_FILE   4
#define REMOVE_FILE 5
#define TELL_FILE   6
#define CREATE_DIR  7
#define REMOVE_DIR  8

typedef struct{
	const char* file;
	int flags;
	SceUID fd;
	SceSize size;
	uint8_t type;
	SceOff pos;
	int offs;
}kuIoRequest;

volatile kuIoRequest io_request;

static int kuio_thread(SceSize args, void *argp)
{
	
	for (;;){
		
		// Waiting for IO request
		ksceKernelWaitSema(io_request_mutex, 1, NULL);
		
		// Executing the received request
		switch(io_request.type){
			case OPEN_FILE:
				io_request.fd = ksceIoOpen(io_request.file, io_request.flags, 6);
				break;
			case WRITE_FILE:
				ksceIoWrite(io_request.fd, chunk, io_request.size);
				break;
			case READ_FILE:
				ksceIoRead(io_request.fd, chunk, io_request.size);
				break;
			case SEEK_FILE:
				ksceIoLseek(io_request.fd, io_request.offs, io_request.flags);
				break;
			case CLOSE_FILE:
				ksceIoClose(io_request.fd);
				break;
			case REMOVE_FILE:
				ksceIoRemove(io_request.file);
				break;
			case TELL_FILE:
				io_request.pos = ksceIoLseek(io_request.fd, 0, SEEK_CUR);
				break;
			case CREATE_DIR:
				ksceIoMkdir(io_request.file, 6);
				break;
			case REMOVE_DIR:
				ksceIoRmdir(io_request.file);
				break;
			default:
				break;
		}
		
		// Sending results
		ksceKernelSignalSema(io_result_mutex, 1);
		
	}
	
	return 0;
	
}

int kuIoOpen(const char *file, int flags, SceUID* res){
	uint32_t state;
	ENTER_SYSCALL(state);
	ksceKernelStrncpyUserToKernel(fname, file, 256);
	SceUID kres = 0;
	
	// Checking if the request is trying to access ux0:/data for security reasons
	#ifdef SAFE_MODE
	if ((strstr(fname, "ux0:data") != NULL) || (strstr(fname, "ux0:/data") != NULL)){
	#endif
	
		// Performing request to kernel thread
		io_request.type = OPEN_FILE;
		io_request.file = fname;
		io_request.flags = flags;
		ksceKernelSignalSema(io_request_mutex, 1);
		
		// Getting results
		ksceKernelWaitSema(io_result_mutex, 1, NULL);
		kres = io_request.fd;
		
	#ifdef SAFE_MODE	
	}
	#endif
	
	ksceKernelMemcpyKernelToUser(res, &kres, sizeof(SceUID));
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoWrite(SceUID fd, const void *data, SceSize size){
	uint32_t state;
	ENTER_SYSCALL(state);
	
	int i = 0;
	int bufsize = 0;
	io_request.type = WRITE_FILE;
	io_request.fd = fd;
	while (i < size){
		if (size - i > CHUNK_SIZE) bufsize = CHUNK_SIZE;
		else bufsize = size - i;
		ksceKernelMemcpyUserToKernel(chunk, (data + i), bufsize);
		i += bufsize;
		
		// Performing request to kernel thread
		io_request.size = bufsize;
		ksceKernelSignalSema(io_request_mutex, 1);
		
		// Waiting results
		ksceKernelWaitSema(io_result_mutex, 1, NULL);
		
	}
	
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoRead(SceUID fd, void *data, SceSize size){
	uint32_t state;
	ENTER_SYSCALL(state);
	
	int i = 0;
	int bufsize = 0;
	io_request.type = READ_FILE;
	io_request.fd = fd;
	while (i < size){
		if (size - i > CHUNK_SIZE) bufsize = CHUNK_SIZE;
		else bufsize = size - i;
		
		// Performing request to kernel thread
		io_request.size = bufsize;
		ksceKernelSignalSema(io_request_mutex, 1);
		
		// Getting results
		ksceKernelWaitSema(io_result_mutex, 1, NULL);
		ksceKernelMemcpyKernelToUser((data + i), chunk, bufsize);
		i += bufsize;
		
	}
	
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoClose(SceUID fd){
	uint32_t state;
	ENTER_SYSCALL(state);
	
	// Performing request to kernel thread
	io_request.type = CLOSE_FILE;
	io_request.fd = fd;
	ksceKernelSignalSema(io_request_mutex, 1);
	
	// Waiting results
	ksceKernelWaitSema(io_result_mutex, 1, NULL);
	
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoLseek(SceUID fd, int offset, int whence){
	uint32_t state;
	ENTER_SYSCALL(state);
	
	// Performing request to kernel thread
	io_request.type = SEEK_FILE;
	io_request.offs = offset;
	io_request.flags = whence;
	ksceKernelSignalSema(io_request_mutex, 1);
		
	// Waiting results
	ksceKernelWaitSema(io_result_mutex, 1, NULL);
	
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoTell(SceUID fd, SceOff* pos){
	uint32_t state;
	ENTER_SYSCALL(state);
	SceOff kpos;
	
	// Performing request to kernel thread
	io_request.type = TELL_FILE;
	io_request.fd = fd;
	ksceKernelSignalSema(io_request_mutex, 1);
		
	// Getting results
	ksceKernelWaitSema(io_result_mutex, 1, NULL);
	kpos = io_request.pos;
	
	ksceKernelMemcpyKernelToUser(pos, &kpos, sizeof(SceUID));
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoMkdir(const char* dir){
	uint32_t state;
	ENTER_SYSCALL(state);
	ksceKernelStrncpyUserToKernel(fname, dir, 256);
	
	// Checking if the request is trying to access ux0:/data for security reasons
	#ifdef SAFE_MODE
	if ((strstr(fname, "ux0:data") != NULL) || (strstr(fname, "ux0:/data") != NULL)){
	#endif
	
		// Performing request to kernel thread
		io_request.type = CREATE_DIR;
		io_request.file = fname;
		ksceKernelSignalSema(io_request_mutex, 1);
		
		// Waiting results
		ksceKernelWaitSema(io_result_mutex, 1, NULL);
	
	#ifdef SAFE_MODE	
	}
	#endif
	
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoRemove(const char* file){
	uint32_t state;
	ENTER_SYSCALL(state);
	ksceKernelStrncpyUserToKernel(fname, file, 256);
	
	// Checking if the request is trying to access ux0:/data for security reasons
	#ifdef SAFE_MODE
	if ((strstr(fname, "ux0:data") != NULL) || (strstr(fname, "ux0:/data") != NULL)){
	#endif
	
		// Performing request to kernel thread
		io_request.type = REMOVE_FILE;
		io_request.file = fname;
		ksceKernelSignalSema(io_request_mutex, 1);
		
		// Waiting results
		ksceKernelWaitSema(io_result_mutex, 1, NULL);
	
	#ifdef SAFE_MODE
	}
	#endif
		
	EXIT_SYSCALL(state);
	return 0;
}

int kuIoRmdir(const char* dir){
	uint32_t state;
	ENTER_SYSCALL(state);
	ksceKernelStrncpyUserToKernel(fname, dir, 256);
	
	// Checking if the request is trying to access ux0:/data for security reasons
	#ifdef SAFE_MODE
	if ((strstr(fname, "ux0:data") != NULL) || (strstr(fname, "ux0:/data") != NULL)){
	#endif
	
		// Performing request to kernel thread
		io_request.type = REMOVE_DIR;
		io_request.file = fname;
		ksceKernelSignalSema(io_request_mutex, 1);
		
		// Waiting results
		ksceKernelWaitSema(io_result_mutex, 1, NULL);
	
	#ifdef SAFE_MODE
	}
	#endif
		
	EXIT_SYSCALL(state);
	return 0;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {	
	
	// Starting I/O mutexes
	io_request_mutex = ksceKernelCreateSema("kuio_request", 0, 0, 1, NULL);
	io_result_mutex = ksceKernelCreateSema("kuio_result", 0, 0, 1, NULL);
	
	// Starting a secondary thread to hold kernel privileges
	kuio_thd_id = ksceKernelCreateThread("kuio_thread", kuio_thread, 0x3C, 0x1000, 0, 0x10000, 0);
	ksceKernelStartThread(kuio_thd_id, 0, NULL);
	
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	return SCE_KERNEL_STOP_SUCCESS;	
}