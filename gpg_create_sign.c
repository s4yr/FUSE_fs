/*
	"./%e" --openpgp  /home/s4yr/signature.c
 */
#define PGM "gpg_create_sign"
#include "gpg_support.h"
#define BUF_SIZE 512
bool create_signature(gpgme_ctx_t ctx,char* file_path,char* key_string);
/*
int main(int argc, char** argv)
{
	gpgme_error_t err;
	gpgme_ctx_t ctx;
	init_gpgme(GPGME_PROTOCOL_OpenPGP);
	err = gpgme_new(&ctx);
	check_errors(err,"error with gpgme_new");
	create_signature(ctx,"signature.c",NULL);
	
	return 0;
}
*/
bool create_signature(gpgme_ctx_t ctx,char* file_path,char* key_string)
{
	gpgme_error_t err;
	gpgme_data_t in;
	gpgme_data_t out;
	gpgme_sign_result_t result;
	gpgme_set_armor (ctx, 1);	//set readeble
	gpgme_sig_mode_t sigmode = GPGME_SIG_MODE_DETACH;
	char buf[BUF_SIZE+1];
	char sig_path[BUF_SIZE+1];
	char* p_sig_path;
	int ret = 0;
	FILE* fp_sig;
	if(key_string)
	{
		gpgme_key_t akey;
		err = gpgme_get_key (ctx, key_string, &akey, 1);	//set keys in current context
		check_errors(err,"error with `gpgme_get_key`");
		err = gpgme_signers_add(ctx,akey);
		check_errors(err,"error with `gpgme_signers_add` in create signature");
		gpgme_key_unref (akey);
	}
	err = gpgme_data_new_from_file(&in,file_path,1);	//write in gpgme_data object IN contents of file
	check_errors(err,"error with `data_new_from_file`");
	err = gpgme_data_new(&out);							//create empty gpgme_data object for signature
	check_errors(err,"error with data object");
	err = gpgme_op_sign(ctx,in,out,sigmode);	//create NORMAL signature of IN object in OUT object
	result = gpgme_op_sign_result(ctx);					//get result of creating signature;
	if(result)
		print_result_of_creating(result,sigmode);		//printf report about creating
	check_errors(err,"error with gpgme_op_sign");
	print_data(out);
	
	p_sig_path = strcpy(sig_path,file_path);
	p_sig_path = strcat(p_sig_path,".sig");
	fp_sig = fopen(p_sig_path,"w");
	ret = gpgme_data_seek(out,0,SEEK_SET);		//set in begin of OUT object
	while((ret = gpgme_data_read(out,buf,BUF_SIZE)) > 0)
		fwrite(buf,ret,1,fp_sig);
	fclose(fp_sig);

	gpgme_data_set_file_name(out,"qwert");
	gpgme_data_release (out);
	gpgme_data_release (in);
	return true;
}
