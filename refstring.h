#ifndef ___________________________REFSTRING_H
#define ___________________________REFSTRING_H

#include "REF.h"
#include <string>
#include <inttypes.h>

/// refbuffer - string replacement with recount
struct refbuffer: public Refcountable
{
private:
public:
    uint8_t * buffer;
    size_t size;
    size_t capacity;

    std::string asString()
    {
        if(size && buffer)
            return std::string((char*)buffer,size);
        else return "";
    }
    refbuffer():buffer(NULL),size(0),capacity(0) {}
    ~refbuffer()
    {
        if(buffer)
            ::free(buffer);
    }
    uint8_t * alloc(size_t sz)
    {
        if(size>=sz && buffer)
            return buffer;
        if(buffer)
            free(buffer);
        buffer=(uint8_t*)malloc(sz+0x20);
        size=sz+10;
        return buffer;
    }
};

inline REF_getter<refbuffer> toRef(const std::string&s)
{
    if(s.size())
    {
        REF_getter<refbuffer> r=new refbuffer;
        r->capacity=s.size()+0x20;
        r->buffer=(uint8_t*) malloc(r->capacity);
        if(!r->buffer) throw std::runtime_error("alloc error");
        r->size=s.size();
        memcpy(r->buffer,s.data(),s.size());
        return r;
    }
    REF_getter<refbuffer> r=new refbuffer;
    return r;
}
#endif // REFSTRING_H
