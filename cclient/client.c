#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include "board.h"

#define BUFLEN 512
#define DEFAULT_PORT 8000

#define MODE_UNDEF 0
#define HOST_MODE 1
#define JOIN_MODE 2
#define SPEC_MODE 3
#define PLAYERS_MODE 4
#define GAMES_MODE 5
#define HELP_MODE 6

/*
 * Prints the help section.
 * pname: program name (usually argv[0]).
 */
void print_help(char* pname);

/*
 * Function executed on interuption signal.
 */
void interupt(int s);

/*
 * Function for leaving the server and closing the socket properly.
 */
int silent_leave();

/*
 * Function for command line argument parsing.
 * argc: argument count.
 * argv: argument array.
 */
void getargs(int argc, char** argv);

/*
 * Function for the connection to the server.
 * After its execution, socket_fd contains a file descriptor for the socket.
 */
void server_connect();

/*
 * Function to get the list of the players from the server and print it.
 */
void list_players();

/*
 * Function to get the list of the games from the server and print it.
 */
void list_games();

/*
 * Function to log in. If the login hasn't been set it is asked to the user.
 */
void login_to_server();

/*
 * Function for the host mode.
 */
void host_mode();

/*
 * Function for the join mode.
 */
void join_mode();

/*
 * Function for the spectate mode.
 */
void spec_mode();

/*
 * Some global variables...
 */
int socket_fd;           // Socket file descriptor
char buffer[BUFLEN];     // Buffer
int verbose = 0;         // Verbose level
int mode = MODE_UNDEF;   // Current mode
char* login = NULL;      // Login
char* host = NULL;       // Hostname
char* game = NULL;       // Game name
int port = DEFAULT_PORT; // Port

/*
 * Main function. Parses the arguments, connect to the server, perform the
 * game tasks and leave the server properly.
 */
int main(int argc, char **argv){
  getargs(argc, argv);

  server_connect();

  // Mode selection
  switch(mode){
    case MODE_UNDEF:
      fprintf(stderr, "No mode specified...\n");
      print_help(argv[0]);
      exit(-1);
    case HELP_MODE:
      print_help(argv[0]);
      return 0;
    case PLAYERS_MODE:
      list_players();
      return silent_leave(); // ??
      break;
    case GAMES_MODE:
      list_games();
      return silent_leave(); // ??
      break;
    case HOST_MODE:
      login_to_server();
      host_mode();
      break;
    case JOIN_MODE:
      login_to_server();
      join_mode();
      break;
    case SPEC_MODE:
      login_to_server();
      spec_mode();
      break;
    default:
      break;
  }

  return silent_leave();
}

/*
 * Prints the help section.
 * pname: program name (usually argv[0]).
 */
void print_help(char* pname){
  printf("Usage: %s [OPTIONS]\n", pname);
  printf("OPTIONS\n");
  printf("  -H host    Hostname of the server.\n");
  printf("  -p port    Port number to use.\n");
  printf("  -l login   Login to use.\n");
  printf("  -c game    Create a game with the argument as name.\n");
  printf("  -j game    Join a game.\n");
  printf("  -s game    Spectate a game.\n");
  printf("  -L         List games on the server.\n");
  printf("  -P         List players on the server.\n");
  printf("  -v         Verbose mode.\n");
  printf("  -h         Display this help section.\n");
}

/*
 * Function executed on interuption signal.
 */
void interupt(int s){
  printf("  Interuption signal caught.\nNoticing the server...\n");
  exit(silent_leave());
}

/*
 * Function for leaving the server and closing the socket properly.
 */
int silent_leave(){
  // Leaving
  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "leave.\n");
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  // Closing
  if(0 == close(socket_fd)){
    return 0;
  }else{
    fprintf(stderr, "Error, unable to close the socket...\n");
    exit(-1);
  }
}

/*
 * Function for command line argument parsing.
 * argc: argument count.
 * argv: argument array.
 */
void getargs(int argc, char** argv){
  char opt;

  if(argc == 1){
    print_help(argv[0]);
    exit(-1);
  }

  while((opt = getopt(argc, argv, "H:p:l:c:j:s:LPvh")) != -1){
    switch(opt){
      case 'H':
        host = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 'l':
        login = optarg;
        break;
      case 'c':
        if(mode != MODE_UNDEF){
          fprintf(stderr, "Error, opts -c / -j / -s not compatible...\n");
          exit(-1);
        }
        mode = HOST_MODE;
        game = optarg;
        break;
      case 'j':
        if(mode != MODE_UNDEF){
          fprintf(stderr, "Error, opts -c / -j / -s not compatible...\n");
          exit(-1);
        }
        mode = JOIN_MODE;
        game = optarg;
        break;
      case 's':
        if(mode != MODE_UNDEF){
          fprintf(stderr, "Error, opts -c / -j / -s not compatible...\n");
          exit(-1);
        }
        mode = SPEC_MODE;
        game = optarg;
        break;
      case 'L':
        if(mode != MODE_UNDEF){
          fprintf(stderr, "Error, opts c, j, s, L, P, h not compatible...\n");
          exit(-1);
        }
        mode = GAMES_MODE;
        break;
      case 'P':
        if(mode != MODE_UNDEF){
          fprintf(stderr, "Error, opts c, j, s, L, P, h not compatible...\n");
          exit(-1);
        }
        mode = PLAYERS_MODE;
        break;
      case 'v':
        verbose = 1;
        break;
      case 'h':
        if(mode != MODE_UNDEF){
          fprintf(stderr, "Error, opts c, j, s, L, P, h not compatible...\n");
          exit(-1);
        }
        mode = HELP_MODE;
        break;
      default:
        fprintf(stderr, "Argument error...\n");
        print_help(argv[0]);
        exit(-1);
    }
  }
}

/*
 * Function for the connection to the server.
 * After its execution, socket_fd contains a file descriptor for the socket.
 */
void server_connect(){
  struct sockaddr_in serv_addr;
  struct hostent *server;

  if(verbose)
    printf("Connection to host: %s:%i\n", host, port);
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_fd < 0){
    fprintf(stderr, "Error while opening socket...\n");
    exit(-1);
  }

  server = gethostbyname(host);
  if(NULL == server){
    fprintf(stderr, "Error, no such host...\n");
    exit(-1);
  }

  // Init serv_addr
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(port);

  // Connection
  if(connect(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    fprintf(stderr, "Error while connecting...\n");
    exit(-1);
  } 

  if(verbose)
    printf("Connected.\n");
}

/*
 * Function to get the list of the players from the server and print it.
 */
void list_players(){
  // command to the server
  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "list_players.\n");
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  // Message received
  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  char players[512];
  sscanf(buffer, "%s", players);
  printf("Players connected: %s\n", players);
}

/*
 * Function to get the list of the games from the server and print it.
 */
void list_games(){
  // command to the server
  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "list_games.\n");
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  // Message received
  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  char *newgames, *rungames;
  strtok(buffer, "["); // FIXME problem with empty lists...
  newgames = strtok(NULL, "]");
  strtok(NULL, "[");
  rungames = strtok(NULL, "]");
  printf("New games: %s\nStarted games: %s\n", newgames, rungames);
}

/*
 * Function to log in. If the login hasn't been set it is asked to the user.
 */
void login_to_server(){
  if(NULL == login){
    login = malloc(40 * sizeof(char));
    if(NULL == login){
      fprintf(stderr, "Malloc error...\n");
      exit(-2);
    }
    printf("Login: ");
    scanf("%s", login);
    while(getchar() != '\n');
  }
  if(verbose) printf("Login as %s.\n", login);

  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "{login,\"%s\"}.\n", login);
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  if(strcmp(buffer, "ok\n") == 0){
    if(verbose) printf("Login OK, String from server: %s", buffer);
  }else{
    printf("Login error, received: %s", buffer);
  }

  // From now catch interuption since we are logged in
  signal(SIGINT, interupt);
}

/*
 * Function for the host mode.
 */
void host_mode(){
  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "{host_game,\"%s\"}.\n", game);
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  if(strcmp(buffer, "ok\n") == 0){
    if(verbose) printf("Game created OK, String from server: %s", buffer);
  }else{
    printf("Host game error, received: %s", buffer);
  }

  char ans = 'o';
  do{
    printf("Waiting for players...\n");
    read(socket_fd, buffer, BUFLEN);
    printf("Received: %s", buffer);
    printf("Start the game? (y/n) ");
    ans = getchar();
    while(ans != 'n' && ans != 'y'){
      printf("Wrong answer...\nStart the game? (y/n) ");
      while((ans = getchar ()) != '\n');
      ans = getchar();
    }
  }while(ans != 'y');

  // Start the game
  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "start_game.\n");
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  if(strcmp(buffer, "ok\n") == 0){
    if(verbose) printf("Game started, String from server: %s", buffer);
  }else{
    printf("Game start error, received: %s", buffer);
  }

  // Get the board and players.
  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  char *ind = (char*) memchr(buffer, '#', BUFLEN);
  *(ind - 1) = '\0';
  char *str;
  strtok(buffer, ",");
  str = strtok(NULL, ",");
  printf("You are player N°%i\n", atoi(str));
  str = strtok(NULL, "[");
  str = strtok(NULL, "]");
  printf("Players: %s\n", str);
  print_board(ind);

  // Game loop
  int end = 0;
  char *msg;
  do{
    printf("Waiting for input...\n");
    bzero(buffer, BUFLEN);
    read(socket_fd, buffer, BUFLEN);

    msg = strtok(&buffer[1], ",");
    if(verbose)
      printf("Message received : %s\n", msg);

    if(strcmp(msg, "your_turn") == 0){
      ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      printf("Your next move? ");
      char move[256];
      int l1, l2;
      char c1, c2;
      scanf("%i %c %i %c", &l1, &c1, &l2, &c2);

      bzero(buffer, BUFLEN);
      snprintf(buffer, BUFLEN, "{move,[%i,%i]}.\n", get_array_pos(l1, c1),
                                                    get_array_pos(l2, c2));
      if(verbose) printf("String to server: %s", buffer);
      write(socket_fd, buffer, strlen(buffer));
      // FIXME FIXME
    }else if(strcmp(msg, "update") == 0){
      printf("Board updated:\n");
      ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      // FIXME give more info
    }else if(strcmp(msg, "won") == 0){
      char *winner = strtok(NULL, ",");
      printf("Player %s won the game.\n", winner);
      ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      end = 1;
    }else if(strcmp(msg, "error") == 0){
      int l1, l2;
      char c1, c2;
      printf("Wrong move, try again? ");
      scanf("%i %c %i %c", &l1, &c1, &l2, &c2);

      bzero(buffer, BUFLEN);
      snprintf(buffer, BUFLEN, "{move,[%i,%i]}.\n", get_array_pos(l1, c1),
                                                    get_array_pos(l2, c2));
      if(verbose) printf("String to server: %s", buffer);
      write(socket_fd, buffer, strlen(buffer));
      // FIXME FIXME
    }else{
      fprintf(stderr, "Error, unknown message...\n");
      exit(-1);
    }
  }while(end == 0);
}

/*
 * Function for the join mode.
 */
void join_mode(){
  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "{join_game,\"%s\"}.\n", game);
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  if(strcmp(buffer, "ok\n") == 0){
    if(verbose) printf("Game joined, String from server: %s", buffer);
  }else{
    printf("Join game error, received: %s", buffer);
  }

  printf("Waiting for the game to start...\n");

  // Get the board and players.
  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  char *ind = (char*) memchr(buffer, '#', BUFLEN);
  *(ind - 1) = '\0';
  char *str;
  strtok(buffer, ",");
  str = strtok(NULL, ",");
  printf("You are player N°%i\n", atoi(str));
  str = strtok(NULL, "[");
  str = strtok(NULL, "]");
  printf("Players: %s\n", str);
  print_board(ind);

  // Game loop
  int end = 0;
  char *msg;
  do{
    printf("Waiting for input...\n");
    bzero(buffer, BUFLEN);
    read(socket_fd, buffer, BUFLEN);

    msg = strtok(&buffer[1], ",");
    if(verbose)
      printf("Message received : %s\n", msg);

    if(strcmp(msg, "your_turn") == 0){
      ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      printf("Your next move? ");
      char move[256];
      int l1, l2;
      char c1, c2;
      scanf("%i %c %i %c", &l1, &c1, &l2, &c2);

      bzero(buffer, BUFLEN);
      snprintf(buffer, BUFLEN, "{move,[%i,%i]}.\n", get_array_pos(l1, c1),
                                                    get_array_pos(l2, c2));
      if(verbose) printf("String to server: %s", buffer);
      write(socket_fd, buffer, strlen(buffer));
      // FIXME FIXME
    }else if(strcmp(msg, "update") == 0){
      printf("Board updated:\n");
      ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      // FIXME give more info
    }else if(strcmp(msg, "won") == 0){
      char *winner = strtok(NULL, ",");
      printf("Player %s won the game.\n", winner);
      ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      end = 1;
    }else if(strcmp(msg, "error") == 0){
      int l1, l2;
      char c1, c2;

      printf("Wrong move, try again? ");
      scanf("%i %c %i %c", &l1, &c1, &l2, &c2);

      bzero(buffer, BUFLEN);
      snprintf(buffer, BUFLEN, "{move,[%i,%i]}.\n", get_array_pos(l1, c1),
                                                    get_array_pos(l2, c2));
      if(verbose) printf("String to server: %s", buffer);
      write(socket_fd, buffer, strlen(buffer));
      // FIXME FIXME
    }else{
      fprintf(stderr, "Error, unknown message...\n");
      exit(-1);
    }
  }while(end == 0);
}

/*
 * Function for the spectate mode.
 */
void spec_mode(){
  bzero(buffer, BUFLEN);
  snprintf(buffer, BUFLEN, "{spectate,\"%s\"}.\n", game);
  if(verbose) printf("String to server: %s", buffer);
  write(socket_fd, buffer, strlen(buffer));

  bzero(buffer, BUFLEN);
  read(socket_fd, buffer, BUFLEN);
  if(strcmp(buffer, "ok\n") == 0){
    if(verbose) printf("Spectating game, String from server: %s", buffer);
  }else{
    printf("Spectate error, received: %s", buffer);
  }

  // Game loop
  int end = 0;
  char *msg;
  do{
    printf("Waiting for input...\n");
    bzero(buffer, BUFLEN);
    read(socket_fd, buffer, BUFLEN);

    msg = strtok(&buffer[1], ",");
    if(verbose)
      printf("Message received : %s\n", msg);

    if(strcmp(msg, "update") == 0){
      printf("Board updated:\n");
      char *ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      // FIXME give more info
    }else if(strcmp(msg, "game_state") == 0){
      printf("Board updated:\n");
      char *ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      // FIXME give more info
    }else if(strcmp(msg, "won") == 0){
      char *winner = strtok(NULL, ",");
      printf("Player %s won the game.\n", winner);
      char *ind = (char*) memchr(buffer, '#', BUFLEN);
      print_board(ind);
      end = 1;
    }else{
      fprintf(stderr, "Error, unknown message...\n");
      exit(-1);
    }
  }while(end == 0);
}

