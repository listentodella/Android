#ifndef _ANONYMOUS7_H
#define _ANONYMOUS7_H


// ---------------------------------------------------------------------------
enum  {
  ACQUIRE_BUFFER =  IBinder::FIRST_CALL_TRANSACTION,
  DETACH_BUFFER,
  ATTACH_BUFFER,
  RELEASE_BUFFER,
  CONSUMER_CONNECT,
  CONSUMER_DISCONNECT,
  GET_RELEASED_BUFFERS,
  SET_DEFAULT_BUFFER_SIZE,
  SET_DEFAULT_MAX_BUFFER_COUNT,
  DISABLE_ASYNC_BUFFER,
  SET_MAX_ACQUIRED_BUFFER_COUNT,
  SET_CONSUMER_NAME,
  SET_DEFAULT_BUFFER_FORMAT,
  SET_CONSUMER_USAGE_BITS,
  SET_TRANSFORM_HINT,
  GET_SIDEBAND_STREAM,
  DUMP

};
#endif
