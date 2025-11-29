Initiating project - working on the branch tsomacha-experiment
    Always push changes to feature branch

link to the book -
    https://it-ebooks.dev/books/programming/beejs-guide-to-network-programming

### What is a socket?
- a way to speak to other programs using standard Unix file descriptors
- file can be a network connection, a FIFO, a pipe, a terminal, a real on-the-disk file, or just about anything else. Everything in Unix is a file!
- make a call to the socket() system routin
- It returns the socket descriptor
- communicate through it using the specialized send() and recv() socket calls.

### Types of Internet Sockets
* `SOCK_RAW` - Raw Sockets
* `SOCK_STREAM` - Stream Sockets
* `SOCK_DGRAM` - Datagram Sockets

more info
- TCP https://datatracker.ietf.org/doc/html/rfc793
- IP https://datatracker.ietf.org/doc/html/rfc791
- UDP https://datatracker.ietf.org/doc/html/rfc768

A layered model more consistent with Unix might be :

[ ETHERNET [ IP [ UDP [ TFTP [ DATA ] ] ] ] ]

* Application Layer (telnet, ftp, etc.)
* Host-to-Host Transport Layer (TCP, UDP)
* Internet Layer (IP and routing)
* Network Access Layer (Ethernet, wi-fi, or whatever)

### Byte Order
* Network Byte Order
  - stored in memory as the sequential bytes with the big end first
  - storage method is called Big-Endian
* Host Byte Order
  - stored in memory as the sequential bytes with the big end first
  - storage method is called Little-Endian

- convert the numbers to Network Byte Order before they go out on the wire
- convert them to Host Byte Order as they come in off the wire

    * htons - host to network short
    * htonl - host to network long
    * ntohs - network to host short
    * ntohl - network to host long

### struct addrinfo

```C
    struct addrinfo {
                int ai_flags;               // AI_PASSIVE, AI_CANONNAME, etc.
                int ai_family;              // AF_INET, AF_INET6, AF_UNSPEC
                int ai_socktype;            // SOCK_STREAM, SOCK_DGRAM
                int ai_protocol;            // use 0 for "any"
                size_t ai_addrlen;          // size of ai_addr in bytes
                struct sockaddr *ai_addr;   // struct sockaddr_in or _in6
                char* ai_canonname;         // full canonical hostname
                struct addrinfo *ai_next;   // linked list, next node
    };
```

- call getaddrinfo() will return a pointer to a new linked list of these structures filled out with all the goodies

### struct sockaddr

```C
    struct sockaddr {
        unsigned short  sa_family;  // address family, AF_xxx
        char    sa_data[14];        // 14 bytes of protocol addressChapter
    };
```

### struct sockaddr_in (IPv4)

```C
    struct sockaddr_in {
        short int sin_family;           // Address family, AF_INET
        unsigned short int sin_port;    // Port number
        struct in_addr sin_addr;        // Internet address
        unsigned char sin_zero[8];      // Same size as struct sockaddr
    };
```
- connect() expect struct sockaddr* still a pointer struct sockaddr_in* can cast to a struct sockaddr* anytime and vice-versa

- sin_zero need to set zero using memset()
- sa_family should be set to AF_INET
- sin_port must be Network Byte Order (using host to network short)

```C
    struct in_addr {
        uint32_t s_addr;            // that's a 32-bit int (4 bytes)
    };
```

### struct sockaddr_in6 (IPv6)

```C
    struct sockaddr_in6 {
        u_int16_t   sin6_family;    // address family, AF_INET6
        u_int16_t   sin6_port;      // port, Network Byte Order
        u_int32_t   sin6_flowinfo;  // IPv6 flow information
        struct in6_addr sin6_addr;  // IPv6 address
        u_int32_t   sin6_scope_id;  // Scope ID
    };
```
- connect() expect struct sockaddr* still a pointer struct sockaddr_in6* can cast to a struct sockaddr* anytime and vice-versa

```C
    struct in6_addr {
        unsigned char s6_addr[16];  // IPv6 address
    };
```
### struct sockaddr_storage (IPv4 and IPv6)

```C
    struct sockaddr_storage {
        sa_family_t ss_family;  // address family

        // all this is padding, implementation specific, ignore it:
        char    __ss_pad1[_SS_PAD1SIZE];
        int64_t __ss_align;
        char    __ss_pad2[_SS_PAD2SIZE];
    };
```
- ss_family can be AF_INET or AF_INET6 (for IPv4 or IPv6)
- connect() expect struct sockaddr* still a pointer struct sockaddr_storage* can cast to a struct sockaddr* anytime and vice-versa

### pton

- presentation to network
- inet_pton() need to implement own version of this function
- converts an IP address in numbers-and-dots notation into either a struct in_addr or a struct in6_addr
depending on whether you specify AF_INET or AF_INET6
- inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr));
- inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr));

### ntop

- network to presentation
- inet_ntop() need to implement own version of this function
- converts a binary representaion of an address into IP address in numbers-and-dots notation
- inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
- inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);

# List of allowed functions

#### execve
- Headers: `<unistd.h>`
- Prototype: `int execve(const char *pathname, char *const argv[], char *const envp[]);`
- Description: Replace the current process image with a new program.
- Params: `pathname` (path to executable), `argv` (NULL-terminated args), `envp` (NULL-terminated env).
- Returns: Only on error: `-1` and sets `errno`. On success, no return (process image replaced).
- Notes/Errors: Common `errno`: `EACCES`, `ENOENT`, `ENOMEM`, `ETXTBSY`, `E2BIG`.

#### dup
- Headers: `<unistd.h>`
- Prototype: `int dup(int oldfd);`
- Description: Duplicate an open file descriptor to the lowest-numbered available fd.
- Params: `oldfd` (existing descriptor).
- Returns: New fd (≥0) on success; `-1` on error.
- Notes/Errors: `EBADF`, `EMFILE`.

#### dup2
- Headers: `<unistd.h>`
- Prototype: `int dup2(int oldfd, int newfd);`
- Description: Duplicate `oldfd` onto `newfd` (closes `newfd` first if open).
- Params: `oldfd`, `newfd`.
- Returns: `newfd` on success; `-1` on error.
- Notes/Errors: `EBADF`, `EMFILE`, `EINTR`.

#### pipe
- Headers: `<unistd.h>`
- Prototype: `int pipe(int pipefd[2]);`
- Description: Create a unidirectional data channel.
- Params: `pipefd[0]` read end, `pipefd[1]` write end.
- Returns: `0` on success; `-1` on error.
- Notes/Errors: `EMFILE`, `ENFILE`.

#### strerror
- Headers: `<string.h>`
- Prototype: `char *strerror(int errnum);`
- Description: Convert `errno` number to a human-readable string.
- Params: `errnum` (error code).
- Returns: Pointer to static string.
- Notes/Errors: Not thread-safe; prefer `strerror_r` if available.

#### gai_strerror
- Headers: `<netdb.h>`
- Prototype: `const char *gai_strerror(int ecode);`
- Description: Convert `getaddrinfo()` error code to a string.
- Params: `ecode` (from `getaddrinfo`).
- Returns: Constant string.

#### errno
- Headers: `<errno.h>`
- Identifier: `extern int errno;`
- Description: Thread-local error number set by failing syscalls/library calls.

#### fork
- Headers: `<unistd.h>`
- Prototype: `pid_t fork(void);`
- Description: Create a child process.
- Returns: In parent: child PID (≥0). In child: `0`. On error: `-1`.
- Notes/Errors: `EAGAIN`, `ENOMEM`.

#### socketpair
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int socketpair(int domain, int type, int protocol, int sv[2]);`
- Description: Create a pair of connected sockets.
- Params: `domain` (e.g., `AF_UNIX`), `type` (e.g., `SOCK_STREAM`), `protocol` (0), `sv` (out fds).
- Returns: `0` on success; `-1` on error.

#### htons / htonl / ntohs / ntohl
- Headers: `<arpa/inet.h>`
- Prototypes:
  `uint16_t htons(uint16_t hostshort);`
  `uint32_t htonl(uint32_t hostlong);`
  `uint16_t ntohs(uint16_t netshort);`
  `uint32_t ntohl(uint32_t netlong);`
- Description: Host ↔ network byte-order conversion (big-endian).
- Returns: Converted value.

#### select
- Headers: `<sys/select.h>` or `<sys/time.h> <sys/types.h> <unistd.h>`
- Prototype: `int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);`
- Description: Monitor multiple fds for readiness.
- Params: `nfds` (max fd + 1), `fd_set`s, `timeout` (NULL = block).
- Returns: Number of ready fds; `0` on timeout; `-1` on error.
- Notes/Errors: `EINTR`, `EBADF`. Macros: `FD_ZERO`, `FD_SET`, `FD_ISSET`.

#### poll
- Headers: `<poll.h>`
- Prototype: `int poll(struct pollfd fds[], nfds_t nfds, int timeout);`
- Description: I/O readiness like `select` with array of `pollfd`.
- Returns: Count of ready fds; `0` on timeout; `-1` on error.
- Notes/Errors: `EINTR`. `timeout` in ms (`-1` blocks).

#### epoll_create / epoll_ctl / epoll_wait
- Headers: `<sys/epoll.h>`
- Prototypes:
  `int epoll_create(int size);`
  `int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);`
  `int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);`
- Description: Scalable event polling API (Linux).
- Returns: `epoll_create`: epoll fd or `-1`.
  `epoll_ctl`: `0` or `-1`.
  `epoll_wait`: number of events ≥0, `-1` on error.
- Notes/Errors: `op` is `EPOLL_CTL_ADD|MOD|DEL`; `events` uses `EPOLLIN`, `EPOLLOUT`, etc.

#### kqueue / kevent
- Headers: `<sys/types.h> <sys/event.h> <sys/time.h>`
- Prototypes:
  `int kqueue(void);`
  `int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);`
- Description: Kernel event queue API (BSD/macOS).
- Returns: `kqueue`: fd or `-1`. `kevent`: number of events, `0` on timeout, `-1` on error.

#### socket
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int socket(int domain, int type, int protocol);`
- Description: Create an endpoint for communication.
- Params: `domain` (`AF_INET`, `AF_INET6`, `AF_UNIX`), `type` (`SOCK_STREAM`, `SOCK_DGRAM`), `protocol` (usually `0`).
- Returns: Socket fd or `-1`.

#### accept
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);`
- Description: Accept a pending connection on a listening socket.
- Returns: New connected socket fd or `-1`.
- Notes/Errors: `addr`/`addrlen` may be `NULL`. `EAGAIN`/`EWOULDBLOCK` with nonblocking sockets.

#### listen
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int listen(int sockfd, int backlog);`
- Description: Mark socket as passive (server) to accept connections.
- Returns: `0` or `-1`.

#### send
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `ssize_t send(int sockfd, const void *buf, size_t len, int flags);`
- Description: Send data on a connected socket.
- Returns: Bytes sent (≥0) or `-1`.
- Notes/Errors: May send fewer than `len`. `EPIPE` may raise `SIGPIPE` unless `MSG_NOSIGNAL`.

#### recv
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `ssize_t recv(int sockfd, void *buf, size_t len, int flags);`
- Description: Receive data from a connected socket.
- Returns: Bytes received (0 means peer closed) or `-1`.

#### chdir
- Headers: `<unistd.h>`
- Prototype: `int chdir(const char *path);`
- Description: Change the current working directory.
- Returns: `0` or `-1`.

#### bind
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);`
- Description: Assign a local address to a socket.
- Returns: `0` or `-1`.
- Notes/Errors: `EADDRINUSE` if port in use.

#### connect
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);`
- Description: Initiate a connection on a socket.
- Returns: `0` or `-1` (`EINPROGRESS` for nonblocking).

#### getaddrinfo
- Headers: `<sys/types.h> <sys/socket.h> <netdb.h>`
- Prototype: `int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);`
- Description: Convert host/service to address structures.
- Returns: `0` on success; nonzero `EAI_*` code on failure.
- Notes: Free results with `freeaddrinfo`.

#### freeaddrinfo
- Headers: `<netdb.h>`
- Prototype: `void freeaddrinfo(struct addrinfo *res);`
- Description: Free list from `getaddrinfo`.

#### setsockopt
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);`
- Description: Set options on a socket (e.g., `SO_REUSEADDR`, `SO_KEEPALIVE`).
- Returns: `0` or `-1`.

#### getsockname
- Headers: `<sys/types.h> <sys/socket.h>`
- Prototype: `int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);`
- Description: Get the current address to which the socket is bound.
- Returns: `0` or `-1`.

#### getprotobyname
- Headers: `<netdb.h>`
- Prototype: `struct protoent *getprotobyname(const char *name);`
- Description: Look up a protocol (e.g., `"tcp"`, `"udp"`).
- Returns: Pointer or `NULL`.
- Notes: Not thread-safe (static storage).

#### fcntl
- Headers: `<fcntl.h>`
- Prototype: `int fcntl(int fd, int cmd, ...);`
- Description: Manipulate fd flags and modes.
- Returns: Command-dependent; `-1` on error.

#### close
- Headers: `<unistd.h>`
- Prototype: `int close(int fd);`
- Description: Close an open file descriptor.
- Returns: `0` or `-1`.

#### read
- Headers: `<unistd.h>`
- Prototype: `ssize_t read(int fd, void *buf, size_t count);`
- Description: Read up to `count` bytes into `buf`.
- Returns: Bytes read (0 = EOF), or `-1`.

#### write
- Headers: `<unistd.h>`
- Prototype: `ssize_t write(int fd, const void *buf, size_t count);`
- Description: Write up to `count` bytes from `buf`.
- Returns: Bytes written or `-1`.

#### waitpid
- Headers: `<sys/types.h> <sys/wait.h>`
- Prototype: `pid_t waitpid(pid_t pid, int *wstatus, int options);`
- Description: Wait for state changes in child process.
- Returns: Child PID, `0` (with `WNOHANG`), or `-1`.

#### kill
- Headers: `<signal.h> <sys/types.h>`
- Prototype: `int kill(pid_t pid, int sig);`
- Description: Send a signal to a process/process group.
- Returns: `0` or `-1`.

#### signal
- Headers: `<signal.h>`
- Prototype: `void (*signal(int signum, void (*handler)(int)))(int);`
- Description: Set a signal handler.
- Returns: Previous handler on success; `SIG_ERR` on error.

#### access
- Headers: `<unistd.h>`
- Prototype: `int access(const char *pathname, int mode);`
- Description: Check real-user permissions/availability.
- Returns: `0` or `-1`.

#### stat
- Headers: `<sys/types.h> <sys/stat.h> <unistd.h>`
- Prototype: `int stat(const char *path, struct stat *buf);`
- Description: Get file status (size, mode, times).
- Returns: `0` or `-1`.

#### open
- Headers: `<fcntl.h> <sys/types.h> <sys/stat.h>`
- Prototype: `int open(const char *pathname, int flags, ...);`
- Description: Open or create a file.
- Returns: fd or `-1`.

#### opendir
- Headers: `<dirent.h>`
- Prototype: `DIR *opendir(const char *name);`
- Description: Open a directory stream.
- Returns: `DIR*` or `NULL`.

#### readdir
- Headers: `<dirent.h>`
- Prototype: `struct dirent *readdir(DIR *dirp);`
- Description: Read next directory entry.
- Returns: Pointer to static `struct dirent` or `NULL`.

#### closedir
- Headers: `<dirent.h>`
- Prototype: `int closedir(DIR *dirp);`
- Description: Close a directory stream.
- Returns: `0` or `-1`.
