#include "gpg_support.h"
#define SIZE 1024
static void check_result (gpgme_sign_result_t result, gpgme_sig_mode_t type)
{
	if (result->invalid_signers)
    {
      fprintf (stderr, "Invalid signer found: %s\n",result->invalid_signers->fpr);
      exit (1);
    }
	if (!result->signatures || result->signatures->next)
    {
      fprintf (stderr, "Unexpected number of signatures created\n");
      exit (1);
    }
	if (result->signatures->type != type)
    {
      fprintf (stderr, "Wrong type of signature created\n");
      exit (1);
    }
	if (result->signatures->pubkey_algo != GPGME_PK_DSA)
    {
      fprintf (stderr, "Wrong pubkey algorithm reported: %i\n",result->signatures->pubkey_algo);
      exit (1);
    }
	if (result->signatures->hash_algo != GPGME_MD_SHA1)
    {
      fprintf (stderr, "Wrong hash algorithm reported: %i\n",result->signatures->hash_algo);
      exit (1);
    }
	if (result->signatures->sig_class != 1)
    {
      fprintf (stderr, "Wrong signature class reported: %u\n",result->signatures->sig_class);
      exit (1);
    }
	if (strcmp ("52535FB1AB350C91FD42EB5AF3BCD7EFE08CDAEE",result->signatures->fpr))
    {
      fprintf (stderr, "Wrong fingerprint reported: %s\n",result->signatures->fpr);
      exit (1);
    }
}
int main(int argc, char** argv)
{
	gpgme_ctx_t ctx;
	gpgme_error_t err;
	gpgme_data_t in, out;
	gpgme_sign_result_t result;
	char *agent_info;
	init_gpgme(GPGME_PROTOCOL_OpenPGP);		//inicialization of GPG lib
	
	err = gpgme_new(&ctx);	//create new gpg context
	check_errors(err,"error with gpgme_new");
	agent_info = getenv ("GPG_AGENT_INFO");		//get enviroment variable for gpg agent
	if (!(agent_info && strchr (agent_info, ':')))
	{
		puts("problems with gpg-agent");
	}
	//make readeble
	gpgme_set_textmode (ctx, 1);
	gpgme_set_armor (ctx, 1);
	
//------------------------
#if 0
{
	gpgme_key_t akey;
    err = gpgme_get_key (ctx, "E08CDAEE", &akey, 0);	//fingerprintf of public key(last 8 simbols)who create a signature
	check_errors(err,"error with key");
	err = gpgme_signers_add (ctx, akey);	//add akey to list of signers
	check_errors(err,"error with gpgme_signers");
}	
#endif
//----------------------------------------------------
	err = gpgme_data_new_from_mem(&in,"Hallo s4yrr\n", 12, 0);
	check_errors(err,"error with gpgme_data_new_from_mem");
	
	err = gpgme_data_new (&out);
	check_errors(err,"error data_new");
	err = gpgme_op_sign (ctx, in, out, GPGME_SIG_MODE_NORMAL); //create signature for text in IN and return it in OUT
	check_errors(err,"error with gpg_op_sig");
	result = gpgme_op_sign_result (ctx);	//return result of signature
	check_result (result, GPGME_SIG_MODE_NORMAL);	//check result of signature
	print_data (out);		//printf the context of gpgme_data object
	gpgme_data_release (out);	//destroy gpgme_data object
	
//--detached------------------------------------------
	puts("==========DETACHED SIGNATURE==================================");
	gpgme_data_seek (in, 0, SEEK_SET);	//in begin of pgpme_object
	err = gpgme_data_new(&out);			//create a new data object
	check_errors(err,"error with data_new2");
	err = gpgme_op_sign (ctx, in, out, GPGME_SIG_MODE_DETACH); //create a separate signature
	check_errors(err,"error with detach");
	result = gpgme_op_sign_result (ctx);
	check_result (result, GPGME_SIG_MODE_DETACH);
	print_data (out);
	gpgme_data_release (out);
	
//--cleartext signature-------------------------------
	puts("==========CLEAR SIGNATURE=====================================");
	gpgme_data_seek (in, 0, SEEK_SET);	//in begin of object
	err = gpgme_data_new (&out);
	check_errors(err,"error with data_new3");
	err = gpgme_op_sign (ctx, in, out, GPGME_SIG_MODE_CLEAR);	//creating clear signature
	check_errors(err,"error with clear signature");
	result = gpgme_op_sign_result (ctx);
	check_result (result, GPGME_SIG_MODE_CLEAR);
	print_data (out);
	gpgme_data_release (out);

	gpgme_data_release (in);
	gpgme_release (ctx);
	return 0;
}






