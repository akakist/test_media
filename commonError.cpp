#include "commonError.h"
#if !defined __ANDROID__ && !defined __FreeBSD__

#include <sys/timeb.h>
#endif
#ifndef _WIN32
#include <syslog.h>
#endif

#ifdef __MACH__
#ifndef __IOS__
#ifdef QT5
#include <QStandardPaths>
#include <QCoreApplication>
#endif
#endif
#endif
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <stdarg.h>

#ifdef QT5
#include <QStandardPaths>
#endif

bool prevLogUnlinked=false;
CommonError::CommonError(const std::string& str):m_error(str)
{
}
CommonError::CommonError(const char* fmt, ...) :m_error(fmt)
{

    va_list ap;
    char str[1024*10];
    va_start(ap, fmt);
    vsnprintf(str, sizeof(str), fmt, ap);
    va_end(ap);
    m_error=str;

}




