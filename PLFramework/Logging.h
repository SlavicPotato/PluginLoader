#pragma once

namespace Log
{
    class ILog
    {
    public:
        ILog();
        virtual ~ILog() noexcept;

        virtual bool Open(const TCHAR* fname);
        virtual bool Open(const TCHAR* basepath, const TCHAR* fname);
        virtual void Close();
        virtual void Message(const TCHAR* fmt, ...);
        virtual void _Message(const TCHAR* fmt, va_list args);
    private:

        ILog(const ILog&) = delete;
        ILog& operator=(const ILog&) = delete;

        void Write(const TCHAR* fmt);

        STD_OFSTREAM ofs;
        TCHAR fmtBuf[8192];
        CRITICAL_SECTION cs;
    };

    extern ILog gLog;

    __inline void _MESSAGE(const TCHAR* fmt, ...)
    {
        va_list args;

        va_start(args, fmt);
        gLog._Message(fmt, args);
        va_end(args);
    }

}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MESSAGE(fmt, ...) Log::_MESSAGE( _T(__FILE__) ":" TOSTRING(__LINE__) " [" __FUNCTION__ "] " fmt, __VA_ARGS__)
