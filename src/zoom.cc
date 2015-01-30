#include <nan.h>
#include "query.h"
#include "record.h"
#include "options.h"
#include "resultset.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
    node_zoom::Query::Init(exports);
    node_zoom::Options::Init(exports);

    node_zoom::Record::Init();
    node_zoom::ResultSet::Init();
}

NODE_MODULE(zoom, InitAll);
