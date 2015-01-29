#pragma once

#include <nan.h>

class Connection : public::ObjectWrap {
    public:
        explicit Connection();
        ~Connection();

        static void Init(v8::Handle<v8::Object> exports);
}
