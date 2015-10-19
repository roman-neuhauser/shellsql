/*
    SHSQL suite - SQL utility for LINUX/UNIX shell scriptiing
    Copyright (C) 2004  Edward Macnaghten

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Edward Macnaghten
    EDL Systems
    16 Brierley Walk
    Cambridge
    CB4 3NH
    UK

    eddy@edlsystems.com
 */

/*
 * FREETDS  - Uses the cs_ and ct_ libraries
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <cspublic.h>
#include <ctpublic.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>

#include "string.h"
#include "message.h"
#include "shellsql.h"
#include "sarg.h"


struct dats_t {
	CS_DATAFMT da;
	int len;
	int isnull;
	long rlen;
	char *buffer;
	

};

typedef struct dats_t dats;

CS_INT freetds_errcallback(CS_CONTEXT *ctx, CS_CLIENTMSG *msg);
char *getstrfield(dats *dat, int *tbf);
int fetchnext(CS_COMMAND *res, dats *dat, CS_INT ncol);
char *getdata(long *len, int *isnull, CS_COMMAND *res, int colnum);


#define FREETDS_SERVER 0
#define FREETDS_USER 1
#define FREETDS_PASSWORD 2
#define FREETDS_DBNAME 3
#define FREETDS_PORT 4
#define FREETDS_APPNAME 5
#define FREETDS_HOST 6

#define FREETDS_PARAMETERS 7

typedef void (*sighandler_t)(int);
void mainloop();
void sighand();
int dolines(CS_COMMAND *res, char format);
void wipe(char *s);
void parse_strings(char **sqlarg, char *ins);

/*
 * Some global items.....
 */

message *mes;

CS_CONTEXT *ctx;	
CS_CONNECTION *con;
char *freetds_errmessage = NULL;
long freetds_errnumber;

int isloop = -1;

int main(int argc, char *argv[])
{
	int errn;
	string *str;
	int complete = -1;
	int i;

	char *ws;
	char *dbname = NULL;
	char *server = NULL;

	int pass, pass1;

	CS_INT restype;

	
	char *sqlname[] = {
		"server",
		"user",
		"password",
		"dbname",
		"port",
		"appname"
		"hostname",
	};

	sqlarg *sarg;


	/*
	 * Username and password may be in the connection string
	 */

	unsigned int port = 0;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s connecstring\n", argv[0]);
		exit(1);
	}

	if((mes = new_message(0)) == NULL)
	{
		fprintf(stderr, "%s: Cannot open message queue\n", argv[0]);
		exit(1);
	}


	/*
	 * I believe a fork duplicates malloced stuff, I am in the poo if it does not :-)
	 */
	errn = fork();

	if(errn == -1)
	{
		fprintf(stderr, "%s: Cannot fork child process\n", argv[0]);
		exit(1);
	}
	else if(errn)
	{
/*
 * This is the parent - which behaves like clien
 */

		/*
		 * Initially just a status message is snt back, zero string length 
		 *
		 * Still needs a string to put it into
		 */


		str = new_string();
		
		if(message_receive(mes, str, &complete, MES_SERVER_TO_CLIENT) < 0)
		{
			fprintf(stderr, "%s: %s\n", argv[0], string_s(str));
			message_destroy(mes);
			message_delete(mes);
			string_delete(str);
			exit(1);
		}
		else
		{
			printf("%d\n",message_id(mes)) ;
			message_delete(mes);
			string_delete(str);
			exit(0);
		}
	}
/*
 * All parents have exited now
 */

/*
 * This is all a child
 */

	/*
	 * I think I need to do this here...
	 * I do not fully understand why, it just works!
	 * otherwise shell parent hangs!
	 */

	setsid();
	if(fork()) exit(0);
	close(0);
	close(1);
	close(2);

	/*
	 * We are in daemon mode now
	 */

	/*
	 * open the database
	 */

	str = new_string();

	for(i=1;i<argc;i++)
	{
		string_cat(str, argv[i]);
		if((!strncasecmp(argv[i], "password", 8))
		 || (!strncasecmp(argv[i], "user", 4)))
			wipe(argv[i]);
		if(i < argc - 1) string_cat_c(str, ' ');
	}

	sarg = new_sqlarg(sqlname, string_s(str), FREETDS_PARAMETERS);

	string_delete(str);

	if(sarg == NULL)
	{
		message_status(mes, 127, "Error allocating string parameters\n", MES_SERVER_TO_CLIENT);
		message_delete(mes);
		exit(1);
	}

	if(sqlargi(sarg, FREETDS_PORT) != NULL) port = atoi(sqlargi(sarg, FREETDS_PORT));


	if(cs_ctx_alloc(CS_VERSION, &(ctx)) != CS_SUCCEED) 
	{
		message_delete(mes);
		sqlarg_delete(sarg);
		exit(1);
	}
	if(ct_init(ctx, CS_VERSION) != CS_SUCCEED)
	{
		cs_ctx_drop(ctx);
		message_delete(mes);
		sqlarg_delete(sarg);
		exit(1);
	}


	cs_config(ctx, CS_SET, CS_MESSAGE_CB, (CS_VOID *)freetds_errcallback, CS_UNUSED, NULL);

	if(ct_con_alloc(ctx, &(con)) != CS_SUCCEED)
	{
		ct_exit(ctx, CS_FORCE_EXIT);
		cs_ctx_drop(ctx);
		message_delete(mes);
		sqlarg_delete(sarg);
		exit(1);
	}


	pass = -1;
	for(i=0;i<FREETDS_PARAMETERS;i++)
	{
		ws = sqlargi(sarg, i);

		if(ws == NULL) continue;
		if(!(*ws)) continue;

		switch(i) {
		case FREETDS_HOST:
			if(ct_con_props(con, CS_SET, CS_HOSTNAME, ws, strlen(ws), NULL) != CS_SUCCEED)
				pass = 0;
			break;
		case FREETDS_USER:
			if(ct_con_props(con, CS_SET, CS_USERNAME, ws, strlen(ws), NULL) != CS_SUCCEED)
				pass = 0;
			break;
		case FREETDS_PASSWORD:
			if(ct_con_props(con, CS_SET, CS_PASSWORD, ws, strlen(ws), NULL) != CS_SUCCEED)
				pass = 0;
			break;
		case FREETDS_SERVER:
			server = ws;
			break;
		case FREETDS_DBNAME:
			dbname = ws;
			break;
		case FREETDS_PORT:
			port = atoi(ws);
			if(port)
			{
				if(ct_con_props(con, CS_SET, CS_PASSWORD, (CS_BYTE *)&port, sizeof(int), NULL) != CS_SUCCEED)
					pass = 0;
			}
			break;
		case FREETDS_APPNAME:
			if(ct_con_props(con, CS_SET, CS_APPNAME, ws, strlen(ws), NULL) != CS_SUCCEED)
				pass = 0;
			break;
		}
		if(!pass) break;
	}

	if(server == NULL)
	{
		ct_con_drop(con);
		ct_exit(ctx, CS_FORCE_EXIT);
		cs_ctx_drop(ctx);
		message_status(mes, 127, "Server not specified\n", MES_SERVER_TO_CLIENT);
		sqlarg_delete(sarg);
		message_delete(mes);
		exit(1);
	}

	if(pass)
	{
		CS_RETCODE rrr;
		if((rrr = ct_connect(con, server, strlen(server))) != CS_SUCCEED) pass = 0;
	}
			

	if(!pass)
	{
		ct_con_drop(con);
		ct_exit(ctx, CS_FORCE_EXIT);
		cs_ctx_drop(ctx);
		message_status(mes, 127, "Error connecting to server\n", MES_SERVER_TO_CLIENT);
		sqlarg_delete(sarg);
		message_delete(mes);
		exit(1);
	}

	if(dbname != NULL)
	{
		if(*dbname)	/* I will use [ and ] for now - will change for Sybase?*/
		{
			CS_COMMAND *comm;

			pass = -1;
			pass1 = 0;

			
			if((ws = malloc(16 + strlen(dbname))) == NULL) pass = 0;
			if(pass)
			{
				strcpy(ws, "USE ");
				strcat(ws, dbname);

				if(ct_cmd_alloc(con, &comm) != CS_SUCCEED)
					pass = 0;
				else
					pass1 = -1;
			}
			if(pass)
			{
				if(ct_command(comm, CS_LANG_CMD, ws, strlen(ws), CS_UNUSED) != CS_SUCCEED)
					pass = 0;
			}
			if(pass)
			{
				if(ct_send(comm) != CS_SUCCEED)
					pass = 0;
				else
				{
					while(ct_results(comm, &restype) == CS_SUCCEED);
				}
			}

			if(pass1) ct_cmd_drop(comm);
		}
			
			
		if(!pass)
		{
			ct_con_drop(con);
			ct_exit(ctx, CS_FORCE_EXIT);
			cs_ctx_drop(ctx);
			message_status(mes, 127, "Cannot use database\n", MES_SERVER_TO_CLIENT);
			sqlarg_delete(sarg);
			message_delete(mes);
			exit(1);
		}
	}
	sqlarg_delete(sarg);

	
	/*
	 * Transmit to parent that we are hunky dory
	 */

	message_status(mes, 0, "", MES_SERVER_TO_CLIENT);

	/*
	 * We are open for business - Lets go
 	 */

	mainloop();

	/*
	 * At the end, tidy up
	 */
	ct_close(con, CS_FORCE_CLOSE);
	ct_con_drop(con);
	ct_exit(ctx, CS_FORCE_EXIT);
	cs_ctx_drop(ctx);

	message_destroy(mes);
	message_delete(mes);
	exit(0);
}

CS_INT freetds_errcallback(CS_CONTEXT *ctx, CS_CLIENTMSG *msg)
{
	freetds_errnumber = msg->msgnumber;
	if(freetds_errmessage != NULL) free(freetds_errmessage);
	freetds_errmessage = strdup(msg->msgstring);
	return CS_SUCCEED;
}

/*
 * This is the main server bit
 *
 * It receives SQL statements and executes them, breaking at EOF
 */
void mainloop()
{

	int sz;

	string *str = NULL;
	CS_COMMAND *res;

	int complete = -1;
	int pass;
	char format;
	char *sql;

	int loop;
	int type;

	CS_INT restype;
	CS_INT trows;


	str = new_string();

	while(isloop)
	{
		
		signal(SIGINT, sighand);
		signal(SIGTERM, sighand);

		complete = -1;
		sz = message_receive(mes, str, &complete, MES_CLIENT_TO_SERVER);
		
		if(sz < 0)
		{
			/*
			 * End of file 
			 */
			isloop = 0;
			break;
		}
		else if(sz <= 1) continue;

		format = *(string_s(str));
		sql = string_s(str) + 1;


		pass = -1;
		if(ct_cmd_alloc(con, &(res)) != CS_SUCCEED)
		{
			pass = 0;
			res = NULL;
		}
		else if(ct_command(res, CS_LANG_CMD, sql, strlen(sql), CS_UNUSED) != CS_SUCCEED)
			pass = 0;

		if(pass)
		{
			if(ct_send(res) != CS_SUCCEED) pass = 0;
		}

		string_clear(str);



		if(!pass)
		{
			message_status(mes, 127, "cannot execute mysql query", MES_SERVER_TO_CLIENT);
		}
		else
		{
			/*
			 * Might be multiple results here
			 */
			loop = -1;
			pass = -1;
			type = 0;
			for(;;)
			{
				switch(ct_results(res, &restype)) {
				case CS_END_RESULTS:
					complete = -1;
					loop = 0;
					break;
				case CS_FAIL:
				/* case CS_CANCELLED: - not supported for freetds */
					pass = 0;
					loop = 0;
					complete = -1;
					break;
				}
		
				if(!loop) break;
				
				switch(restype) {
				case CS_ROW_RESULT:
				case CS_PARAM_RESULT:	/* SQL select query */
					type = 1;
					if(!dolines(res, format)) pass = 0;
					break;
				case CS_STATUS_RESULT:	/* DML insert/update/delete */
				case CS_CMD_DONE:	
					type = 2;
					break;
				case CS_CMD_SUCCEED:
					type = 2;
					if(ct_res_info(res, CS_ROW_COUNT, (CS_BYTE *)&trows, sizeof(CS_INT), NULL) != CS_SUCCEED)
						pass = 0;
					break;
				case CS_CMD_FAIL:	/* Error - I will abort here */
					pass = 0;
					loop = 0;
					type = 0;
				}
		
				if(!loop) break;
			}
			if(!pass)
				message_status(mes, 127, "Error executing sql statement", MES_SERVER_TO_CLIENT);
			else
				message_send(mes, str, -1, MES_SERVER_TO_CLIENT);

		}
		if(res != NULL) ct_cmd_drop(res);
	}
	string_delete(str);
}


int dolines(CS_COMMAND *res, char format)
{
	int i;

	int rr;

	string *str;
	char *s;

	dats *dat, *odat;
	int tbf;

	CS_INT ncol = 0;


	if(ct_res_info(res, CS_NUMDATA, (CS_BYTE *)&ncol, sizeof(CS_INT), NULL) != CS_SUCCEED) return 0;

	if((odat = dat = (dats *)malloc(sizeof(dats) * ncol)) == NULL)
		return 0;

	memset(dat, 0, sizeof(dats) * ncol);

	for(i=0;i<ncol;i++,dat++)
	{
		if(ct_describe(res, (CS_INT) i + 1, &(dat->da)) != CS_SUCCEED)
		{
			free(odat);
			return 0;
		}

		switch(dat->da.datatype) {
		case CS_BINARY_TYPE:
		case CS_LONGBINARY_TYPE:
		case CS_VARBINARY_TYPE:
		case CS_CHAR_TYPE:
		case CS_VARCHAR_TYPE:
		case CS_LONGCHAR_TYPE:
			dat->len = 0;
			break;
		case CS_FLOAT_TYPE:
			dat->len = sizeof(CS_FLOAT);
			break;
		case CS_DATETIME_TYPE:
			dat->len = sizeof(CS_DATETIME);
			break;
		case CS_DATETIME4_TYPE:
			dat->len = sizeof(CS_DATETIME4);
			break;
		case CS_REAL_TYPE:
			dat->len = sizeof(CS_REAL);
			break;
		case CS_SMALLINT_TYPE:
			dat->len = sizeof(CS_SMALLINT);
			break;
		case CS_INT_TYPE:
			dat->len = sizeof(CS_INT);
			break;
		case CS_BIT_TYPE:
			dat->len = sizeof(CS_BIT);
			break;
		case CS_MONEY_TYPE:
			dat->len = sizeof(CS_MONEY);
			break;
		case CS_MONEY4_TYPE:
			dat->len = sizeof(CS_MONEY4);
			break;
		/*
		case CS_PACKED370_TYPE:
			dat->len = sizeof(CS_PACKED370);
			break;
		*/
		case CS_NUMERIC_TYPE:
			dat->len = sizeof(CS_NUMERIC);
			break;
		case CS_DECIMAL_TYPE:
			dat->len = sizeof(CS_DECIMAL);
			break;
		default:
			dat->len = 0;
			break;
		}

		dat->rlen = 0;

		if(dat->len)
		{
			if((dat->buffer = malloc(dat->len)) == NULL)
			{
				free(odat);
				return 0;
			}
		}
	}

	str = new_string();

	while((rr = fetchnext(res, odat, ncol)) == 0)
	{
		for(i=0, dat = odat;i<ncol;i++,dat++)
		{
			if((s = getstrfield(dat, &tbf)) != NULL)
			{
				dolitem(str, s, (i == ncol - 1), format);
				if(tbf) free(s);
			}
			else
				dolitem(str, "", (i == ncol - 1), format);
			if(dat->len == 0) free(dat->buffer);
			dat->rlen = 0;
		}
		string_cat_c(str, '\n');
		if(message_send(mes, str, 0, MES_SERVER_TO_CLIENT) < 0) return 0;
		string_clear(str);
	}

	for(dat=odat, i=0;i<ncol;i++, dat++)
	{
		if(dat->rlen) free(dat->buffer);
	}
	free(odat);
	string_delete(str);
	return -1;
}

/*
 * fetchnext returns -1 on error, 0 on row read, 1 on EOF
 */
int fetchnext(CS_COMMAND *res, dats *dat, CS_INT ncol)
{
	CS_INT rows_read;
	CS_RETCODE ret;
	CS_INT olen;
	int i;
	int cols;


	switch(ct_fetch(res, CS_UNUSED, CS_UNUSED, CS_UNUSED, &rows_read)) {
	case CS_SUCCEED:

		for(i=0;i<ncol;i++, dat++)
		{
			dat->isnull = 0;
			if(dat->len)
			{
				dat->rlen = 0;
				ret = ct_get_data(res, i + 1, dat->buffer, dat->len, &olen);
				if(ret != CS_END_ITEM && ret != CS_END_DATA)
				{
					/* Error - cannot read row */
					dat->rlen = 0;
					return -1;
				}
				if(!olen) dat->isnull = -1;
				dat->rlen = olen;
			}
			else
			{
				if((dat->buffer = getdata((long *)&(dat->rlen), &(dat->isnull), res, i + 1)) == NULL)
				{
					dat->rlen = 0;
					return -1;	/* Error reading */
				}
			}
		}
		return 0;
		break;
		
	case CS_END_DATA:
		return 1;
		break;
	default:
		return -1;
		break;
	}
	return 0;	/* should never get here */
}

/*
 * This gets string and other long data
 */
char *getdata(long *len, int *isnull, CS_COMMAND *res, int colnum)
{
	long plen, slen = 0;
	*isnull = 0;
	*len = 0;
	char *ws = NULL;
	CS_INT olen;
	CS_RETCODE ret;

	for(;;)
	{
		if(ws == NULL)
		{
			plen = 264;
			ws = malloc(264);	/* 8 more than 256 */
		}
		else
		{
			plen += 256;
			ws = realloc(ws, plen);
		}
		if(ws == NULL) return NULL;

		

		ret = ct_get_data(res, colnum, ws + slen, 256, &olen);

		slen += olen;
		ws[slen] = 0;

		if(ret == CS_END_ITEM || ret == CS_END_DATA)
		{
			*len = slen;
			ws[slen] = 0;
			return ws;
		}
		else if(ret != CS_SUCCEED)
		{
			free(ws);
			ws = NULL;
			*len = 0;
			return NULL;
		}
	}
	/*
	 * For zero byte stuff returned assume nulls - means there is no zero strings 
	 */
	if(!slen) *isnull = -1;
	return ws;

}


char g_null[] = "";


char *getstrfield(dats *dat, int *tbf)
{

	char *ws;
	CS_DATAFMT ddesc;
	CS_INT xlen;

	*tbf = 0;

	if(dat->isnull) return g_null;

	switch(dat->da.datatype) {
	case CS_CHAR_TYPE:
	case CS_TEXT_TYPE:
	case CS_LONGCHAR_TYPE:
		return dat->buffer;
		break;
	}

	ddesc.datatype = CS_CHAR_TYPE;
	ddesc.maxlength = dat->rlen + 32;
	ddesc.locale = NULL;
	ddesc.format = CS_FMT_NULLTERM;
	ddesc.scale = CS_SRC_VALUE;
	ddesc.precision = CS_SRC_VALUE;

	if((ws = malloc(dat->rlen + 32)) == NULL) return NULL;
	*tbf = -1;

	if(cs_convert(ctx, &(dat->da), dat->buffer, &ddesc, ws, &xlen) != CS_SUCCEED)
	{
		free(ws);
		*tbf = 0;
		return NULL;
	}
	ws[dat->rlen + 31] = 0;
	return ws;
}

			
void sighand()
{
	isloop = 0;
}


/*
 * For blanking out passwords....
 */
void wipe(char *s)
{
	while(*s)
	{
		*s = '*';
		s++;
	}
}



