#include "queryobj.h"

QueryObj::QueryObj(std::string _tbl) {
    tableName = _tbl;
    usedTables.insert(tableName);
    type = Identity_;
}

QueryObj QueryObj::Select(bool (*callback)(int, char*, int)) {
    QueryObj newQuery;
    newQuery.type = SelectQ_;
    newQuery.callback = callback;
    newQuery.left = this;
    newQuery.usedTables = this->usedTables;
    return newQuery;
}

QueryObj QueryObj::Select(int indexNo, int op, std::vector<void*> values) {
    QueryObj newQuery;
    newQuery.type = SelectO_;
    newQuery.op = op;
    newQuery.values = values;
    newQuery.left = this;
    newQuery.usedTables = this->usedTables;
    newQuery.indexNo = indexNo;
    return newQuery;
}
QueryObj QueryObj::Project(std::vector<int> cols) {
    QueryObj newQuery;
    newQuery.type = Project_;
    newQuery.cols1 = cols;
    newQuery.left = this;
    newQuery.usedTables = this->usedTables;
    return newQuery;
}
QueryObj Join(QueryObj *q1, QueryObj *q2, std::vector<int> cols1, std::vector<int> cols2) {
    QueryObj newQuery;
    newQuery.type = Join_;
    newQuery.cols1 = cols1;
    newQuery.cols2 = cols2;
    newQuery.left = q1;
    newQuery.right = q2;
    newQuery.usedTables = q1->usedTables;
    newQuery.usedTables.insert(q2->usedTables.begin(), q2->usedTables.end());
    return newQuery;
}
QueryObj Union(QueryObj *q1, QueryObj *q2) {
    QueryObj newQuery;
    newQuery.type = Union_;
    newQuery.left = q1;
    newQuery.right = q2;
    newQuery.usedTables = q1->usedTables;
    newQuery.usedTables.insert(q2->usedTables.begin(), q2->usedTables.end());
    return newQuery;
}
QueryObj Intersection(QueryObj *q1, QueryObj *q2) {
    QueryObj newQuery;
    newQuery.type = Intersection_;
    newQuery.left = q1;
    newQuery.right = q2;
    newQuery.usedTables = q1->usedTables;
    newQuery.usedTables.insert(q2->usedTables.begin(), q2->usedTables.end());
    return newQuery;
}

QueryObj::QueryObj() {
    
}