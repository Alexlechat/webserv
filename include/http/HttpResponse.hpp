#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include "config/ServerConfig.hpp"
# include "http/HttpRequest.hpp"

// ─── Entry point ──────────────────────────────────────────────────────────────
// Call this once the request is fully parsed.
// Returns a complete HTTP response string ready to be sent to the client.
std::string buildResponse(const HttpRequest& req, const ServerConfig& config);

// ─── Individual builders ──────────────────────────────────────────────────────
std::string buildErrorResponse(int code, const ServerConfig& config);
std::string buildRedirectResponse(int code, const std::string& location);
std::string buildStaticResponse(const std::string& filepath);
std::string buildAutoindexResponse(const std::string& dirPath,
                                    const std::string& urlPath);
std::string buildUploadResponse(const HttpRequest& req, const LocationConfig& loc, const ServerConfig& config);
std::string buildDeleteResponse(const std::string& filepath);

// ─── Utilities ────────────────────────────────────────────────────────────────
std::string getMimeType(const std::string& filepath);
std::string resolveFilepath(const LocationConfig& loc, const std::string& url);
bool        isMethodAllowed(const LocationConfig& loc, const std::string& method);

#endif // RESPONSE_HPP
