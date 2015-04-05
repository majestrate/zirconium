// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/extensions/file_manager/private_api_mount.h"

#include <string>

#include "base/files/file_util.h"
#include "base/format_macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/chromeos/drive/file_system_interface.h"
#include "chrome/browser/chromeos/drive/file_system_util.h"
#include "chrome/browser/chromeos/extensions/file_manager/private_api_util.h"
#include "chrome/browser/chromeos/file_manager/fileapi_util.h"
#include "chrome/browser/chromeos/file_manager/volume_manager.h"
#include "chrome/browser/drive/event_logger.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/extensions/api/file_manager_private.h"
#include "chromeos/disks/disk_mount_manager.h"
#include "content/public/browser/browser_thread.h"
#include "google_apis/drive/task_util.h"
#include "ui/shell_dialogs/selected_file_info.h"

using chromeos::disks::DiskMountManager;
using content::BrowserThread;
namespace file_manager_private = extensions::api::file_manager_private;

namespace extensions {

namespace {

// Does chmod o+r for the given path to ensure the file is readable from avfs.
void EnsureReadableFilePermissionOnBlockingPool(
    const base::FilePath& path,
    const base::Callback<void(drive::FileError, const base::FilePath&)>&
        callback) {
  int mode = 0;
  if (!base::GetPosixFilePermissions(path, &mode) ||
      !base::SetPosixFilePermissions(path, mode | S_IROTH)) {
    callback.Run(drive::FILE_ERROR_ACCESS_DENIED, base::FilePath());
    return;
  }
  callback.Run(drive::FILE_ERROR_OK, path);
}

}  // namespace

bool FileManagerPrivateAddMountFunction::RunAsync() {
  using file_manager_private::AddMount::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  drive::EventLogger* logger = file_manager::util::GetLogger(GetProfile());
  if (logger) {
    logger->Log(logging::LOG_INFO, "%s[%d] called. (source: '%s')", name(),
                request_id(),
                params->source.empty() ? "(none)" : params->source.c_str());
  }
  set_log_on_completion(true);

  const base::FilePath path = file_manager::util::GetLocalPathFromURL(
      render_view_host(), GetProfile(), GURL(params->source));

  if (path.empty())
    return false;

  // Check if the source path is under Drive cache directory.
  if (drive::util::IsUnderDriveMountPoint(path)) {
    drive::FileSystemInterface* file_system =
        drive::util::GetFileSystemByProfile(GetProfile());
    if (!file_system)
      return false;

    // Ensure that the cache file exists.
    const base::FilePath drive_path = drive::util::ExtractDrivePath(path);
    file_system->GetFile(
        drive_path,
        base::Bind(&FileManagerPrivateAddMountFunction::RunAfterGetDriveFile,
                   this,
                   drive_path));
  } else {
    file_manager::VolumeManager* volume_manager =
        file_manager::VolumeManager::Get(GetProfile());
    DCHECK(volume_manager);

    bool is_under_downloads = false;
    const std::vector<base::WeakPtr<file_manager::Volume>> volumes =
        volume_manager->GetVolumeList();
    for (const auto& volume : volumes) {
      if (volume->type() == file_manager::VOLUME_TYPE_DOWNLOADS_DIRECTORY &&
          volume->mount_path().IsParent(path)) {
        is_under_downloads = true;
        break;
      }
    }

    if (is_under_downloads) {
      // For files under downloads, change the file permission and make it
      // readable from avfs/fuse if needed.
      BrowserThread::PostBlockingPoolTask(
          FROM_HERE,
          base::Bind(&EnsureReadableFilePermissionOnBlockingPool,
                     path,
                     google_apis::CreateRelayCallback(
                         base::Bind(&FileManagerPrivateAddMountFunction::
                                        RunAfterMarkCacheFileAsMounted,
                                    this,
                                    path.BaseName()))));
    } else {
      RunAfterMarkCacheFileAsMounted(
          path.BaseName(), drive::FILE_ERROR_OK, path);
    }
  }
  return true;
}

void FileManagerPrivateAddMountFunction::RunAfterGetDriveFile(
    const base::FilePath& drive_path,
    drive::FileError error,
    const base::FilePath& cache_path,
    scoped_ptr<drive::ResourceEntry> entry) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  if (error != drive::FILE_ERROR_OK) {
    SendResponse(false);
    return;
  }

  drive::FileSystemInterface* const file_system =
      drive::util::GetFileSystemByProfile(GetProfile());
  if (!file_system) {
    SendResponse(false);
    return;
  }

  file_system->MarkCacheFileAsMounted(
      drive_path,
      base::Bind(
          &FileManagerPrivateAddMountFunction::RunAfterMarkCacheFileAsMounted,
          this,
          drive_path.BaseName()));
}

void FileManagerPrivateAddMountFunction::RunAfterMarkCacheFileAsMounted(
    const base::FilePath& display_name,
    drive::FileError error,
    const base::FilePath& file_path) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  if (error != drive::FILE_ERROR_OK) {
    SendResponse(false);
    return;
  }

  // Pass back the actual source path of the mount point.
  SetResult(new base::StringValue(file_path.AsUTF8Unsafe()));
  SendResponse(true);

  // MountPath() takes a std::string.
  DiskMountManager* disk_mount_manager = DiskMountManager::GetInstance();
  disk_mount_manager->MountPath(
      file_path.AsUTF8Unsafe(),
      base::FilePath(display_name.Extension()).AsUTF8Unsafe(),
      display_name.AsUTF8Unsafe(),
      chromeos::MOUNT_TYPE_ARCHIVE);
}

bool FileManagerPrivateRemoveMountFunction::RunAsync() {
  using file_manager_private::RemoveMount::Params;
  const scoped_ptr<Params> params(Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params);

  drive::EventLogger* logger = file_manager::util::GetLogger(GetProfile());
  if (logger) {
    logger->Log(logging::LOG_INFO, "%s[%d] called. (volume_id: '%s')", name(),
                request_id(), params->volume_id.c_str());
  }
  set_log_on_completion(true);

  using file_manager::VolumeManager;
  using file_manager::Volume;
  VolumeManager* const volume_manager = VolumeManager::Get(GetProfile());
  DCHECK(volume_manager);

  base::WeakPtr<Volume> volume =
      volume_manager->FindVolumeById(params->volume_id);
  if (!volume.get())
    return false;

  // TODO(tbarzic): Send response when callback is received, it would make more
  // sense than remembering issued unmount requests in file manager and showing
  // errors for them when MountCompleted event is received.
  switch (volume->type()) {
    case file_manager::VOLUME_TYPE_REMOVABLE_DISK_PARTITION:
    case file_manager::VOLUME_TYPE_MOUNTED_ARCHIVE_FILE: {
      DiskMountManager::GetInstance()->UnmountPath(
          volume->mount_path().value(), chromeos::UNMOUNT_OPTIONS_NONE,
          DiskMountManager::UnmountPathCallback());
      break;
    }
    case file_manager::VOLUME_TYPE_PROVIDED: {
      chromeos::file_system_provider::Service* service =
          chromeos::file_system_provider::Service::Get(GetProfile());
      DCHECK(service);
      // TODO(mtomasz): Pass a more detailed error than just a bool.
      if (!service->RequestUnmount(volume->extension_id(),
                                   volume->file_system_id())) {
        return false;
      }
      break;
    }
    default:
      // Requested unmounting a device which is not unmountable.
      return false;
  }

  SendResponse(true);
  return true;
}

bool FileManagerPrivateGetVolumeMetadataListFunction::RunAsync() {
  if (args_->GetSize())
    return false;

  const std::vector<base::WeakPtr<file_manager::Volume>>& volume_list =
      file_manager::VolumeManager::Get(GetProfile())->GetVolumeList();

  std::string log_string;
  std::vector<linked_ptr<file_manager_private::VolumeMetadata> > result;
  for (const auto& volume : volume_list) {
    linked_ptr<file_manager_private::VolumeMetadata> volume_metadata(
        new file_manager_private::VolumeMetadata);
    file_manager::util::VolumeToVolumeMetadata(GetProfile(), *volume.get(),
                                               volume_metadata.get());
    result.push_back(volume_metadata);
    if (!log_string.empty())
      log_string += ", ";
    log_string += volume->mount_path().AsUTF8Unsafe();
  }

  drive::EventLogger* logger = file_manager::util::GetLogger(GetProfile());
  if (logger) {
    logger->Log(logging::LOG_INFO,
                "%s[%d] succeeded. (results: '[%s]', %" PRIuS " mount points)",
                name(), request_id(), log_string.c_str(), result.size());
  }

  results_ =
      file_manager_private::GetVolumeMetadataList::Results::Create(result);
  SendResponse(true);
  return true;
}

}  // namespace extensions
