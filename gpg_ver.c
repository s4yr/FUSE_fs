/*
 * "./%e" --openpgp /home/s4yr/signature.c.sig /home/s4yr/signature.c
 */
#include "gpg_support.h"
#define PGM "gpg_ver.c"
bool verify_signature(gpgme_ctx_t ctx,char* sig_path,char* file_path);	//verificat a detach signature
/*
int main(int argc, char** argv)
{
	gpgme_ctx_t ctx;
	gpgme_error_t err;
	init_gpgme(GPGME_PROTOCOL_OpenPGP);
	err = gpgme_new(&ctx);
	check_errors(err,"error with gpgme_new");
	verify_signature(ctx,"signature.c.sig","signature.c");
	
	return 0;
}
*/
bool verify_signature(gpgme_ctx_t ctx, char* sig_path,char* file_path)
{
	gpgme_error_t err;
	FILE* fp_sig = NULL;
	FILE* fp_msg = NULL;
	gpgme_data_t sig = NULL;
	gpgme_data_t msg = NULL;
	gpgme_verify_result_t result;
	fp_sig = fopen(sig_path,"rb");	//open file with signature
	if(!fp_sig)
	{
		err = gpg_error_from_syserror();
		fprintf (stderr, PGM ": can't open `%s': %s\n", sig_path, gpg_strerror(err));
		return false;
	}
	fp_msg = fopen(file_path,"rb");	//open file which want to check
	if(!fp_msg)
	{
		err = gpg_error_from_syserror();
		fprintf(stderr,PGM ": cant open `%s`: %s\n", file_path, gpg_strerror(err));
		return false;
	}
	err = gpgme_data_new_from_stream(&sig,fp_sig);	//write to gpgme_data object from fp_sig stream(signature)
	if(err)
	{
		fprintf (stderr, PGM ": error allocating data object: %s\n",gpg_strerror (err));
		return false;
	}
	if(fp_msg)
	{
		err = gpgme_data_new_from_stream(&msg,fp_msg);	//write to gpgme_data object from fp_msg(file)
		if(err)
		{
			fprintf(stderr,PGM ": error allocation data object: %s\n",gpg_strerror(err));
			return false;
		}
	}
	err = gpgme_op_verify(ctx,sig,msg,NULL);	//verify signature
	check_errors(err,"error with verify");
	result = gpgme_op_verify_result(ctx);		//get result of checking signature
	if(result)
	{
		print_result_of_checking(result);		//printf checking information
	}
	fclose(fp_sig);
	fclose(fp_msg);
	gpgme_data_release (msg);
	gpgme_data_release (sig);		
	return true;
}
