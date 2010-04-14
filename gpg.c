#include <stdio.h>
#include <gpgme.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h> 
#define check_errors(err,str) if(err != GPG_ERR_NO_ERROR){puts(str);return -1;}
#define SIZE 1024
static int progress_called;
static void progress (void *self, const char *what, int type, int current, int total)
{
  if (!strcmp (what, "primegen") && !current && !total
      && (type == '.' || type == '+' || type == '!'
	  || type == '^' || type == '<' || type == '>'))
    {
      printf ("%c", type);
      fflush (stdout);
      progress_called = 1;
    }
  else
    {
      fprintf (stderr, "unknown progress `%s' %d %d %d\n", what, type,
	       current, total);
      exit (1);
    }
}
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

int main(int argc, char** argv)
{
	//system("ls -l");
	gpgme_ctx_t ctx;
	gpgme_ctx_t ctx_gen;
	gpgme_ctx_t ctx_gen2;
	gpgme_error_t err;
	gpgme_data_t data,in,out;
	gpgme_key_sig_t key_one;
	gpgme_key_t key[3] = { NULL, NULL, NULL };
	int tmp;
	char* p;
	char buf[SIZE];
	size_t read_bytes;
	const char *parms = "<GnupgKeyParms format=\"internal\">\n"
    "Key-Type: DSA\n"
    "Key-Length: 1024\n"
    "Subkey-Type: ELG-E\n"
    "Subkey-Length: 1024\n"
    "Name-Real: Joe Tester\n"
    "Name-Comment: (pp=abc)\n"
    "Name-Email: joe@foo.bar\n"
    "Expire-Date: 0\n"
    "Passphrase: abc\n"
    "</GnupgKeyParms>\n";
	gpgme_genkey_result_t result;
	init_gpgme(GPGME_PROTOCOL_OpenPGP);
	err = gpgme_new(&ctx);
	if(err != GPG_ERR_NO_ERROR)
	{
		puts("error with GPG context");
		return -1;
	}
	gpgme_set_armor (ctx, 1);
	err = gpgme_set_protocol(ctx,GPGME_PROTOCOL_OpenPGP);	//set crypto protocol in current engine
	check_errors(err,"error with protocol");
	err = gpgme_data_new(&data);
	check_errors(err,"error with data_new");
	//set encoding for data buffer------
	err = gpgme_data_set_encoding(data,GPGME_DATA_ENCODING_ARMOR);
	check_errors(err,"error with encoding");
	// check encoding-----------------
	tmp = gpgme_data_get_encoding(data);
	if(tmp == GPGME_DATA_ENCODING_ARMOR)
		printf("encode ok\n");
	else
      printf("encode broken\n");
	err = gpgme_op_export(ctx,NULL,0,data);
	check_errors(err,"error with key exporting");
	read_bytes = gpgme_data_seek(data,0,SEEK_END);
	printf("end: %d \n",read_bytes);
	if(read_bytes == -1)
	{
		p = (char*) gpgme_strerror(errno);
		printf("data seek error %s \n",p);
		return -1;
	}
	read_bytes = gpgme_data_seek(data,0,SEEK_SET);	
	printf("start %d\n",read_bytes);
	//while((read_bytes = gpgme_data_read(data,buf,SIZE)) > 0)	//read content of gpgme_data object
	{
	//	write(2,buf,read_bytes);
	}
	//write(2,"\n",1);
 //  gpgme_data_release(data);
//	gpgme_release(ctx);
//======================================================================
	err = gpgme_data_new_from_mem (&in, "Hallo Leute\n", 12, 0);		//fill gpgme_data	by string
	check_errors(err,"error in data_new_from_mem");		
	err = gpgme_data_new(&out);		//create empty gpgme_data_object
	gpgme_ctx_t ctx_new;
	gpgme_new(&ctx_new);
	err = gpgme_new(&ctx_gen2);
	check_errors(err,"error with ctx_gen");
	
//	gpgme_set_progress_cb (ctx_gen2, progress, NULL);	//set function display while key generat--background
//	err = gpgme_op_genkey (ctx_gen2, parms, NULL, NULL);	//generate key
//	check_errors(err,"error with genkey");
	
	err = gpgme_op_export(ctx,NULL,0,data);
	read_bytes = gpgme_data_seek(data,0,SEEK_SET);	
	printf("start %d\n",read_bytes);
	while((read_bytes = gpgme_data_read(data,buf,SIZE)) > 0)	//read content of gpgme_data object
	{
		write(2,buf,read_bytes);
	}
	write(2,"\n",1);
	
	result = gpgme_op_genkey_result(ctx_gen2);	//get result of generated key
	if(!result)
	{
		fprintf (stderr, "%s:%d: gpgme_op_genkey_result returns NULL\n",__FILE__, __LINE__);
		gpgme_release (ctx_gen2);
		exit(1);
	}
	if (progress_called)
		printf ("\n");
		
	printf ("Generated key: %s (%s)\n", result->fpr ? result->fpr : "none",
	  result->primary ? (result->sub ? "primary, sub" : "primary")
	  : (result->sub ? "sub" : "none"));

	if (result->fpr && strlen (result->fpr) != 40)
    {
      fprintf (stderr, "%s:%d: generated key has unexpected fingerprint\n",__FILE__, __LINE__);
	  gpgme_release (ctx_gen2);
      exit (1);
    }
	if (!result->primary)
    {
      fprintf (stderr, "%s:%d: primary key was not generated\n",__FILE__, __LINE__);
	  gpgme_release (ctx_gen2);
      exit (1);
    }
	if (!result->sub)
    {
      fprintf (stderr, "%s:%d: sub key was not generated\n",__FILE__, __LINE__);
	  gpgme_release (ctx_gen2);
      exit (1);
    }
	gpgme_release (ctx_gen2);

	return 0;
}
