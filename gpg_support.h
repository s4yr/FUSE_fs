#include <stdio.h>
#include <gpgme.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h> 
#define check_errors(err,str) if(err != GPG_ERR_NO_ERROR){puts(str);return -1;}
void init_gpgme (gpgme_protocol_t proto)
{
	gpgme_error_t err;
	gpgme_engine_info_t enginfo;
	char* p;
	setlocale (LC_ALL, "");
	p = (char*)gpgme_check_version(NULL);
	printf("version %s \n", p);
	gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
	err = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
	if(err != GPG_ERR_NO_ERROR)
	{
		puts("error with OpenPGP protocol");
		return;
	}
	p = (char*)gpgme_get_protocol_name(GPGME_PROTOCOL_OpenPGP);
	printf("protocol name: %s \n",p);
	//get engine information 
	err = gpgme_get_engine_info(&enginfo);
	if(err != GPG_ERR_NO_ERROR)
	{
		puts("error with engine info");
		return;
	}
	printf("file: %s, home: %s\n",enginfo->file_name,enginfo->home_dir);
}

void print_data (gpgme_data_t dh)
{
  #define BUF_SIZE 512
  char buf[BUF_SIZE + 1];
  int ret;
  //puts("printf_data");
  ret = gpgme_data_seek (dh, 0, SEEK_SET);
  while ((ret = gpgme_data_read (dh, buf, BUF_SIZE)) > 0)
    fwrite (buf, ret, 1, stdout);

}
