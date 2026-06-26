module;

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../../include/json.hpp"

export module component.hypr;
import std;

using namespace nlohmann;

#define ENV_RUN_TIME_DIR "XDG_RUNTIME_DIR"
#define ENV_SIGNATURE "HYPRLAND_INSTANCE_SIGNATURE"
#define HYPR_DIR "hypr"
#define SOCKET_FILE ".socket.sock"

#define RUN_TIME_MISSING_ERM std::format( "Directory ${} does not exist!", \
                                          ENV_RUN_TIME_DIR )
#define HYPRLAND_INACTIVE_ERM "No instance of Hyprland found!"
#define SOCKET_CREATE_ERM "Could not create socket!"
#define SOCKET_CONNECT_REFUSE_ERM "Could not connect to the Hyprland socket!"

#define GET_OPTION_J "j/getoption"

/**
 * @brief      Builds a path to the hyprland socket file.
 *
 * @return     The a string containing the path to the hyprland socket file.
 */
const std::string build_hypr_path() {
  // Get the runtime directory as well as the signature
  const char* runtime_dir{std::getenv(ENV_RUN_TIME_DIR)};
  const char* signature{std::getenv(ENV_SIGNATURE)};

  // Check that the runtime directory exists.
  if (!runtime_dir)
    throw std::runtime_error(RUN_TIME_MISSING_ERM);

  // Look for an active instance of Hyprland.
  if (!signature)
    throw std::runtime_error(HYPRLAND_INACTIVE_ERM);

  // Finally, combine all part into a final path to the socket file
  return std::format( "{}/{}/{}/{}", 
                      runtime_dir, 
                      HYPR_DIR, 
                      signature, 
                      SOCKET_FILE );
}

void send_command(const std::string& command, std::string& response) {
  const std::string socket_path = build_hypr_path();

  // Attempt to create an IPC socket
  int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socket_fd < 0) 
    throw std::runtime_error(SOCKET_CREATE_ERM);

  // Set the address of the IPC socket to point to the Hyprland socket path
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path)-1);

  // Attempt to connect to the Hyprland socket
  if (connect(socket_fd, 
              reinterpret_cast<struct sockaddr*>(&addr), 
              sizeof(addr)) < 0) {
    // On failure, close the socket and throw an error
    close(socket_fd);
    throw std::runtime_error(SOCKET_CONNECT_REFUSE_ERM);
  }

  // Send the command
  send(socket_fd, command.c_str(), command.length(), 0);

  // Read the response into the response string
  char buffer[4096];
  ssize_t bytes_read = 0;
  while ((bytes_read = read(socket_fd, buffer, sizeof(buffer)-1)) > 0) {
    buffer[bytes_read] = '\0';
    response += buffer;
  }

  // Close the socket communicating with hyprland
  close(socket_fd);
}

namespace hypr {

export auto get_option(const std::string& opt) {
  // Get the json representing the value of the desired option
  std::string out_json;
  send_command(std::format("{} {}", GET_OPTION_J, opt), out_json);

  // Return a parsed json object
  return json::parse(out_json);
}

};