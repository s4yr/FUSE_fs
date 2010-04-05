#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <gcrypt.h>
#include "functions.h"
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
char name1[10] = "/file";	
char name2[10];
static int savefd;
char* intToChar(char* bufer, int numb);
static struct fuse_session* se;

static int pr_getattr(const char* path,struct stat* st_buf)		//get attributes about file/directory
{
	int res;

	res = lstat(path, st_buf);
	if (res == -1)
		return -errno;
	return 0;
}
static int pr_fgetattr(const char* path, struct stat* st_buf)
{
	int res;
	res = lstat(path,st_buf);
	if(res == -1)
		return -errno;
	return 0;
}
	
static int pr_access(const char *path, int mask)
{
	int res;
	res = access(path, mask);
	if (res == -1)
		return -errno;
	return 0;
}
static int pr_readlink(const char *path, char *buf, size_t size)		//read symbolik link -??????
{
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;
	buf[res] = '\0';
	return 0;
}
//read content of current directory
static int pr_readdir(const char* path, void* buf, fuse_fill_dir_t filler,off_t offset,struct fuse_file_info* fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) 
	{
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;	
		if (filler(buf, de->d_name, &st, 0))
			break;
	}
	closedir(dp);
	return 0;
}
//syslog
//var/log/messages
// fuse realization of mkdir
static int my_printf()
{
	size_t len = 20;
	char *tmp = alloca (len + 2);
	printf("qwerty\n");
	write(1,"qwe\n",5);
	write(2,"qwert\n",7);
	return 0;
}
static int pr_mkdir(const char *path, mode_t mode)
{
	int res;
	res = mkdir(path, mode);
	if (res == -1)
		return -errno;
	FILE* fp = fopen( "/tmp/afuse.log", "w+" );
	fprintf( fp, "PRINT FROM FUSE: %s\n",path);
	fclose(fp);
	my_printf();
	return 0;
}
//fuse realization of delete dir
static int pr_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

//read file content
static int pr_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;
	offset= 3;
	res = pread(fd, buf, size, offset);	//read file contenxt
	if (res == -1)
		res = -errno;
	close(fd);
	return res;
}
//open file
static int pr_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;
	close(res);
	return 0;
}
//-----------------------------------------------------------
static int pr_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int pr_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int pr_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int pr_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}
//
static struct fuse_operations pr_operations = {
	.getattr		= pr_getattr,
	.access			= pr_access,
	.readlink		= pr_readlink,
	.readdir		= pr_readdir,
	.mkdir			= pr_mkdir,
	.rmdir			= pr_rmdir,
	.read			= pr_read,
	};	
int main(int argc,char* argv[])
{
	DIR* dp;
	int ret;	
	umask(0);
	struct fuse_args args;
	ret = fuse_main(argc,argv,&pr_operations,NULL);;
	return ret;
}
