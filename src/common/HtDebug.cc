//
// HtDebug.cc
//

#include "HtDebug.h"
#include <stdarg.h>


HtDebug* HtDebug::_instance = NULL;


HtDebug* HtDebug::Instance()
{
    if (_instance == NULL)
    {
        _instance = new HtDebug();
    }

    return _instance;
}


HtDebug::HtDebug()
{
    _stdoutLevel = 0;
    _fileLevel = 0;
}


HtDebug::~HtDebug()
{
    close();
}


void HtDebug::finalize()
{
    close();

    if (_instance != NULL)
    {
        delete _instance;
        _instance = NULL;
    }
}


bool HtDebug::setLogfile (const char* filename)
{
    if (_file.is_open())
    {
        close();
    }

    //
    // always append to the file. this could be a config setting eventually
    //
    _file.open(filename, ios_base::out | ios_base::app);

    if (!_file.is_open())
    {
        //
        // at least tell someone that the file open failed
        //
        _filename.clear();
        cout << "HtDebug: Debug logfile open failed, filename = [" << filename << "]" << endl;
        return false;
    }

    _filename = filename;

    time_t now = time(NULL);
    _file << "### Logfile opened " << ctime(&now);

    return true;
}


void HtDebug::close()
{
    closeLogfile();
}


void HtDebug::closeLogfile ()
{
    if (_file.is_open())
    {
        time_t now = time(NULL);
        _file << "### Logfile closed " << ctime(&now) << endl;
        _file.close();
    }
    _filename.clear();
}


void HtDebug::outlog (int level, const char * message, ...)
{
    va_list args;
    bool logged = false;

    va_start(args, message);
    vsnprintf(messageBuffer, sizeof(messageBuffer)-1, message, args);
    va_end(args);

    _lastMessage = messageBuffer;

    //
    // make sure the file is open before testing the level
    //
    if (_file.is_open() && (_fileLevel > level))
    {
        _file << _lastMessage;
        _file.flush();
        logged = true;
    }

    if (_stdoutLevel > level)
    {
        cout << _lastMessage;
        cout.flush();
        logged = true;
    }

    if(logged)
    {
        _lastLogMessage = _lastMessage;
    }
}


