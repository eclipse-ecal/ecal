/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <iostream>

#include "ecal_process_stub.h"

int main(int argc, char** argv)
{
  if (argc <= 1)
  {
    std::cout << "This program is part of eCAL and should not be launched manually." << std::endl;
    return EXIT_FAILURE;
  }
  else if ((argc == 2) && (std::string(argv[1]) == "--version"))
  {
    // POSIX wants us to provide some output when launching with --version. We also use this to determine the correctnis of this application from other applications.
    std::cout << ECAL_PROCESS_STUB_VERSION_STRING << std::endl;
    return EXIT_SUCCESS;
  }
  else if (argc > 3)
  {
    char*        fifo_name          = argv[1];
    char*        lockfile_name      = argv[2];
    int          process_args_start = 3;

    // Create and lock the lockfile. The lockfile will automaticall be
    // closed (and unlocked) when the execvp was successfull or the
    // process exits.
    // We have to do this BEFORE writing the FIFO, because after we have
    // written the PID to the FIFO, the main process will also attempt
    // to lock the lockfile.
    int lockfile_fd = open(lockfile_name, O_RDWR | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (lockfile_fd)
    {
      if (flock(lockfile_fd, LOCK_EX) == -1)
        std::cerr << "Error locking lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
    }
    else
    {
      std::cerr << "Error creating lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
    }

    // Open FIFO and send the PID to the eCAL. The other process will wait until
    // we send the PID, here. It is important to know that although we have a
    // valid PID, we cannot know whether the execvp will be successfull, yet.
    // This is were the lockfile becomes important, as it will implicitely be
    // unlocked when execvp is successfull.
    pid_t process_id = getpid();
    int fifo_fd = open(fifo_name, O_WRONLY);
    if (fifo_fd >= 0)
    {
      if(write(fifo_fd, &process_id, sizeof(process_id)) <= 0)
      {
        std::cerr << "Error writing to FIFO \"" << fifo_name << "\": " << strerror(errno) << std::endl;
      }
      close(fifo_fd);
    }
    else
    {
      std::cerr << "Error opening FIFO \"" << fifo_name << "\": " << strerror(errno) << std::endl;
    }

    // Now call execvp, which will replace this process by the new one, if
    // successfull. In that case, the lockfill will automatically be closed and
    // unlocked.
    const char** c_argv = new const char*[argc - process_args_start + 1];
    for (int i = 0; i < argc - process_args_start; i++)
    {
      c_argv[i] = argv[i + process_args_start];
    }
    c_argv[argc - process_args_start] = nullptr;

    execvp(c_argv[0], (char**)c_argv);

    // ERROR! >> If we have ever reached this code, execvp has not succeeded.
    // Now we have to tell the external main process via the lockfile. We simply
    // write our errno to that file and then exit. This will release the lock
    // and let the main process read the errno.
    std::cerr << "Error executing process " << c_argv[0] << ": " << strerror(errno);

    int errno_int = errno;
    size_t written_bytes = write(lockfile_fd, &errno_int, sizeof(errno_int));
    if(written_bytes == 0)
    {
      std::cerr << "Error writing errno to file \"" << lockfile_name << "\": " << strerror(errno);
    }

    delete[] c_argv;

    return EXIT_FAILURE;
  }
  else
  {
    return EXIT_FAILURE;
  }
}
