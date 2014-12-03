/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_TCPSOCKET_H__
#define __DAVAENGINE_TCPSOCKET_H__

#include <Base/Function.h>

#include "TCPSocketTemplate.h"

namespace DAVA
{

/*
 Class TCPSocket provides a TCP socket.
 TCPSocket allows to establish connection and transfer data.
 All operations are executed asynchronously and all these operations must be started in thread context
 where IOLoop is running, i.e. they can be started during handler processing or using IOLoop Post method.
 List of methods starting async operations:
    Connect
    StartRead
    Write
    Shutdown
    Close

 TCPSocket notifies user about operation completion and its status through user-supplied functional objects (handlers or callbacks).
 Handlers are called with some parameters depending on operation. Each handler besides specific parameters is passed a
 pointer to TCPSocket instance.

 Note: handlers should not block, this will cause all network system to freeze.

 To start working with TCPSocket you should do one of the following:
    1. call Connect method and in its handler (if not failed) you can start reading and writing to socket
    2. pass it to Accept method of TCPAcceptor class while handling acceptor's connect callback

 Read operation is started only once, it will be automatically restarted after executing handler. User should
 only provide new destination buffer through ReadHere method if neccesary while in read handler. To stop reading and
 of course any other activity call Close method.

 Write operation supports sending of several buffers at a time. Write handler receives these buffers back and you get
 a chance to free buffers if neccesary.
 Note 1: user should ensure that buffers stay valid and untouched during write operation.
 Note 2: next write must be issued only after completion of previous write operation.

 Shutdown used for gracefull disconnect before calling Close, it shutdowns sending side of socket and waits for
 pending write operations to complete.

 Close also is executed asynchronously and in its handler it is allowed to destroy TCPSocket object.
*/
class TCPAcceptor;
class TCPSocket : public TCPSocketTemplate<TCPSocket>
{
    friend TCPSocketTemplate<TCPSocket>;    // Make base class friend to allow it to call my Handle... methods
    friend TCPAcceptor;                     // Make TCPAcceptor friend to allow it to do useful work in its Accept

public:
    // Handler types
    typedef Function<void(TCPSocket* socket)> CloseHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error)> ShutdownHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error)> ConnectHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, size_t nread)> ReadHandlerType;
    typedef Function<void(TCPSocket* socket, int32 error, const Buffer* buffers, size_t bufferCount)> WriteHandlerType;

public:
    TCPSocket(IOLoop* ioLoop);

    int32 Connect(const Endpoint& endpoint, ConnectHandlerType handler);
    int32 StartRead(Buffer buffer, ReadHandlerType handler);
    int32 Write(const Buffer* buffers, size_t bufferCount, WriteHandlerType handler);
    int32 Shutdown(ShutdownHandlerType handler);
    void Close(CloseHandlerType handler = CloseHandlerType());

    void ReadHere(Buffer buffer);

private:
    void HandleClose();
    void HandleConnect(int32 error);
    void HandleShutdown(int32 error);
    void HandleAlloc(Buffer* buffer);
    void HandleRead(int32 error, size_t nread);
    void HandleWrite(int32 error, const Buffer* buffers, size_t bufferCount);

private:
    Buffer readBuffer;
    CloseHandlerType closeHandler;
    ShutdownHandlerType shutdownHandler;
    ConnectHandlerType connectHandler;
    ReadHandlerType readHandler;
    WriteHandlerType writeHandler;
};

}	// namespace DAVA

#endif  // __DAVAENGINE_TCPSOCKET_H__
