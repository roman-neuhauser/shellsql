/*
 * Some PARSE STRINGS for SQL connect strings (maybe amoungst others)
 * sqlname is an array of pointers that have the names
 * ins is the array string input
 * num is the number of names (sqlarg and sqlname needs to be allocated accordingly)
 * tda the thread data
 *
 */

#include "sarg.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

sqlarg *new_sqlarg(char **sqlname, char *ins, int num)
{
	/*
	 * while scanning
	 * mode = 0 for name, 1 for data, 2 in quotes
	 */
	int mode = 0;

	/*
	 * The fld we are dealing with, -1 is none
	 */
	int fld = -1;
	int fldtest;	/* Boolean for efficiency */

	/*
	 * Max of 32 for name, 1024 for data
	 */
	char name[32];
	char data[1024];
	int lname = 0;
	int ldata = 0;

	int i;

	sqlarg *e_sarg;
	char **sarg;

	*name = 0;
	*data = 0;

	if((e_sarg = (sqlarg *)malloc(sizeof(sqlarg))) == NULL) 
	{
		return NULL;
	}

	if(num < 0) num = 0;

	e_sarg->num = num;

	if(!num)
	{
		e_sarg->arg = NULL;
		return e_sarg;
	}
	if((sarg = (char **)malloc(sizeof(char *) * num)) == NULL)
	{
		free(e_sarg);
		return NULL;
	}

	e_sarg->arg = sarg;

	for(i=0;i<num;i++) sarg[i] = NULL;

	for(;;ins++)
	{
		if(mode == 0)
		{
			if(!(*ins)) break;
			if(*ins <= ' ') continue;
			if(*ins == ';')
			{
				lname = 0;
				*name = 0;
				continue;
			}
			if(*ins == '=') 
			{
				fld = -1;
				fldtest = 0;
				for(i=0;i<num;i++)
				{
					if(!strcmp(name, sqlname[i]))
					{
						fld = i;
						fldtest = -1;
						break;
					}
				}

				*name= 0;
				lname = 0;

				mode = 1;

				ins++;
				while((*ins <= ' ') && (*ins)) ins++;
				ins--;
				continue;
			}
			if(lname >= 31) continue;
			name[lname] = tolower(*ins);
			lname++;
			name[lname] = 0;
		}
		else if(mode >= 1)
		{
			if(!(*ins)) mode = 1;

			if((*ins <= ' '  || *ins == ';') && mode == 1)
			{
				if(fldtest)
				{
					if(sarg[fld] == NULL)
					{
						if((sarg[fld] = strdup(data)) == NULL)
						{
						
							sqlarg_delete(e_sarg);
							return NULL;
						}
					}
					*data = 0;
					ldata = 0;
				}
				mode = 0;
				if(!(*ins)) break;
				continue;
			}
			if(*ins == '\'')
			{
				if(*(ins + 1) == '\'')
					ins++;
				else
				{
					if(mode == 1)
						mode = 2;
					else
						mode = 1;
					continue;
				}
			}
			if(ldata >= 1023) continue;
			data[ldata] = *ins;
			ldata++;
			data[ldata] = 0;
		}
	}
	return e_sarg;
}


void sqlarg_delete(sqlarg *sarg)
{
	int i, num;
	char **p;

	p = sarg->arg;
	num = sarg->num;

	for(i=0;i<num;i++)
	{
		if(*p != NULL)
		{
			free(*p);
			*p = NULL;
			p++;
		}
	}
	if(num) free(sarg->arg);
	free(sarg);
}

char *sqlarg_get(sqlarg *sarg, int index)
{
	if(index < 0 || index >= sarg->num)
		return NULL;
	return sarg->arg[index];
}
	

