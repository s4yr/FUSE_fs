
#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fuse.h>
#include <fcntl.h>
#include <stddef.h>
#include <fuse_opt.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
char name1[10] = "/file";	
char name2[10];
static int savefd;
char* intToChar(char* bufer, int numb);
static struct fuse_session* se;
struct mbr_data
{
  int fd;
  uint8_t ro;
  char *filename;
 // struct mbr_partition *primary;
};
static int pr_getattr(const char* path,struct stat* st_buf)		//get attributes about file/directory
{
	int res = 0;	//temporary result
	char* tmp_path;
	int lenght = strlen(path) - 1;
	memset(st_buf,0,sizeof(struct stat));
	tmp_path = (char*)calloc(strlen(path)+1,sizeof(char));
	//path = fixpath(path);
	res = stat(path,st_buf);
	fprintf(2,"saasd12");
	return res;


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
static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}
static inline DIR *get_dirp(struct fuse_file_info *fi)		//return directory pointer
{
    return (DIR *) (uintptr_t) fi->fh;
}
int pr_opendir(const char *path, struct fuse_file_info *fi)
{
	DIR* dp;
	//path = fixpath(path);
	dp = opendir(path);
	if(dp = NULL)
	{
		printf("errr");
		return -errno;
	}
	fi->fh = (unsigned long) dp;
	return 0;		
}
static int pr_readdir(const char* path, void* buf, fuse_fill_dir_t filler,off_t offset,struct fuse_file_info* fi)
{
	int res = 0;
	struct stat st;
	struct dirent* de;
	DIR* dp = get_dirp(fi);
	(void)path;
	seekdir(dp,offset);
	while((de = readdir(dp)) != NULL)
	{
		struct stat st;
		memset(&st,0,sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if(filler(buf,de->d_name,&st,telldir(dp)))
				break;
	}
	return res;
}
static struct fuse_operations pr_operations = {
	.getattr 	= pr_getattr,
	.readlink	= xmp_readlink,
	//.fgetattr	= pr_fgetattr,
	//.readdir	= pr_readdir, 
	//.open		= pr_opendir,
	//.access		= pr_access,
	};	
int main(int argc,char* argv[])
{
	DIR* dp;
	int ret;	
	umask(0);
	struct fuse_args args;
	//args = (const struct fuse_args) FUSE_ARGS_INIT (argc,argv);
	//se = fuse_lowlevel_new (&args,&pr_operations,sizeof(pr_operations),NULL);
	ret = fuse_main(argc,argv,&pr_operations,NULL);
	printf("%d",ret);
	return ret;
}
char* intToChar(char* bufer, int numb)
{
	char tmp[20];
	int buf;
	int lenght = 0;
	int i = 0;
	int j;
	if(numb >= 10)
	{
		do
		{
			buf = numb%10;
			tmp[i] = buf + '0';
			i++;
			numb /=10;		
		} while (numb >= 10);
		tmp[i] = numb + '0';
	}else
	{
		tmp[i] = numb + '0';
	};
	lenght = i;
	for (j = 0; j <= lenght; j++)
	{
		bufer[j] = tmp[i--];
	}
	bufer[j] = NULL;
	return bufer;	
}
