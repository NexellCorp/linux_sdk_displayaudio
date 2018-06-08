#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include <execinfo.h>
#include <signal.h>

#include <exception>
#include <iostream>
#include <fstream>

using namespace std;

class ExceptionTracer
{
    public:
        ExceptionTracer()
        {
            void * array[100];
            int nSize = backtrace(array, 100);
            char ** symbols = backtrace_symbols(array, nSize);

            // error file
            ofstream outFile;
            outFile.open("/program-error.log", ios::trunc);
            outFile << __DATE__ << "  " << __TIME__ << endl;
            for (int i = 0; i < nSize; i++)
            {
                cout << symbols[i] << endl;
                outFile << symbols[i] << endl;
            }

            free(symbols);
        }
};

template <class SignalExceptionClass> class SignalTranslator
{
    private:
        class SingleTonTranslator
        {
            public:
                SingleTonTranslator()
                {
                    signal(SignalExceptionClass::GetSignalNumber(), SignalHandler);
                }

                static void SignalHandler(int)
                {
                    throw SignalExceptionClass();
                }
        };

    public:
        SignalTranslator()
        {
            static SingleTonTranslator s_objTranslator;
        }
};

// An example for SIGSEGV
class SegmentationFault : public ExceptionTracer, public exception
{
    public:
        static int GetSignalNumber() {return SIGSEGV;}
};
SignalTranslator<SegmentationFault> g_objSegmentationFaultTranslator;

// An example for SIGFPE
class FloatingPointException : public ExceptionTracer, public exception
{
    public:
        static int GetSignalNumber() {return SIGFPE;}
};
SignalTranslator<FloatingPointException> g_objFloatingPointExceptionTranslator;

// An example for SIGILL
class IllegalInstructionException : public ExceptionTracer, public exception
{
    public:
        static int GetSignalNumber() {return SIGILL;}
};
SignalTranslator<IllegalInstructionException>
    g_objIllegalInstructionExceptionTranslator;

// An example for SIGTRAP
class TrapException : public ExceptionTracer, public exception
{
    public:
        static int GetSignalNumber() {return SIGTRAP;}
};
SignalTranslator<TrapException> g_objTrapExceptionTranslator;

class ExceptionHandler
{
    private:
        class SingleTonHandler
        {
            public:
                SingleTonHandler()
                {
                    set_terminate(Handler);
                }

                static void Handler()
                {
                    // Exception from construction/destruction of global variables
                    try
                    {
                        // re-throw
                        throw;
                    }
                    catch (SegmentationFault &)
                    {
                        cout << "SegmentationFault" << endl;
                    }
                    catch (FloatingPointException &)
                    {
                        cout << "FloatingPointException" << endl;
                    }
                    catch (IllegalInstructionException &)
                    {
                        cout << "IllegalInstructionException" << endl;
                    }
                    catch (TrapException &)
                    {
                        cout << "TrapException" << endl;
                    }
                    catch (...)
                    {
                        cout << "Unknown Exception" << endl;
                    }

                    //if this is a thread performing some core activity
                    abort();
                    // else if this is a thread used to service requests
                    // pthread_exit();
                }
        };

    public:
        ExceptionHandler()
        {
            static SingleTonHandler s_objHandler;
        }
};

// Before defining any global variable, we define a dummy instance
// of ExceptionHandler object to make sure that
// ExceptionHandler::SingleTonHandler::SingleTonHandler() is invoked
ExceptionHandler g_objExceptionHandler;

#endif // EXCEPTIONHANDLER_H
