#include <nan.h>
#include "options.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
    node_zoom::Options::Init(exports);
}

NODE_MODULE(zoom, InitAll);
