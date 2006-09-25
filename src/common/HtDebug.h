//
// HtDebug.h
//


#ifndef	_HtDebug_h_
#define	_HtDebug_h_

#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

class HtDebug
{
    public:
        //
        // singleton accessor
        //
        static HtDebug* Instance();
        void closeLogfile (void);
        void close (void);

        void finalize (void); // allows calling of the REAL destructor

        //
        // set functions
        //
        void setLevel (int level)                   { _stdoutLevel = _fileLevel = level > 0 ? level : 0; } // :)
        void setStdoutLevel (int level)             { _stdoutLevel = level > 0 ? level : 0; }
        void setFileLevel (int level)               { _fileLevel = level > 0 ? level : 0; }
        bool setLogfile (string filename)           { return setLogfile(filename.c_str()); }
        bool setLogfile (const char* filename);

        //
        // get functions
        //
        int getLevel (void)                         { return _fileLevel > _stdoutLevel ? _fileLevel : _stdoutLevel; }
        int getFileLevel (void)                     { return _fileLevel; }
        int getStdoutLevel (void)                   { return _stdoutLevel; }
        string getLogfile (void)                    { return _filename; }
        string getLastMessage (void)                { return _lastMessage; }
        string getLastLogMessage (void)             { return _lastLogMessage; }

        //
        // logging functions
        //
        void outlog (int level, const char * message, ...);
        void outlog (int level, string message)     { outlog(level, message.c_str()); }

    protected:
        HtDebug();
        ~HtDebug();

    private:
        //
        // the message level passed to outlog() must be less than or equal to these in order
        // to be logged to the respective output sink. attempting to set levels less than
        // zero will make the levels zero, ergo calling outlog() with a level of -1 will cause
        // the message to ALWAYS be printed.
        //
        int         _stdoutLevel;
        int         _fileLevel;

        //
        // the file object and its name
        //
        string      _filename;
        ofstream    _file;

        //
        // the last messagelogged, and the last message recieved
        //
        string      _lastMessage;
        string      _lastLogMessage;

        //
        // this buffer is for the vsnprintf call, so variable arguments
        // can be written in the log
        //
        char        messageBuffer[500];

        //
        // the actual singleton object
        //
        static HtDebug* _instance;
};

//********************************************************************
//

#endif


