// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_VIDEO_JPEG_DECODE_ACCELERATOR_H_
#define MEDIA_VIDEO_JPEG_DECODE_ACCELERATOR_H_

#include "base/basictypes.h"
#include "media/base/bitstream_buffer.h"
#include "media/base/media_export.h"
#include "media/base/video_frame.h"

namespace media {

// JPEG decoder interface.
// The input are JPEG images including headers (Huffman tables may be omitted).
// The output color format is I420. The decoder will convert the color format
// to I420 if the color space or subsampling does not match that and if it is
// capable of doing so. The client is responsible for allocating buffers and
// keeps the ownership of them. All methods must be called on the same thread.
// The intended use case of this interface is decoding MJPEG images coming
// from camera capture. It can also be used for normal still JPEG image
// decoding, but normal JPEG images may use more JPEG features that may not be
// supported by a particular accelerator implementation and/or platform.
class MEDIA_EXPORT JpegDecodeAccelerator {
 public:
  static const int32_t kInvalidBitstreamBufferId = -1;

  // Enumeration of decode errors generated by NotifyError callback.
  enum Error {
    // Invalid argument was passed to an API method, e.g. the output buffer is
    // too small, JPEG width/height are too big for JDA.
    INVALID_ARGUMENT,
    // Encoded input is unreadable, e.g. failed to map on another process.
    UNREADABLE_INPUT,
    // Failed to parse compressed JPEG picture.
    PARSE_JPEG_FAILED,
    // Failed to decode JPEG due to unsupported JPEG features, such as profiles,
    // coding mode, or color formats.
    UNSUPPORTED_JPEG,
    // A fatal failure occurred in the GPU process layer or one of its
    // dependencies. Examples of such failures include hardware failures,
    // driver failures, library failures, browser programming errors, and so
    // on. Client is responsible for destroying JDA after receiving this.
    PLATFORM_FAILURE,
    // Largest used enum. This should be adjusted when new errors are added.
    LARGEST_ERROR_ENUM = PLATFORM_FAILURE,
  };

  class MEDIA_EXPORT Client {
   public:
    // Callback called after each successful Decode().
    // Parameters:
    //  |bitstream_buffer_id| is the id of BitstreamBuffer corresponding to
    //  Decode() call.
    virtual void VideoFrameReady(int32_t bitstream_buffer_id) = 0;

    // Callback to notify errors. Client is responsible for destroying JDA when
    // receiving a fatal error, i.e. PLATFORM_FAILURE. For other errors, client
    // is informed about the buffer that failed to decode and may continue
    // using the same instance of JDA.
    // Parameters:
    //  |error| is the error code.
    //  |bitstream_buffer_id| is the bitstream buffer id that resulted in the
    //  recoverable error. For PLATFORM_FAILURE, |bitstream_buffer_id| may be
    //  kInvalidBitstreamBufferId if the error was not related to any
    //  particular buffer being processed.
    virtual void NotifyError(int32_t bitstream_buffer_id, Error error) = 0;

   protected:
    virtual ~Client() {}
  };

  // JPEG decoder functions.

  // Initializes the JPEG decoder. Should be called once per decoder
  // construction. This call is synchronous and returns true iff initialization
  // is successful.
  // Parameters:
  //  |client| is the Client interface for decode callback. The provided
  //  pointer must be valid until Destroy() is called.
  virtual bool Initialize(Client* client) = 0;

  // Decodes the given bitstream buffer that contains one JPEG picture. It
  // supports at least baseline encoding defined in JPEG ISO/IEC 10918-1. The
  // decoder will convert the color format to I420 or return UNSUPPORTED_JPEG
  // if it cannot convert. Client still owns this buffer, but should deallocate
  // or access the buffer only after receiving a decode callback VideoFrameReady
  // with the corresponding bitstream_buffer_id, or NotifyError.
  // Parameters:
  //  |bitstream_buffer| contains encoded JPEG picture.
  //  |video_frame| contains an allocated video frame for the output.
  //  Client is responsible for filling the coded_size of video_frame and
  //  allocating its backing buffer. For now, only shared memory backed
  //  VideoFrames are supported. After decode completes, decoded JPEG picture
  //  will be filled into the |video_frame|.
  //  Ownership of the |bitstream_buffer| and |video_frame| remains with the
  //  client. The client is not allowed to deallocate them before
  //  VideoFrameReady or NotifyError() is invoked for given id of
  //  |bitstream_buffer|, or Destroy() returns.
  virtual void Decode(const BitstreamBuffer& bitstream_buffer,
                      const scoped_refptr<media::VideoFrame>& video_frame) = 0;

  // Destroys the decoder: all pending inputs are dropped immediately. This
  // call may asynchronously free system resources, but its client-visible
  // effects are synchronous. After this method returns, no more callbacks
  // will be made on the client. Deletes |this| unconditionally, so make sure
  // to drop all pointers to it!
  virtual void Destroy() = 0;

 protected:
  // Do not delete directly; use Destroy() or own it with a scoped_ptr, which
  // will Destroy() it properly by default.
  virtual ~JpegDecodeAccelerator();
};

}  // namespace media

namespace base {

template <class T>
struct DefaultDeleter;

// Specialize DefaultDeleter so that scoped_ptr<JpegDecodeAccelerator> always
// uses "Destroy()" instead of trying to use the destructor.
template <>
struct MEDIA_EXPORT DefaultDeleter<media::JpegDecodeAccelerator> {
 public:
  void operator()(void* jpeg_decode_accelerator) const;
};

}  // namespace base

#endif  // MEDIA_VIDEO_JPEG_DECODE_ACCELERATOR_H_