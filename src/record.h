#pragma once
#include <nan.h>

extern "C"{
    #include <yaz/zoom.h>
}

namespace node_zoom {

class Record : public node::ObjectWrap {
    public:
        explicit Record(ZOOM_record record);
        ~Record();

        static void Init();
        static NAN_METHOD(New);
        static NAN_METHOD(Get);

    protected:
        ZOOM_record zrecord_;
        static v8::Persistent<v8::Function> constructor;
};

} // namespace node_zoom
