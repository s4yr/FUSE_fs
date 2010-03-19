
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
char* intToChar(char* bufer, int numb);
struct mbr_partition
{
  struct mbr_table {
    uint8_t boot;
    uint8_t start_chs[3];
    uint8_t type;
    uint8_t end_chs[3];
    uint32_t offset;
    uint32_t length;
  } table;

  off_t offset;
  off_t length;

  struct mbr_partition *next;
  struct mbr_partition *sub;

  bool mounted;
};

struct mbr_data
{
  int fd;
  uint8_t ro;
  char *filename;
 // struct mbr_partition *primary;
};
struct BLOCKS
{
	char data[256];
	struct BLOCKS* next_block;
	struct BLOCKS* prev_block;
};
struct NODES
{
	int type;
	int link_count;
	int lenght;
	size_t block_size;
	struct BLOCKS* blocks;
};
struct pr_data_record
{
	int fd;
	char filename[15];
	struct NODES* inode;
};
static int pr_getattr(const char* path,struct stat* st_buf)	//get attributes about file/directory
{
	int res = 0;	//temporary result
	struct pr_data_record rec;
	rec.fd = 123;
	bool flg_file = false;
	///bool flg_dir = false;
	char* tmp_path;
	int lenght = strlen(path) - 1;
	memset(st_buf,0,sizeof(struct stat));
	tmp_path = (char*)calloc(strlen(path)+1,sizeof(char));
	res = stat(path,st_buf);
	return res;
	//struct mbr_data* md = (struct mbr_data *) fuse_get_context ()->private_data;
	/*if(strcmp(path,"/") == 0)	//in root directory
	{
		st_buf->st_mode = S_IFDIR | 0755;
		st_buf->st_nlink = 21;
		return 0;
	}
	if(*(path + lenght) != '/')		//set as a file
	{
		while(*(path + lenght) != '/')
		{
			if(*(path +lenght) == '.')
			{
				flg_file = true;
				break;
			}
			lenght--;
		}
		if(flg_file)	//this is regular file
		{
			st_buf->st_mode = S_IFREG | 0644;
			st_buf->st_nlink = 5;
			st_buf->st_size = 13;
			return 0;
		}
		else			//directory with free access
		{
			st_buf->st_mode = S_IFDIR | 0666;
			st_buf->st_nlink = 3; 
			return 0;
		}
	}
	*/
	return res;
}
static int pr_readdir(const char* path, void* buf, fuse_fill_dir_t filler,off_t offset,struct fuse_file_info* fi)
{
	int res = 0;
	struct stat st;
	struct dirent* de;
	DIR* dp = (DIR *)(uintptr_t)fi->fh;
//	de = readdir(dp);
	
	//seekdir(dp, 0);
	int i = 0;
	char* buf_t = (char*)calloc(20,sizeof(char));
	//buf_t = intToChar(buf_t,156);							
	char* tmp = (char*)calloc(10,sizeof(char));
	strcpy(tmp,name1);
//	if(strcmp(path,"/") != 0)		
//		return -ENOENT;
	filler(buf,".",NULL,0);
	filler(buf,"..",NULL,0);
	filler(buf,path,NULL,0);
	
//	readdir
//	filler(buf,"text.c",NULL,0);
	//filler(buf,name1+1,NULL,0);
	return res;
}
int pr_opendir(const char *path, struct fuse_file_info *fi)
{
	return 0;		
}



static struct fuse_operations pr_operations = {
	.getattr 	= pr_getattr,
	.readdir	= pr_readdir, 
	.open		= pr_opendir,
	};	
	

int main(int argc,char* argv[])
{
	struct BLOCKS blk;
	DIR* dp;	
	//strcpy(name1,"/file_create");
	strcpy(name1,"/ads");	
	return fuse_main(argc,argv,&pr_operations,NULL);
	return 0;
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





