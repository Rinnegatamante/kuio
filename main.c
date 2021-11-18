
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/fios2.h>
#include <psp2kern/sblacmgr.h>
#include "include/kuio.h"

#define SAFE_MODE 1

int kuioCpyPath(SceUID pid, char *dst, int max, const char *src){

	int res = 0x80010016;
	char path[0x400];

	if(pid == SCE_GUID_KERNEL_PROCESS_ID)
		goto end;

	SceSSize str_res = ksceKernelStrncpyFromUser(path, src, sizeof(path));

	if(str_res == 0 || str_res == sizeof(path) || strncmp(path, "sdstor", 6) == 0)
		goto end;

	res = ksceFiosKernelOverlayResolveSync(pid, 0, path, dst, max);
	if(res < 0)
		goto end;

#ifdef SAFE_MODE	
	if(strncmp(dst, "ux0:/data/", 10) != 0)
		res = 0x80010016;
#endif

end:
	return res;
}

int kuIoOpen(const char *file, int flags, SceUID *res){

	uint32_t state;
	int result;
	SceUID pid, fd_guid, fd_puid = 0x80010016;
	char path_resolved[0x400];

	ENTER_SYSCALL(state);

	pid = ksceKernelGetProcessId();

	result = kuioCpyPath(pid, path_resolved, sizeof(path_resolved), file);
	if(result >= 0){
		int curr_al = ksceKernelSetPermission(0x40);
		fd_guid = ksceIoOpen(path_resolved, flags, 6);
		ksceKernelSetPermission(curr_al);

		if(fd_guid >= 0){
			fd_puid = kscePUIDOpenByGUID(pid, fd_guid);
			result = 0;
		}else{
			fd_puid = fd_guid;
			result = fd_guid;
		}
	}

	ksceKernelMemcpyKernelToUser(res, &fd_puid, sizeof(*res));
	EXIT_SYSCALL(state);
	return result;
}

int kuIoClose(SceUID fd){

	int res = 0x80010016;
	uint32_t state;
	SceUID pid, fd_guid;
	ENTER_SYSCALL(state);

	int curr_al = ksceKernelSetPermission(0x40);

	pid = ksceKernelGetProcessId();
	if(pid == SCE_GUID_KERNEL_PROCESS_ID)
		goto end;

	fd_guid = kscePUIDtoGUID(pid, fd);
	if(fd_guid < 0){
		res = fd_guid;
		goto end;
	}

	res = ksceIoClose(fd_guid);
	if(res >= 0){
		res = kscePUIDClose(pid, fd);
	}

end:
	ksceKernelSetPermission(curr_al);
	EXIT_SYSCALL(state);
	return res;
}

int kuIoRead(SceUID fd, void *data, SceSize size){

	uint32_t state;
	int res = 0x80010016;
	SceUID pid, fd_guid;
	void *kernel_page = NULL;
	SceSize kernel_size = 0;
	SceUInt32 kernel_offset = 0;

	ENTER_SYSCALL(state);

	int curr_al = ksceKernelSetPermission(0x40);

	pid = ksceKernelGetProcessId();
	if(pid == SCE_GUID_KERNEL_PROCESS_ID)
		goto end;

	fd_guid = kscePUIDtoGUID(pid, fd);
	if(fd_guid < 0){
		res = fd_guid;
		goto end;
	}

	int type = (ksceSblACMgrIsGameProgram(0) != 0) ? 0x11 : 1;

	SceUID mapid = ksceKernelMapUserBlock("KuioUserRefer", SCE_KERNEL_MEMORY_REF_PERM_USER_W, type, data, size, &kernel_page, &kernel_size, &kernel_offset);
	if(mapid < 0){
		res = mapid;
		goto end;
	}

	res = ksceIoRead(fd_guid, (void *)(((uintptr_t)kernel_page) + kernel_offset), size);

	ksceKernelMemBlockRelease(mapid);

end:
	ksceKernelSetPermission(curr_al);
	EXIT_SYSCALL(state);
	return res;
}

int kuIoWrite(SceUID fd, const void *data, SceSize size){

	uint32_t state;
	int res = 0x80010016;
	SceUID pid, fd_guid;
	void *kernel_page = NULL;
	SceSize kernel_size = 0;
	SceUInt32 kernel_offset = 0;

	ENTER_SYSCALL(state);

	int curr_al = ksceKernelSetPermission(0x40);

	pid = ksceKernelGetProcessId();
	if(pid == SCE_GUID_KERNEL_PROCESS_ID)
		goto end;

	fd_guid = kscePUIDtoGUID(pid, fd);
	if(fd_guid < 0){
		res = fd_guid;
		goto end;
	}

	int type = (ksceSblACMgrIsGameProgram(0) != 0) ? 0x11 : 1;

	SceUID mapid = ksceKernelMapUserBlock("KuioUserRefer", SCE_KERNEL_MEMORY_REF_PERM_USER_R, type, data, size, &kernel_page, &kernel_size, &kernel_offset);
	if(mapid < 0){
		res = mapid;
		goto end;
	}

	res = ksceIoWrite(fd_guid, (const void *)(((uintptr_t)kernel_page) + kernel_offset), size);

	ksceKernelMemBlockRelease(mapid);

end:
	ksceKernelSetPermission(curr_al);
	EXIT_SYSCALL(state);
	return res;
}

int kuIoLseek(SceUID fd, int offset, int whence){

	uint32_t state;
	int res = 0x80010016;
	SceUID pid, fd_guid;

	ENTER_SYSCALL(state);

	int curr_al = ksceKernelSetPermission(0x40);

	pid = ksceKernelGetProcessId();
	if(pid == SCE_GUID_KERNEL_PROCESS_ID)
		goto end;

	fd_guid = kscePUIDtoGUID(pid, fd);
	if(fd_guid < 0){
		res = fd_guid;
		goto end;
	}

	SceOff seek_res = ksceIoLseek(fd_guid, (int)offset, whence);
	if(seek_res < 0LL)
		res = (int)seek_res;
	else
		res = 0;

end:
	ksceKernelSetPermission(curr_al);
	EXIT_SYSCALL(state);
	return res;
}

int kuIoTell(SceUID fd, SceOff *pos){

	uint32_t state;
	int res = 0x80010016;
	SceUID pid, fd_guid;

	ENTER_SYSCALL(state);

	int curr_al = ksceKernelSetPermission(0x40);

	pid = ksceKernelGetProcessId();
	if(pid == SCE_GUID_KERNEL_PROCESS_ID)
		goto end;

	fd_guid = kscePUIDtoGUID(pid, fd);
	if(fd_guid < 0){
		res = fd_guid;
		goto end;
	}

	SceOff seek_res = ksceIoLseek(fd_guid, 0, SCE_SEEK_CUR);
	if(seek_res < 0LL){
		res = (int)seek_res;
		seek_res = 0LL;
	}else{
		res = 0;
	}

	ksceKernelMemcpyToUser(pos, &seek_res, sizeof(*pos));

end:
	ksceKernelSetPermission(curr_al);
	EXIT_SYSCALL(state);
	return res;
}

int kuIoMkdir(const char* dir){

	uint32_t state;
	int res;
	char path_resolved[0x400];

	ENTER_SYSCALL(state);

	res = kuioCpyPath(ksceKernelGetProcessId(), path_resolved, sizeof(path_resolved), dir);
	if(res >= 0){
		int curr_al = ksceKernelSetPermission(0x40);
		res = ksceIoMkdir(path_resolved, 6);
		ksceKernelSetPermission(curr_al);
	}

	EXIT_SYSCALL(state);
	return res;
}

int kuIoRemove(const char *file){

	uint32_t state;
	int res;
	char path_resolved[0x400];

	ENTER_SYSCALL(state);

	res = kuioCpyPath(ksceKernelGetProcessId(), path_resolved, sizeof(path_resolved), file);
	if(res >= 0){
		int curr_al = ksceKernelSetPermission(0x40);
		res = ksceIoRemove(path_resolved);
		ksceKernelSetPermission(curr_al);
	}

	EXIT_SYSCALL(state);
	return res;
}

int kuIoRmdir(const char *dir){

	uint32_t state;
	int res;
	char path_resolved[0x400];

	ENTER_SYSCALL(state);

	res = kuioCpyPath(ksceKernelGetProcessId(), path_resolved, sizeof(path_resolved), dir);
	if(res >= 0){
		int curr_al = ksceKernelSetPermission(0x40);
		res = ksceIoRmdir(path_resolved);
		ksceKernelSetPermission(curr_al);
	}

	EXIT_SYSCALL(state);
	return res;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
	return SCE_KERNEL_START_SUCCESS;
}
