# Webserv - Liste des erreurs

## Erreurs critiques

| # | Fichier | Ligne | Description | Detail |
|---|---------|-------|-------------|--------|
| 1 | `srcs/server/EventLoop.cpp` | 282-283 | Double-close de file descriptor | `_removeClient` fait `close(_fds[i].fd)` puis `delete client` qui declenche `~Socket()` fermant le meme fd une deuxieme fois. Undefined behavior : le fd peut etre reassigne entre-temps. |
| 2 | `srcs/http/HttpRequest.cpp` | 117-125 | `_maxBodySize` non verifie pour les body non-chunked | `_tryParseBody()` ne verifie jamais `_maxBodySize`. Seul `_tryParseChunkedBody()` le fait. Un client peut envoyer un body illimite via `Content-Length` et saturer la memoire → crash. |
| 3 | `srcs/server/EventLoop.cpp` | 262-263 | `_handleWrite` ferme la connexion sur EAGAIN | Si `send()` retourne -1 (`EAGAIN`), le client est deconnecte au lieu d'etre reessaye au prochain poll. Reponses partielles sous charge. |
| 4 | `srcs/http/HttpResponseBuilder.cpp` | 550-558 | Pas de limite sur la taille des fichiers servis | `_readFile` charge le fichier entier en memoire. Un fichier de plusieurs Go cause `std::bad_alloc` → crash. |
| 5 | `srcs/http/Cgi.cpp` | 93-124 | Fuite de fd dans le processus CGI enfant | Apres `fork()`, le child herite de tous les fd du parent (sockets serveur, sockets clients, autres pipes). Seuls les pipes du CGI courant sont fermes. |
| 6 | `srcs/server/EventLoop.cpp` | 73 | `exit(1)` dans le constructeur | `exit(1)` au lieu d'un throw. Les `Server*` deja alloues avant l'erreur ne sont jamais `delete` → fuite memoire. |

## Erreurs fonctionnelles

| # | Fichier | Ligne | Description | Detail |
|---|---------|-------|-------------|--------|
| 7 | `srcs/http/HttpResponseBuilder.cpp` | 508 | HEAD ne marche pas si seul GET est dans methods | Le fallback HEAD→GET est commente. Si la config dit `methods GET;`, HEAD retourne 405 alors qu'il devrait etre autorise partout ou GET l'est. |
| 8 | `srcs/http/HttpResponseBuilder.cpp` | 87 | Location matching sans verification de limite de path | Prefix match brut : `/uploadx` matche la location `/upload`. Il faut verifier que le caractere apres le prefix est `/` ou fin de chaine. |
| 9 | `srcs/http/HttpResponseBuilder.cpp` | 463 | `error_pages` dans les blocs location ignorees | `_buildError` cherche les error_pages dans `_config` (ServerConfig) mais pas dans `_location`. Le TODO en `ConfigParser.cpp:76` le confirme. |
| 10 | `include/config/LocationConfig.hpp` | 42-47 | `upload_return` valide comme repertoire au lieu d'URL | `setUploadReturn` appelle `dir_exists()`, mais `upload_return` est un chemin de redirection HTTP (ex: `/`), pas un repertoire disque. |
| 11 | `srcs/config/ConfigParser.cpp` | 94-101 | `error_log` attend obligatoirement 2 arguments | `_handleErrorLog` consomme 2 tokens. Si le config ne donne qu'un argument (`error_log logs/error.log;`), le `;` sera consomme comme min_log_level. |
| 12 | `srcs/config/ConfigParser.cpp` | 114-118 | Pas de support `interface:port` dans `listen` | Le sujet demande de definir des paires interface:port. Le parser ne supporte que `listen <port>`. |
| 13 | `srcs/http/Cgi.cpp` | 37 | PATH_INFO non implemente | `PATH_INFO` est toujours vide. Le sujet exige que la requete complete et ses arguments soient disponibles pour le CGI. |
| 14 | `srcs/server/Client.cpp` | 64-85 | Pas de validation du header `Host` (requis HTTP/1.1) | Le serveur ne renvoie pas 400 quand le header `Host` est absent (RFC 7230 §5.4). |
| 15 | `srcs/server/Client.cpp` | 154 | Pas de keep-alive / pipelining HTTP | `connection: close` est force sur chaque reponse. HTTP/1.1 est keep-alive par defaut. Le serveur devrait supporter des requetes multiples sur une meme connexion. |
| 16 | `srcs/http/HttpResponseBuilder.cpp` | 102 | `_effectiveRoot()` peut retourner une chaine vide | Si ni `root` ni `upload_store` ne sont definis, le path sera relatif au working directory → fichiers non prevus accessibles. |
| 17 | `srcs/http/HttpRequest.cpp` | 40-61 | Pas de validation du format de version HTTP | `_tryParseRequestLine` ne verifie pas que la version est au format `HTTP/x.x`. |
| 18 | `srcs/http/HttpResponseBuilder.cpp` | - | Pas de decodage des URL percent-encoded | Les URLs comme `/path%20with%20spaces` ne sont pas decodees. Fichiers avec caracteres speciaux inaccessibles. |
| 19 | `srcs/http/HttpResponse.cpp` | 50-64 | Pas de header `Date` dans les reponses | HTTP/1.1 (RFC 7231) exige un header `Date` pour les serveurs avec horloge. |

## Erreurs mineures / qualite

| # | Fichier | Ligne | Description | Detail |
|---|---------|-------|-------------|--------|
| 20 | `include/config/Config.hpp` | 77, 93 | Messages d'erreur copy-paste | `"Port out of range"` pour `client_max_body_size` et `error_page code` au lieu de messages adaptes. |
| 21 | `srcs/server/EventLoop.cpp` | 21 | `volatile bool` au lieu de `volatile sig_atomic_t` | Le standard C/C++ ne garantit l'acces atomique dans un signal handler que pour `sig_atomic_t`. |
| 22 | `srcs/server/EventLoop.cpp` | 38 | `std::cout` dans le signal handler | `std::cout` n'est pas async-signal-safe. Appeler `std::cout` dans un signal handler est undefined behavior. |
| 23 | `srcs/http/HttpResponseBuilder.cpp` | 286 | Vulnerabilite XSS dans l'autoindex | Les noms de fichiers sont inseres dans le HTML sans echappement. Un fichier nomme `<script>alert(1)</script>` serait execute. |
| 24 | `srcs/socket/SocketServer.cpp` | 38-47 | `SocketServer::acceptClient()` est du code mort | Methode jamais appelee (EventLoop utilise `accept()` directement). Lance une exception et fait `std::cout` si utilisee. |
| 25 | `README.md` | 1 | `<your_login>` comme placeholder | Le README contient `<your_login>` au lieu du vrai login. |
| 26 | `README.md` | 100-105 | README decrit l'ancienne architecture CGI | Mentionne un "local poll() loop" pour CGI qui n'existe plus dans le code actuel. Trompeur. |
| 27 | `.` | - | `compile_commands.json` dans le repo | Fichier genere par l'IDE, ne devrait pas etre commite. |
| 28 | `logs/` | - | Fichiers de log trackes par git | `access.log` et `error.log` ne devraient pas etre dans le repo. |
| 29 | `www/config.cfg` | 14 | Repertoire `www/uploads` inexistant | `upload_store www/uploads;` reference un dossier qui n'existe pas dans le repo. Le parsing echouera. |
