#include "errors.h"
#include "resultset.h"

using namespace v8;

namespace node_zoom {

Persistent<Function> ResultSet::constructor;

void ResultSet::Init() {
    NanScope();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
    tpl->SetClassName(NanNew("ResultSet"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    
    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "set", Set);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get", Get);

    NanAssignPersistent(constructor, tpl->GetFunction());
}

ResultSet::ResultSet(ZOOM_resultset resultset) : zset_(resultset) {}

ResultSet::~ResultSet() {
    ZOOM_resultset_destroy(zset_);
}

NAN_METHOD(ResultSet::New) {}

NAN_METHOD(ResultSet::Get) {
    ResultSet* resset = node::ObjectWrap::Unwrap<ResultSet>(args.This());

    if (args.Length() < 1) {
        NanThrowError(ArgsSizeError("Get", 1, args.Length()));
        return;
    }

    NanUtf8String key(args[0]);
    const char *value = ZOOM_resultset_option_get(resset->zset_, *key);

    if (value) {
        NanReturnValue(NanNew(value));
    }
}

NAN_METHOD(ResultSet::Set) {
    ResultSet* resset = node::ObjectWrap::Unwrap<ResultSet>(args.This());

    if (args.Length() < 2) {
        NanThrowError(ArgsSizeError("Set", 1, args.Length()));
        return;
    }

    NanUtf8String key(args[0]);
    NanUtf8String value(args[1]);
    ZOOM_resultset_option_set(resset->zset_, *key, *value);

    NanReturnValue(args.This());
}

} // namespace node_zoom
