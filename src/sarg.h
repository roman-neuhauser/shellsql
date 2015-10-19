/*
 * SQL parse stuff
 *
 */

struct sqlarg_t {
	char **arg;
	int num;
};

typedef struct sqlarg_t sqlarg;

sqlarg *new_sqlarg(char **sqlnames, char *inputstr, int numb);
void sqlarg_delete(sqlarg *s);
char *sqlarg_get(sqlarg *s, int index);

/*
 * A quick look at the element, no validation
 */

#define sqlargi(sarg, index) sarg->arg[index]


