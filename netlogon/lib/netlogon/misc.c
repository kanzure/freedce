#include <stdio.h>
#include <string.h>
#include <dce/rpc.h>
#include <dce/dce_error.h>
#include <ctype.h>

void 
nl_chk_dce_err(ecode, where, why, fatal)
     error_status_t ecode;
     char * where;
     char * why;
     unsigned int fatal;
{

  dce_error_string_t errstr;
  int error_status;                           
  
  if (ecode != error_status_ok)
    {
       dce_error_inq_text(ecode, errstr, &error_status); 
       if (error_status == error_status_ok)
	 printf("ERROR.  where = <%s> why = <%s> error code = 0x%lx"
		"reason = <%s>\n",
	      where, why, ecode, errstr);
       else
	 printf("ERROR.  where = <%s> why = <%s> error code = 0x%lx\n",
	      where, why, ecode);
       
       if (fatal) exit(1);
    }
}

void nl_print_asc(unsigned char  const *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		printf("%c", isprint(buf[i]) ? buf[i] : '.');
	}
}

void nl_dump_data(const char *buf1, int len)
{
	unsigned char const *buf = (unsigned char const *)buf1;
	int i = 0;

	if (buf == NULL)
	{
		printf("dump_data: NULL, len=%d\n", len);
		return;
	}
	if (len < 0)
		return;
	if (len == 0)
	{
		printf("\n");
		return;
	}

	printf("[%03X] ", i);
	for (i = 0; i < len;)
	{
		printf("%02X ", (int)buf[i]);
		i++;
		if (i % 8 == 0)
			printf(" ");
		if (i % 16 == 0)
		{
			nl_print_asc(&buf[i - 16], 8);
			printf(" ");
			nl_print_asc(&buf[i - 8], 8);
			printf("\n");
			if (i < len)
				printf("[%03X] ", i);
		}
	}

	if (i % 16 != 0)	/* finish off a non-16-char-length row */
	{
		int n;

		n = 16 - (i % 16);
		printf(" ");
		if (n > 8)
			printf(" ");
		while (n--)
			printf("   ");

		n = i % 16;
		if (n > 8)
			n = 8;
		nl_print_asc(&buf[i - (i % 16)], n);
		printf(" ");
		n = (i % 16) - n;
		if (n > 0)
			nl_print_asc(&buf[i - n], n);
		printf("\n");
	}
}

