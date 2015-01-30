#pragma once
#include <nan.h>

extern "C"{
    #include <yaz/zoom.h>
}

namespace node_zoom {

class ResultSet : public node::ObjectWrap {
    public:
        explicit ResultSet(ZOOM_resultset resultset);
        ~ResultSet();

        static void Init();
        static NAN_METHOD(New);
        static NAN_METHOD(Get);
        static NAN_METHOD(Set);

    protected:
        ZOOM_resultset zset_;
        static v8::Persistent<v8::Function> constructor;
};

} // namespace node_zoom
