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
//#include "gpg_global.h"
//#include "gpg_support.h"
#include "gpg_create_sign.c"
#include "gpg_ver.c"
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
gpgme_ctx_t ctx;
gpgme_error_t err;
char name1[10] = "/file";	
char name2[10];
static int savefd;
char* intToChar(char* bufer, int numb);
static struct fuse_session* se;
char* make_sig_path(char* file_path)	//create path to signature of selected file
{
	char tmp[512];
	char* p_tmp;
	p_tmp = strcpy(tmp,file_path);
	p_tmp = strcat(p_tmp,".sig");
	return p_tmp;	
}
//---------get attributes of file or directory------------------compilr -f tmp/
static int pr_getattr(const char* path,struct stat* st_buf)		//get attributes about file/directory
{
	int res;

	res = lstat(path, st_buf);
	if (res == -1)
		return -errno;
	return 0;
}
//---------get attributes of file or directory------------------
static int pr_fgetattr(const char* path, struct stat* st_buf)
{
	int res;
	res = lstat(path,st_buf);
	if(res == -1)
		return -errno;
	return 0;
}
//--------check a possible access to file or directory-----------	
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
//------------read content of current directory-------------
static int pr_readdir(const char* path, void* buf, fuse_fill_dir_t filler,off_t offset,struct fuse_file_info* fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
	//printf("readdir path: %s\n",path);
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
//---------------read file content--------------------------
static int pr_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	int fd;
	int res;
	char* if_sig_ptr;
	char* sig_ptr;
	char* file_ptr;
	char* dir_ptr;
	char buff[100];
	char dir_buf[100];
	char tmp[100];
	int i = 0;
	char sig_st[100];
//	printf("pr_read\n");
	dir_ptr = strcpy(dir_buf,path);
	file_ptr = strcpy(buff,path);
	sig_ptr = strcat(buff,".sig");
//	printf("path in read: %s\n",path);
	//sig_ptr = make_sig_path(buff);
	if(strstr(path,".c") != NULL)
	{
		//create_signature(ctx,file_ptr,NULL);
		//printf("signature was created\n");
	}
	for(i = strlen(dir_ptr) - 1; i > 0; i--)	//get path of current directory
	{
		if(*(dir_ptr + i) == '/')
		{
			*(dir_ptr + i) = '\0';
			break;
		}
	}
	//cheking on existing of signature------------
	DIR* dp;
	struct dirent* de;
	dp = opendir(dir_ptr);
	if(dp == NULL)
	{
		printf("err with opendir");
		return -errno;
	}
	while((de = readdir(dp)) != NULL)
	{
		strcpy(tmp,dir_ptr);			//copy directory path
		strcat(tmp,"/");
		strcat(tmp,de->d_name);
		if(strcmp(sig_ptr,tmp) == NULL)	//signature of select file exist
		{
			printf("signature of file: %s\n",tmp);
			strcpy(sig_st,verify_signature(ctx,sig_ptr,path));
			printf("signature was verified status: %s\n",sig_st);	//sig_st contain result of verrification
			if(strcmp(sig_st,"Success") == NULL)					//correct signature
			{
				printf("-------success signature-------\n");
				fd = open(path, O_RDONLY);
				if (fd == -1)
					return -errno;
				res = pread(fd, buf, size, offset);	//read file contenxt
				if (res == -1)
					res = -errno;
				close(fd);
				return res;					
			}
			else
			{
				if(strcmp(sig_st,"Bad signature") == NULL)	//bad signature
				{
					printf("-------bad signature-------\n");
					system("echo sorry,but signature is bad");
					return -EACCES;					
				}
				else{printf("ERROR with verification\n");return -errno;}
			}
			break;			 
		}
	}
	closedir(dp);
	//------end block of checking on existing signature--------
	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;
	res = pread(fd, buf, size, offset);	//read file contenxt
	if (res == -1)
		res = -errno;
	//printf("in read before close path: %s buf:%s\n",path,buf);
	close(fd);
	return res;
}
//---------------------open file----------------------------
static int pr_open(const char *path, struct fuse_file_info *fi)
{  
	int res;
	int lenght = 0;
	char tmp[100];
	char tmp_sig[100];
	char* tmp_sig_p;
	char* tmp_p;
	char command_create_sig[100];
	char command_verify_sig[100];
	printf("pr_open");
	res = open(path, fi->flags);
	if (res == -1)
		return -errno;
	
	strcpy(tmp,path);		//creating path to file
/*	strcpy(tmp_sig,path);
	tmp_sig_p = strcat(tmp_sig,".sig");	//create a path to signature of selected file
	strcpy(command_create_sig,"gpg -b ");
	strcpy(command_verify_sig,"gpg --verify ");
	write(1,"open ",6);
	printf("%s flag: %d\n",path,fi->flags);
	printf("lenght: %d \n",strlen(path));
	printf("tmp: %s\n",tmp);
	*/
	/*if(strstr(tmp,".c") != NULL)
	{
		tmp_p = strcat(command_verify_sig,tmp_sig_p);
		tmp_p = strcat(tmp_p," ");
		tmp_p = strcat(tmp_p,tmp);
		printf("check signature: %s\n",tmp_p);
		system(tmp_p);
		puts("signature was verrified");
	}
	*/
	//printf("in open before close path %s\n",path);
	close(res);
	return 0;
}
//----------------------write into file------------------
static int pr_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	(void) fi;
	printf("pr_write");
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	close(fd);
	//create_signature(ctx,path,NULL);
	//puts("signature was created");
	return res;
}
//-------create a new dirrectory in fs-------------------
static int pr_mkdir(const char *path, mode_t mode)
{
	int res;
	char str[10];
	//gets(str);
	res = mkdir(path, mode);
	if (res == -1)
		return -errno;
	FILE* fp = fopen( "/tmp/afuse.log", "w+" );
	fprintf( fp, "PRINT FROM FUSE: %s\n",path);
	fclose(fp);
	puts("mkdir");
	return 0;
}
//----- create ordinary file in new filesystem---------
int pr_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	printf("mknod path: %s\n",path);
	if (S_ISREG(mode)) 
	{
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}
//---information about mount fs--------------------------sed
static int pr_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;
	return 0;
}
//-------rename name of file---------------------------
static int pr_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}
// fuse realization of mkdir
static int my_printf()
{
	size_t len = 20;
	char *tmp = alloca (len + 2);
	write(1,"inp\n",5);
	return 0;
}
//----------fuse realization of delete dir-----------------
static int pr_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

//-------create a symbolic link of a file-------------
static int pr_symlink(const char *from, const char *to)
{
	int res;
	res = symlink(from, to);
	if (res == -1)
		return -errno;
	return 0;
}
//------create a hard link of a file------------------
static int pr_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}
//-----delete name of file-----------------------------
static int pr_unlink(const char *path)
{
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}
//---truncate a file to size----------------------------
static int pr_truncate(const char *path, off_t size)
{
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}
//-------change file last time file was accessed-----------
static int pr_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
		return -errno;
	return 0;
}
static int pr_release(const char *path, struct fuse_file_info *fi)
{

	(void) path;
	(void) fi;
	return 0;
}
//--write cashe of file on disc---(not realiz)
static int pr_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	/* can be left */
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}
//---change attributes of a file---------------------------
static int pr_chmod(const char *path, mode_t mode)
{
	int res;
	res = chmod(path, mode);
	if (res == -1)
		return -errno;
	return 0;
}
//--change owner of a file-----------------------
static int pr_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;
	return 0;
}
static struct fuse_operations pr_operations = {
	.getattr		= pr_getattr,
	.access			= pr_access,
	.readlink		= pr_readlink,
	.readdir		= pr_readdir,
	.mkdir			= pr_mkdir,
	.mknod			= pr_mknod,
	.rmdir			= pr_rmdir,
	.read			= pr_read,
	.write			= pr_write,
	.statfs			= pr_statfs,
	.rename			= pr_rename,
	.symlink		= pr_symlink,
	.link			= pr_link,
	.unlink			= pr_unlink,
	.truncate		= pr_truncate,
	.utimens		= pr_utimens,
	.open 			= pr_open,
	.fsync 			= pr_fsync,
	.chmod			= pr_chmod,
	.chown			= pr_chown,
	.release		= pr_release,
	};	
int main(int argc,char* argv[])
{
	DIR* dp;
	int ret;	
	umask(0);
	struct fuse_args args;
	init_gpgme(GPGME_PROTOCOL_OpenPGP);
	err = gpgme_new(&ctx);
	check_errors(err,"error with ctx");
	ret = fuse_main(argc,argv,&pr_operations,NULL);;
	return ret;
}
