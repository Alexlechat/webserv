*This project has been created as part of the 42 curriculum by anpicard.*

# webserv

## Description

`webserv` is a fully custom HTTP/1.1 server written from scratch in C++98, built around a
single non-blocking `poll()` loop (no threads, no per-client blocking I/O). It serves static
files, accepts uploads, executes CGI scripts, and is driven entirely by an nginx-inspired
configuration file — you can point a real browser at it and it behaves like a small,
real web server.

Core features:
- Non-blocking I/O multiplexed through **one** `poll()` call for every listening socket and
  every client connection (accept/read/write).
- `GET`, `POST`, `DELETE` (and `HEAD`) methods.
- Static file serving, directory listing (`autoindex`), custom index files, and configurable
  default error pages (with generated fallbacks if none are configured).
- File uploads, including real `multipart/form-data` parsing (as sent by an HTML `<form>`).
- CGI execution (`fork`/`pipe`/`execve`) with the standard CGI environment variables, de-chunked
  request bodies on stdin, and a bounded timeout so a broken script can never hang the server.
  Several interpreters can be mapped on one route (`.py` and `.sh` in the sample config); a
  script that produces no output or exits non-zero is reported as `502 Bad Gateway`.
- HTTP redirections, per-location allowed methods, and `client_max_body_size` enforcement.
- Virtual hosting: several `server {}` blocks can share one `listen` port and are
  disambiguated by the `Host` header, the same way nginx does.
- Idle-connection timeouts, so a request can never hang indefinitely.

## Instructions

### Compilation

```sh
make          # builds the `webserv` binary
make clean    # removes build artifacts (object/dependency files)
make fclean   # clean + removes the binary
make re       # fclean + all
make debug    # rebuild with debug symbols (-g3)
```

Compiled with `c++`, flags `-Wall -Wextra -Werror -std=c++98`.

### Running

```sh
./webserv [configuration file]
```

If no configuration file is given, `./www/config.cfg` is used by default. A ready-to-use
example configuration and a matching `www/` document root (static pages, two CGI scripts in
two different languages, an upload directory, error pages, a second virtual host) are
provided so every feature can be exercised immediately:

```sh
./webserv ./www/config.cfg
```

Then, for example:

```sh
curl http://localhost:8080/                    # static site
curl http://localhost:8080/errors/             # directory listing (autoindex)
curl http://localhost:8080/old                 # 301 redirect
curl http://localhost:8080/cgi-bin/hello.py    # CGI, run through python3
curl http://localhost:8080/cgi-bin/info.sh     # CGI, same route, run through bash
curl -F "file=@somefile.txt" http://localhost:8080/upload
curl -X DELETE http://localhost:8080/upload/somefile.txt
curl -H "Host: other" http://localhost:9090/   # second virtual host
```

### Configuration file

The syntax takes inspiration from nginx's `server {}` blocks. Inside a `server` block you can
set: `listen` (either `listen 8080;` or `listen 127.0.0.1:8080;`), `server_name`,
`client_max_body_size`, `error_page`, `error_log` / `access_log`, and any number of
`location {}` blocks. Inside a `location` block: `root`, `index`, `methods`, `autoindex`,
`upload_store`, `upload_return`, `redirect`, and `cgi_extension`. `cgi_extension` may be
repeated to map several extensions to several interpreters on the same route. See
`www/config.cfg` for a complete, commented-in-practice example.

## Resources

- RFC 7230–7235 (HTTP/1.1 message syntax, semantics, conditional requests) and RFC 3875
  (The Common Gateway Interface Specification).
- [MDN HTTP docs](https://developer.mozilla.org/en-US/docs/Web/HTTP) for header/status-code
  reference while implementing the request parser and response builder.
- NGINX documentation (`server`/`location` directive semantics) used as a behavioural
  reference point, and `nginx`/`telnet` were used side-by-side with this server to compare
  real-world header and status-code behaviour during development.
- `man` pages for `poll`, `fork`, `execve`, `pipe`, `dup2`, `waitpid`, `signal`.

### AI usage

An AI assistant (Claude, Anthropic) was used during this project for:
- Reviewing the codebase against the subject's mandatory requirements and pointing out gaps
  and bugs (e.g. missing CGI implementation, unguarded `SIGPIPE`, uninitialized config struct
  members, a path-traversal gap, a broken `access_log` config handler, an unflushed console
  logger, and multipart/form-data uploads never actually being parsed).
- Implementing and iterating on fixes for the issues above, including the CGI execution
  module (`srcs/http/Cgi.cpp`), together with the developer testing each change against a
  running instance of the server (`curl`, raw `nc` requests, and manual review) before
  accepting it.
- Every AI-assisted change was reviewed, tested against a live server, and understood by the
  author before being kept — no code was merged without being explained and verified.

## Known simplifications

- CGI pipe I/O (stdin/stdout of child processes) is fully integrated into `EventLoop`'s
  single global `poll()` loop, with a bounded timeout that kills runaway scripts. This means
  CGI execution is non-blocking and multiplexed alongside all client socket I/O.
