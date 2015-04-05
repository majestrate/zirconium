// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/file_system_provider/fileapi/buffering_file_stream_writer.h"

#include <algorithm>

#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"

namespace chromeos {
namespace file_system_provider {

BufferingFileStreamWriter::BufferingFileStreamWriter(
    scoped_ptr<storage::FileStreamWriter> file_stream_writer,
    int intermediate_buffer_length)
    : file_stream_writer_(file_stream_writer.Pass()),
      intermediate_buffer_length_(intermediate_buffer_length),
      intermediate_buffer_(new net::IOBuffer(intermediate_buffer_length_)),
      buffered_bytes_(0),
      weak_ptr_factory_(this) {
}

BufferingFileStreamWriter::~BufferingFileStreamWriter() {
  if (buffered_bytes_)
    LOG(ERROR) << "File stream writer not flushed. Data will be lost.";
}

int BufferingFileStreamWriter::Write(net::IOBuffer* buffer,
                                     int buffer_length,
                                     const net::CompletionCallback& callback) {
  // If |buffer_length| is larger than the intermediate buffer, then call the
  // inner file stream writer directly. Note, that the intermediate buffer
  // (used for buffering) must be flushed first.
  if (buffer_length > intermediate_buffer_length_) {
    if (buffered_bytes_) {
      FlushIntermediateBuffer(
          base::Bind(&BufferingFileStreamWriter::
                         OnFlushIntermediateBufferForDirectWriteCompleted,
                     weak_ptr_factory_.GetWeakPtr(),
                     make_scoped_refptr(buffer),
                     buffer_length,
                     callback));
    } else {
      // Nothing to flush, so skip it.
      OnFlushIntermediateBufferForDirectWriteCompleted(
          make_scoped_refptr(buffer), buffer_length, callback, net::OK);
    }
    return net::ERR_IO_PENDING;
  }

  // Buffer consecutive writes to larger chunks.
  const int buffer_bytes =
      std::min(intermediate_buffer_length_ - buffered_bytes_, buffer_length);

  CopyToIntermediateBuffer(
      make_scoped_refptr(buffer), 0 /* buffer_offset */, buffer_bytes);
  const int bytes_left = buffer_length - buffer_bytes;

  if (buffered_bytes_ == intermediate_buffer_length_) {
    FlushIntermediateBuffer(
        base::Bind(&BufferingFileStreamWriter::
                       OnFlushIntermediateBufferForBufferedWriteCompleted,
                   weak_ptr_factory_.GetWeakPtr(),
                   make_scoped_refptr(buffer),
                   buffer_bytes,
                   bytes_left,
                   callback));
    return net::ERR_IO_PENDING;
  }

  // Optimistically return a success.
  return buffer_length;
}

int BufferingFileStreamWriter::Cancel(const net::CompletionCallback& callback) {
  // Since there is no any asynchronous call in this class other than on
  // |file_stream_writer_|, then there must be an in-flight operation going on.
  return file_stream_writer_->Cancel(callback);
}

int BufferingFileStreamWriter::Flush(const net::CompletionCallback& callback) {
  // Flush all the buffered bytes first, then invoke Flush() on the inner file
  // stream writer.
  FlushIntermediateBuffer(base::Bind(
      &BufferingFileStreamWriter::OnFlushIntermediateBufferForFlushCompleted,
      weak_ptr_factory_.GetWeakPtr(),
      callback));
  return net::ERR_IO_PENDING;
}

void BufferingFileStreamWriter::CopyToIntermediateBuffer(
    scoped_refptr<net::IOBuffer> buffer,
    int buffer_offset,
    int buffer_length) {
  DCHECK_GE(intermediate_buffer_length_, buffer_length + buffered_bytes_);
  memcpy(intermediate_buffer_->data() + buffered_bytes_,
         buffer->data() + buffer_offset,
         buffer_length);
  buffered_bytes_ += buffer_length;
}

void BufferingFileStreamWriter::FlushIntermediateBuffer(
    const net::CompletionCallback& callback) {
  const int result = file_stream_writer_->Write(
      intermediate_buffer_.get(),
      buffered_bytes_,
      base::Bind(&BufferingFileStreamWriter::OnFlushIntermediateBufferCompleted,
                 weak_ptr_factory_.GetWeakPtr(),
                 buffered_bytes_,
                 callback));
  DCHECK_EQ(net::ERR_IO_PENDING, result);
}

void BufferingFileStreamWriter::OnFlushIntermediateBufferCompleted(
    int length,
    const net::CompletionCallback& callback,
    int result) {
  if (result < 0) {
    callback.Run(result);
    return;
  }

  DCHECK_EQ(length, result) << "Partial writes are not supported.";
  buffered_bytes_ = 0;

  callback.Run(net::OK);
}

void
BufferingFileStreamWriter::OnFlushIntermediateBufferForDirectWriteCompleted(
    scoped_refptr<net::IOBuffer> buffer,
    int length,
    const net::CompletionCallback& callback,
    int result) {
  if (result < 0) {
    callback.Run(result);
    return;
  }

  // The following logic is only valid if the intermediate buffer is empty.
  DCHECK_EQ(0, buffered_bytes_);

  const int write_result =
      file_stream_writer_->Write(buffer.get(), length, callback);
  DCHECK_EQ(net::ERR_IO_PENDING, write_result);
}

void
BufferingFileStreamWriter::OnFlushIntermediateBufferForBufferedWriteCompleted(
    scoped_refptr<net::IOBuffer> buffer,
    int buffered_bytes,
    int bytes_left,
    const net::CompletionCallback& callback,
    int result) {
  if (result < 0) {
    callback.Run(result);
    return;
  }

  // Copy the rest of bytes to the buffer.
  DCHECK_EQ(net::OK, result);
  DCHECK_EQ(0, buffered_bytes_);
  DCHECK_GE(intermediate_buffer_length_, bytes_left);
  CopyToIntermediateBuffer(buffer, buffered_bytes, bytes_left);

  callback.Run(buffered_bytes + bytes_left);
}

void BufferingFileStreamWriter::OnFlushIntermediateBufferForFlushCompleted(
    const net::CompletionCallback& callback,
    int result) {
  if (result < 0) {
    callback.Run(result);
    return;
  }

  const int flush_result = file_stream_writer_->Flush(callback);
  DCHECK_EQ(net::ERR_IO_PENDING, flush_result);
}

}  // namespace file_system_provider
}  // namespace chromeos
