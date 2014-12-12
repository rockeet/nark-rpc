nark-rpc
========

RPC(Remote Procedure Call) on top of nark-serialization

## Prerequisite
<table>
<tbody>
<tr>
<td>[boost-1.41 or newer](http://sourceforge.net/projects/boost/)</td>
<td>
  * Require `boost_thread`, `boost_date_time`, `boost_system` to be built
  * Other boost libraries are used as header-only
</td>
<tr>
<td>[nark-serialization](https://github.com/rockeet/nark-serialization)</td>
<td>Require binary library</td>
</tr>
<tr>
<td>[nark-hashmap](https://github.com/rockeet/nark-hashmap)</td>
<td>header only</td>
<tr>
<tr>
<td>[nark-bone](https://github.com/rockeet/nark-bone)</td>
<td>Require binary library</td>
</tr>
</tbody>
</table>

**Note: All `nark` repositories should be in the same directory**

## Compile
  1. Compile `boost_thread`, `boost_date_time`, `boost_system`
  2. Compile
```bash
$ cd /path/to/nark-bone
$ make
$ cd ../nark-serialization
$ make
$ cd ../nark-rpc
$ make
$ cd samples # /path/to/nark-rpc/samples
$ make
$ build/*/dbg/echo_server/echo_server.exe # run echo server
$ #
$ # open a new terminal
$ build/*/dbg/echo_client/echo_client.exe # run echo client
$ # Have Fun!
```

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
    RPC_DECLARE_MF_D(echo, (const string& msg, string* echo_of_server))
END_RPC_INTERFACE()
```

* Notes
  1. Function overload is not allowed in IDL.
  2. `RPC_DECLARE_MF` or `RPC_DECLARE_MF_D` should be used consitently in the same RPC interface.

### Client

RPC client just call the (member) functions defined in IDL, the functions seem defined as normal functions.
`RPC_DECLARE_MF` and `RPC_DECLARE_MF_D` are the same at client side.

See [samples/file\_client/file\_client.cpp](samples/file_client/file_client.cpp#L22)<br/>
See [samples/echo\_client/echo\_client.cpp](samples/echo_client/echo_client.cpp#L23)

### Server

RPC server implement the (member) functions, these functions are called by the client through network.

Writing a RPC server is as simple as writing a normal class:

Functions declared by `RPC_DECLARE_MF` are **pure virtual**, so you must define an implementation class:<br/>
See [samples/file\_server/file\_server.cpp](samples/file_server/file_server.cpp#L24)

Functions declared by `RPC_DECLARE_MF_D` are not **pure virtual**, `_D` means `Direct`:<br/>
See [samples/echo\_server/echo\_server.cpp](samples/echo_server/echo_server.cpp#L24)

## More

To be written...
