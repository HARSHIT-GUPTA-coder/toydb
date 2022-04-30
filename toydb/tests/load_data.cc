#include <bits/stdc++.h>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "../schema.h"
#include "../table.h"
#include "../db.h"
#include "../user.h"

#define checkerr(err, message) {if (err < 0) {PF_PrintError(message); exit(1);}}

#define MAX_PAGE_SIZE 4000
#define MAX_TOKENS 100
#define MAX_LINE_LEN   1000

#define DB_NAME "SCUSTOM_SECOND_DB"
#define INDEX_NAME "FIRST_DB.0.idx"
#define CSV_NAME "./dblayer/data.csv"

using namespace std;

int
stricmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

char *trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}


/**
  split takes a character buf, and uses strtok to populate
  an array of token*'s. 
*/
int 
split(char *buf, char *delim, char **tokens) {
    char *token = strtok(buf, delim);
    int n = 0;
    while(token) {
	tokens[n] = trim(token);
	token = strtok(NULL, delim);
	n++;
    }
    return n;
}

Schema_ *
parseSchema(char *buf) {
    buf = strdup(buf);
    char *tokens[MAX_TOKENS];
    int n = split(buf, ",", tokens);
    Schema_ *sch = (Schema_*)malloc(sizeof(Schema_));
    sch->columns = (ColumnDesc*)malloc(n * sizeof(ColumnDesc));
    // strtok is terrible; it depends on global state.
    // Do one split based on ',".
    // Could use strtok_s for this use case
    char *descTokens[MAX_TOKENS];
    sch->numColumns = n;
    for (int i = 0; i < n; i++) {
	int c = split(tokens[i],":", descTokens);
	assert(c == 2);
	ColumnDesc *cd = (ColumnDesc *) malloc(sizeof(ColumnDesc));
	cd->name = strdup(descTokens[0]);
	char *type = descTokens[1];
	int itype = 0;
	if (stricmp(type, "varchar") == 0) {
	    itype = VARCHAR;
	} else if (stricmp(type, "int") == 0) {
	    itype = INT;
	} else if (stricmp(type, "long") == 0) {
	    itype = LONG;
	} else {
	    fprintf(stderr, "Unknown type %s \n", type);
	    exit(EXIT_FAILURE);
	}
	cd->type = itype;
	sch->columns[i] = *cd;
    }
    free(buf);
    return sch;
}

int main(){
    bool success = createUserDb();
    createPrivilegeTable();
    createPrivilegeDb();
    createDbList();
    createDbTableList();

    User u("SUPERUSER", "SUPERUSER_PASSWORD");

    Database db(&u);
    db.create(DB_NAME);

    FILE *fp = fopen(CSV_NAME, "r");
    if (!fp) {
	    perror("data.csv could not be opened");
        exit(EXIT_FAILURE);
    }

    char buf[MAX_LINE_LEN];
    char *line = fgets(buf, MAX_LINE_LEN, fp);
    if (line == NULL) {
        fprintf(stderr, "Unable to read data.csv\n");
        exit(EXIT_FAILURE);
    }

    // Open main db file
    Schema_ *sch = parseSchema(line);
    Schema s(sch, vector<int>(1,0));
    cout<<"Schema loaded"<<endl;
    Table tbl(&s,"main",DB_NAME,true,vector<IndexData>());
    Table tbl2(&s,"main2",DB_NAME,true,vector<IndexData>());
    cout<<"Tables created"<<endl;
    cout<<"----------------------------------------------------------------"<<endl;
    tbl.createIndex(std::vector(1,2));
    cout<<"Index created"<<endl;
    cout<<"----------------------------------------------------------------"<<endl;

    char *tokens[MAX_TOKENS];
    char record[MAX_PAGE_SIZE];

    cout<<"Adding rows"<<endl;
    while ((line = fgets(buf, MAX_LINE_LEN, fp)) != NULL) {
        int n = split(line, ",", tokens);
        assert (n == sch->numColumns);
        tbl.addRow((void**)tokens,true);
    }
    cout<<"----------------------------------------------------------------"<<endl;
    cout<<"Printing"<<endl;
    tbl.print();
    cout<<"----------------------------------------------------------------"<<endl;
    db.createTable(&tbl);
    db.createTable(&tbl2);

    std::string a = tbl.encodeTable();

    char* ptr = (char*)a.c_str();
    tbl.close();

    Table *b = db.load("main");
    // b->print();
    

    return 0;
}