nark-rpc
========

RPC(Remote Procedure Call) on top of nark-serialization

## Quick Start
### IDL
nark-rpc are all in C++, even its IDL is C++, `samples/ifile.h` is a good example:

```c++
BEGIN_RPC_INTERFACE(FileObj, SessionScope)
    RPC_ADD_MF(open)
    RPC_ADD_MF(read)
    RPC_ADD_MF(write)
    RPC_ADD_MF(close)
END_RPC_ADD_MF()
    RPC_DECLARE_MF(open, (const string& fname, const string& mode))
    RPC_DECLARE_MF(read, (vector<char>* buffer, uint32_t length))
    RPC_DECLARE_MF(write, (const vector<char>& buffer, uint32_t* length))
    RPC_DECLARE_MF(close, ())
END_RPC_INTERFACE()
```

This can be thought of as nark-rpc's IDL, as it declared, `FileObj` is in `SessionScope`.

There is another scope: `GlobaleScope`, `samples/echo.h` is such an example:
```c++
BEGIN_RPC_INTERFACE(Echo, GlobaleScope)
    RPC_ADD_MF(echo)
END_RPC_ADD_MF()
    // 3rd macro param is ';' means non-pure-virtual
    RPC_DECLARE_MF_EX(echo, (const string& msg, string* echo_of_server), ;)
END_RPC_INTERFACE()
```

Note: function overload is not allowed in IDL.

### Client

RPC client just call the (member) functions defined in IDL, the functions seem defined as normal functions.

See [samples/file\_client/file\_client.cpp](samples/file_client/file_client.cpp#L22)<br/>
See [samples/echo\_client/echo\_client.cpp](samples/echo_client/echo_client.cpp#L23)

### Server

RPC server implement the (member) functions, these functions are called by the client through network.

Writing a RPC server is as simple as writing a normal class:<br/>
See [samples/file\_server/file\_server.cpp](samples/file_server/file_server.cpp#L24)

Because `echo` declared in `ifile.h` is non-pure-virtual, this makes it much simpler:<br/>
See [samples/echo\_server/echo\_server.cpp](samples/echo_server/echo_server.cpp#L24)

## More

To be written...
