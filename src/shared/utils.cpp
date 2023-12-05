#include "utils.hpp"

#include "protocol.hpp"

// -----------------------------------
// | Extract date and time			 |
// -----------------------------------
std::string extractDate(datetime datetime) {
	std::stringstream out;
	out << datetime.year << "-" << datetime.month << "-" << datetime.day;
	return out.str();
};

std::string extractTime(datetime datetime) {
	std::stringstream out;
	out << datetime.hours << ":" << datetime.minutes << ":" << datetime.seconds;
	return out.str();
}

// -----------------------------------
// | Convert types					 |
// -----------------------------------

uint32_t convert_user_id(std::string string) {
	if (verify_user_id(string) == -1) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_id(std::string string) {
	if (verify_auction_id(string) == -1) {
		throw InvalidMessageException();
	}
	return std::stoi(string);
}

uint32_t convert_auction_value(std::string string) {
	uint32_t value = std::stoi(string);
	if (verify_value(value) == -1) {
		throw InvalidMessageException();
	}
	return value;
}

std::string convert_password(std::string string) {
	if (verify_password(string) == -1) {
		throw InvalidMessageException();
	}
	return string;
}

std::string convert_date_to_str(datetime date) {
	std::string date_str;
	date_str += date.year + ":" + date.month;
	date_str += ":" + date.day;
	date_str += " ";
	date_str += date.hours + ":" + date.minutes;
	date_str += ":" + date.seconds;
	return date_str;
}

// -----------------------------------
// | Reading and writing on files	 |
// -----------------------------------

void sendFile(int connection_fd, std::filesystem::path file_path) {
  std::ifstream file(file_path, std::ios::in | std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file: " << file_path << std::endl;
    throw FileException();
  }

  char buffer[SOCKET_BUFFER_LEN];
  while (file) {
    file.read(buffer, SOCKET_BUFFER_LEN);
    ssize_t bytes_read = (ssize_t)file.gcount();
    ssize_t bytes_sent = 0;
    while (bytes_sent < bytes_read) {
      ssize_t sent = write(connection_fd, buffer + bytes_sent,
                           (size_t)(bytes_read - bytes_sent));
      if (sent < 0) {
        throw MessageSendException();
      }
      bytes_sent += sent;
    }
  }
}


void readAndSaveToFile(const int fd, const std::string &file_name,
                                  const size_t file_size, const std::string directory) {
  std::ofstream file(file_name);

  if (!file.good()) {
    throw FileException();
  }

  size_t remaining_size = file_size;
  size_t to_read;
  ssize_t n;
  char buffer[SOCKET_BUFFER_LEN];

  bool skip_stdin = false;
  while (remaining_size > 0) {
    fd_set file_descriptors;
    FD_ZERO(&file_descriptors);
    FD_SET(fd, &file_descriptors);
    if (!skip_stdin) {
      FD_SET(fileno(stdin), &file_descriptors);
    }

    struct timeval timeout;
    timeout.tv_sec = TCP_READ_TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    int ready_fd = select(std::max(fd, fileno(stdin)) + 1, &file_descriptors,
                          NULL, NULL, &timeout);
    if (ready_fd == -1) {
      perror("select");
      throw ConnectionTimeoutException();
    } else if (FD_ISSET(fd, &file_descriptors)) {
      // Read from socket
      to_read = std::min(remaining_size, (size_t)FILE_BUFFER_LEN);
      n = read(fd, buffer, to_read);
      if (n <= 0) {
        file.close();
        throw InvalidPacketException();
      }
      file.write(buffer, n);
      if (!file.good()) {
        file.close();
        throw IOException();
      }
      remaining_size -= (size_t)n;

      size_t downloaded_size = file_size - remaining_size;
      if (((downloaded_size - (size_t)n) * 100 / file_size) %
              PROGRESS_BAR_STEP_SIZE >
          (downloaded_size * 100 / file_size) % PROGRESS_BAR_STEP_SIZE) {
        std::cout << "Progress: " << downloaded_size * 100 / file_size << "%"
                  << std::endl;
      }
    } else if (FD_ISSET(fileno(stdin), &file_descriptors)) {
      if (std::cin.peek() != '\n') {
        skip_stdin = true;
        continue;
      }
      std::cin.get();
      std::cout << "Cancelling TCP download" << std::endl;
      throw OperationCancelledException();
    } else {
      throw ConnectionTimeoutException();
    }
  }

  file.close();
}

uint32_t getFileSize(std::filesystem::path file_path) {
  try {
    return (uint32_t)std::filesystem::file_size(file_path);
  } catch (...) {
    return -1;
  }
}
