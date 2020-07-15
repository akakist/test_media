#ifndef MF_TYPES_H_
#define MF_TYPES_H_
#include <string>
#include <vector>
#include "ioBuffer.h"

typedef long long int REFERENCE_TIME;

typedef	enum HRESULT
{
    S_OK = 0,
    S_FALSE = 1,
    E_NOTIMPL = 0x80004001L,
    E_OUTOFMEMORY = 0x8007000EL,
    E_INVALIDARG = 0x80070057L
} HRESULT;

typedef struct M_TIME
{
    REFERENCE_TIME rtStartTime;
    REFERENCE_TIME rtEndTime;
} 	M_TIME;

inline outBuffer& operator<< (outBuffer& b,const M_TIME& s)
{

    b<<s.rtStartTime<<s.rtEndTime;
    return b;
}
inline inBuffer& operator>> (inBuffer& b,  M_TIME& s)
{
    b>>s.rtStartTime>>s.rtEndTime;
    return b;
}
typedef enum eMFCC
{
    eMFCC_Default = 0,
    eMFCC_I420 = 0x30323449,
    eMFCC_YV12 = 0x32315659,
    eMFCC_NV12 = 0x3231564e,
    eMFCC_YUY2 = 0x32595559,
    eMFCC_YVYU = 0x55595659,
    eMFCC_UYVY = 0x59565955,
    eMFCC_RGB24 = 0xe436eb7d,
    eMFCC_RGB32 = 0xe436eb7e,
} 	eMFCC;

typedef struct M_VID_PROPS
{
    eMFCC fccType;
    int nWidth;
    int nHeight;
    int nRowBytes;
    short nAspectX;
    short nAspectY;
    double dblRate;
} 	M_VID_PROPS;

inline outBuffer& operator<< (outBuffer& b,const M_VID_PROPS& s)
{
    b<<s.fccType<<s.nWidth<<s.nHeight<<s.nRowBytes<<s.nAspectX<<s.nAspectY<<std::to_string(s.dblRate);
    return b;
}
inline inBuffer& operator>> (inBuffer& b,  M_VID_PROPS& s)
{
    int64_t _fccType;
    std::string _dblRate;
    b>>_fccType>>s.nWidth>>s.nHeight>>s.nRowBytes>>s.nAspectX>>s.nAspectY>>_dblRate;
    s.fccType=(eMFCC)_fccType;
    s.dblRate=atof(_dblRate.c_str());
    return b;
}

typedef struct M_AUD_PROPS
{
    int nChannels;
    int nSamplesPerSec;
    int nBitsPerSample;
    int nTrackSplitBits;
} 	M_AUD_PROPS;
inline outBuffer& operator<< (outBuffer& b,const M_AUD_PROPS& s)
{
    b<<s.nChannels<<s.nSamplesPerSec<<s.nBitsPerSample<<s.nTrackSplitBits;
    return b;
}
inline inBuffer& operator>> (inBuffer& b,  M_AUD_PROPS& s)
{
    b>>s.nChannels>>s.nSamplesPerSec>>s.nBitsPerSample>>s.nTrackSplitBits;
    return b;
}

typedef struct M_AV_PROPS
{
    M_VID_PROPS vidProps;
    M_AUD_PROPS audProps;
} 	M_AV_PROPS;
inline outBuffer& operator<< (outBuffer& b,const M_AV_PROPS& s)
{
    b<<s.vidProps<<s.audProps;
    return b;
}
inline inBuffer& operator>> (inBuffer& b,  M_AV_PROPS& s)
{
    b>>s.vidProps>>s.audProps;
    return b;
}

typedef struct MF_BASE_TYPE: public Refcountable
{
    virtual ~MF_BASE_TYPE() {}
    virtual void unpack(inBuffer& in)=0;
    virtual void pack(outBuffer& out)=0;
} MF_BASE_TYPE;

typedef struct MF_FRAME: public MF_BASE_TYPE
{
//	typedef std::shared_ptr<MF_FRAME> TPtr;

    M_TIME      time = {};
    M_AV_PROPS    av_props = {};
    std::string    str_user_props;
    std::vector<uint8_t> vec_video_data;
    std::vector<uint8_t> vec_audio_data;
    void unpack(inBuffer& in) final
    {
        in>>time>>av_props>>str_user_props>>vec_video_data>>vec_audio_data;
    }
    void pack(outBuffer& out) final
    {
        out<< typeid (MF_FRAME).name();
        out<<time<<av_props<<str_user_props<<vec_video_data<<vec_audio_data;
    }


} MF_FRAME;

typedef enum eMFBufferFlags
{
    eMFBF_Empty = 0,
    eMFBF_Buffer = 0x1,
    eMFBF_Packet = 0x2,
    eMFBF_Frame = 0x3,
    eMFBF_Stream = 0x4,
    eMFBF_SideData = 0x10,
    eMFBF_VideoData = 0x20,
    eMFBF_AudioData = 0x40,
} 	eMFBufferFlags;

typedef struct MF_BUFFER: public MF_BASE_TYPE
{
//	typedef std::shared_ptr<MF_BUFFER> TPtr;

    eMFBufferFlags       flags;
    std::vector<uint8_t> data;
    void unpack(inBuffer& in) final
    {
        int64_t _flags;
        in>>_flags>>data;
        flags=(eMFBufferFlags)_flags;
    }
    virtual void pack(outBuffer& out) final
    {
        out<< typeid (MF_BUFFER).name();
        out<<flags<<data;
    }

} MF_BUFFER;

#endif
