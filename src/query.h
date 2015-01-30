#pragma once
#include <nan.h>
#include <string>

extern "C"{
    #include <yaz/zoom.h>
}

namespace node_zoom {

class Query : public node::ObjectWrap {
    public:
        explicit Query();
        ~Query();

        static void Init(v8::Handle<v8::Object> exports);
        static NAN_METHOD(New);
        static NAN_METHOD(Prefix);
        static NAN_METHOD(CQL);
        static NAN_METHOD(SortBy);

    protected:
        ZOOM_query zquery_;
        static v8::Persistent<v8::Function> constructor;
};

} // namespace node_zoom
