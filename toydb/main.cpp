#include <bits/stdc++.h>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "schema.h"
#include "table.h"
#include "db.h"
#include "user.h"

#define checkerr(err, message) {if (err < 0) {PF_PrintError(message); exit(1);}}

#define MAX_PAGE_SIZE 4000
#define MAX_TOKENS 100
#define MAX_LINE_LEN   1000

#define DB_NAME "data.db"
#define INDEX_NAME "data.db.0.idx"
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


Schema_ *
loadCSV() {
    // Open csv file, parse schema
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
    Table tbl(&s,DB_NAME,"check",true,vector<IndexData>());
    cout<<"Table created"<<endl;
    cout<<"----------------------------------------------------------------"<<endl;
    tbl.createIndex(std::vector(1,2));
    cout<<"Index created"<<endl;
    cout<<"----------------------------------------------------------------"<<endl;
    // char c[] = "Taiwan";
    // tbl.queryIndex(0,GREATER_THAN_EQUAL, vector<void*>(1,(void*)c));
    // cout<<"Query done"<<endl;
    // checkerr(Table_Open(DB_NAME, sch, true, &tbl), "Loadcsv : table open");
    // AM_DestroyIndex(DB_NAME, 0);
    // assert(AM_CreateIndex(DB_NAME, 0, 'i', 4) == AME_OK);
    // int index_fileDesc = PF_OpenFile(INDEX_NAME);
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
    cout<<"Get Rows"<<endl;
    char* tokens2[1];
    tokens2[0] = new char[100];
    tokens2[0] = "Zimbabwe"; 
    char** data= (char**)tbl.getRow((void**)tokens2);
    for(int i=0; i<sch->numColumns; i++) {
        switch(sch->columns[i].type) {
            case INT:
                printf("%d\t", DecodeInt(data[i]));
                break;
            case FLOAT:
                printf("%f\t", DecodeFloat(data[i]));
                break;
            case LONG:
                printf("%lld\t", DecodeLong(data[i]));
                break;
            case VARCHAR:
                printf("%s\t", (char*)data[i]);
                break;
            default:
                printf("%s\t", (char*)data[i]);
                break;
        }
    }
    cout<<"\n----------------------------------------------------------------"<<endl;
    cout<<"Delete Row"<<endl;
    cout<<tbl.deleteRow((void**)tokens2, true)<<' ';
    cout<<"deleted"<<endl;
    tbl.print();
    
    char name[] = "Zimbabwase";
    char capital[] = "Harare";
    char pop[] = "16529904";
    void *data2[3] = {(void*)name, (void*)capital, (void*)pop};
    tbl.addRow((void**)data2,false, true);

    tbl.print();
    cout<<"\n----------------------------------------------------------------"<<endl;
    cout<<"Rolback"<<endl;
    tbl.rollback();
    tbl.print();
    cout<<"\n----------------------------------------------------------------"<<endl;
    cout << "Getting Records" << endl;
    std::vector<std::pair<int, void**> > res = tbl.get_records();
    for(auto rec: res) {
        for(int i=0; i<sch->numColumns; i++) {
            switch(sch->columns[i].type) {
                case INT:
                    printf("%d\t", DecodeInt((char*)rec.second[i]));
                    break;
                case FLOAT:
                    printf("%f\t", DecodeFloat((char*)rec.second[i]));
                    break;
                case LONG:
                    printf("%lld\t", DecodeLong((char*)rec.second[i]));
                    break;
                case VARCHAR:
                    printf("%s\t", (char*)rec.second[i]);
                    break;
                default:
                    printf("%s\t", (char*)rec.second[i]);
                    break;
            }
        }
        cout << endl;
    }


    char c[] = "Yemen";
    int i = 100000;
    Table* out = tbl.queryIndex(0,GREATER_THAN_EQUAL, vector<void*>(1,(void*)&i));
    out->print();
    cout<<"Query done"<<endl;

    cout<<"----------------------------------------------------------------"<<endl;
    cout<<"Encode Decode"<<endl;
    string str = tbl.encodeTable();
    Table tbl2 = decodeTable((char*)str.c_str(), str.size());
    tbl2.print();
    tbl2.close();
    cout<<"----------------------------------------------------------------"<<endl;    
    tbl.eraseIndex(2);
    cout<<"Erased Index"<<endl;
    cout<<"----------------------------------------------------------------"<<endl;    
    tbl.close();
    cout<<"Table Closed"<<endl;
    cout<<"----------------------------------------------------------------"<<endl;    
    // fclose(fp);
    // Table_Close(tbl);
    // checkerr(PF_CloseFile(index_fileDesc), "Loadcsv : close file");
    return sch;
}


int main(){
    loadCSV();

    // ONLY RUN THESE THE FIRST TIME TO CREATE SUPERUSER
    bool success = createUserDb();
    createPrivilegeTable();
    createPrivilegeDb();

    User u("SUPERUSER", "SUPERUSER_PASSWORD");

    u.addUser("weakling", "boi");
    User u2("weakling", "boi");
    u.assignPerm(u2, "MAIN_DB", 1);
    u.assignPerm(u2, "MAIN_DB", "MAIN_TABLE", 2);

    Database db(&u);
    db.create("MAIN_DB");
    db.create("TEMP_DB");
    db.drop();
    db.connect("DB");

    std::string db_name = "MAIN_DB";
    vector<string> table_name = {"MAIN_TABLE", "TEMP_TABLE_2"}; 
    Table* tables[2];
    for(int i =0; i<2; i++){
        std::vector<std::pair<std::string, int> > cols;
        cols.push_back({"KEY", VARCHAR});
        cols.push_back({"VALUE", VARCHAR});  
        std::vector<int> pk = {0};

        char* table_cstr = new char[table_name[i].size()+1];
        memcpy(table_cstr, table_name[i].c_str(), table_name[i].size()+1);

        char* db_cstr = new char[db_name.size()+1];
        memcpy(db_cstr, db_name.c_str(), db_name.size()+1);

        Schema *schema = new Schema(cols, pk);
        vector<IndexData> vi;
        tables[i] = new Table(schema, table_cstr, db_cstr, false, vi);
        db.createTable(tables[i]);
        delete table_cstr, db_cstr;
    }

    cout<<"\n----------------------------------------------------------------"<<endl;
    cout << "Table and Db Checking" << endl;
    cout << "MAIN_DB Exists? " << isDb("MAIN_DB") << endl;
    cout << "MAIN_TABLE Exists? " << isTable("MAIN_DB", "MAIN_TABLE") << endl;
    cout << "TEMP_DB Exists? " << isDb("TEMP_DB") << endl;
    cout << "TEMP_TABLE Exists? " << isTable("MAIN_DB", "TEMP_TABLE") << endl;

    Database db1(&u2);

    cout<<"\n----------------------------------------------------------------"<<endl;
    cout << "User permission validity" << endl;
    cout << "MAIN_DB Allowed to Read? " << db1.isAllowed("MAIN_DB", 1) << endl;
    cout << "MAIN_DB Allowed to Write? " << db1.isAllowed("MAIN_DB", 2) << endl;
    cout << "MAIN_TABLE Allowed to Read? " << db1.isAllowed("MAIN_DB", "MAIN_TABLE", 1) << endl;
    cout << "MAIN_TABLE Allowed to Write? " << db1.isAllowed("MAIN_DB", "MAIN_TABLE", 2) << endl;
    cout << "TEMP_TABLE_1 Allowed to Write? " << db1.isAllowed("MAIN_DB", "TEMP_TABLE_1", 2) << endl; 
    cout<<"\n----------------------------------------------------------------"<<endl;

    cout << "Deleting Now" << endl;
    db.deleteTable(tables[0]);
    tables[0]->close();
    tables[1]->close();

    return 0;
}