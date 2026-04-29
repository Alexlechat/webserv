#include "request/Response.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <map>

// ══════════════════════════════════════════════════════════════════════════════
//  Low-level helpers
// ══════════════════════════════════════════════════════════════════════════════

static std::string intToStr(int n)
{
	std::ostringstream ss; ss << n; return ss.str();
}

static std::string statusMsg(int code)
{
	switch (code) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		default:  return "Unknown";
	}
}

// Assemble a complete HTTP response
static std::string makeResponse(int code,
                                const std::string& contentType,
                                const std::string& body,
                                const std::string& extra = "")
{
	std::ostringstream r;
	r << "HTTP/1.1 " << code << " " << statusMsg(code) << "\r\n"
		<< "Content-Type: "   << contentType << "\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "Connection: close\r\n"
		<< extra
		<< "\r\n"
		<< body;
	return r.str();
}

static std::string readFileBinary(const std::string& path)
{
	std::ifstream f(path.c_str(), std::ios::binary);
	if (!f.is_open()) return "";
	std::ostringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

static bool pathExists(const std::string& path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0;
}

static bool isDir(const std::string& path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

// ══════════════════════════════════════════════════════════════════════════════
//  Utilities
// ══════════════════════════════════════════════════════════════════════════════

std::string getMimeType(const std::string& path)
{
	size_t dot = path.rfind('.');
	if (dot == std::string::npos) return "application/octet-stream";
	std::string ext = path.substr(dot);
	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css")                   return "text/css";
	if (ext == ".js")                    return "application/javascript";
	if (ext == ".json")                  return "application/json";
	if (ext == ".png")                   return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif")                   return "image/gif";
	if (ext == ".svg")                   return "image/svg+xml";
	if (ext == ".ico")                   return "image/x-icon";
	if (ext == ".txt")                   return "text/plain";
	if (ext == ".pdf")                   return "application/pdf";
	return "application/octet-stream";
}

// Strip the location prefix from the URL and prepend root.
// loc.path="/kapouet"  loc.root="/tmp/www"  url="/kapouet/pouic"
// → "/tmp/www/pouic"
std::string resolveFilepath(const LocationConfig& loc, const std::string& url)
{
	std::string relative = url.substr(loc.path.size());
	if (relative.empty() || relative[0] != '/')
		relative = "/" + relative;
	return loc.root + relative;
}

bool isMethodAllowed(const LocationConfig& loc, const std::string& method)
{
	if (loc.methods.empty()) return true;
	for (size_t i = 0; i < loc.methods.size(); ++i)
		if (loc.methods[i] == method) return true;
	return false;
}

// ══════════════════════════════════════════════════════════════════════════════
//  Individual response builders
// ══════════════════════════════════════════════════════════════════════════════

// ── Error response ────────────────────────────────────────────────────────────
// Uses the custom error page from config if defined, otherwise built-in HTML.

std::string buildErrorResponse(int code, const ServerConfig& config)
{
	// Look for a custom error page in config
	std::map<int, std::string>::const_iterator it = config.error_pages.find(code);
	if (it != config.error_pages.end() && !config.locations.empty())
	{
		// Error page paths are relative to the first location's root
		std::string filepath = config.locations[0].root + it->second;
		std::string body = readFileBinary(filepath);
		if (!body.empty())
			return makeResponse(code, "text/html", body);
	}

	// Built-in fallback
	std::string msg = intToStr(code) + " " + statusMsg(code);
	std::string body =
		"<!DOCTYPE html><html><head><title>" + msg + "</title>"
		"<style>body{font-family:sans-serif;background:#0f1117;color:#e2e8f0;"
		"display:flex;align-items:center;justify-content:center;height:100vh;margin:0;}"
		".c{text-align:center;} h1{font-size:4rem;color:#fc8181;margin:0;} "
		"p{color:#718096;}</style></head>"
		"<body><div class='c'><h1>" + intToStr(code) + "</h1>"
		"<p>" + statusMsg(code) + "</p></div></body></html>";
	return makeResponse(code, "text/html", body);
}

// ── Redirect response ─────────────────────────────────────────────────────────

std::string buildRedirectResponse(int code, const std::string& location)
{
	std::string body = "<html><body>Redirecting to <a href=\""
	                 + location + "\">" + location + "</a></body></html>";
	return makeResponse(code, "text/html", body,
	                    "Location: " + location + "\r\n");
}

// ── Static file response ──────────────────────────────────────────────────────

std::string buildStaticResponse(const std::string& filepath)
{
	std::string body = readFileBinary(filepath);
	if (body.empty() && !pathExists(filepath))
		return ""; // caller handles 404
	return makeResponse(200, getMimeType(filepath), body);
}

// ── Directory listing (autoindex) ─────────────────────────────────────────────

std::string buildAutoindexResponse(const std::string& dirPath,
                                    const std::string& urlPath)
{
	DIR* dir = opendir(dirPath.c_str());
	if (!dir) return "";

	std::ostringstream html;
	html << "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
	     << "<title>Index of " << urlPath << "</title>"
	     << "<style>body{font-family:monospace;background:#0f1117;color:#e2e8f0;padding:40px;}"
	     << "h1{color:#63b3ed;border-bottom:1px solid #2d3748;padding-bottom:12px;}"
	     << "a{display:block;color:#68d391;padding:4px 0;text-decoration:none;}"
	     << "a:hover{color:#9ae6b4;}</style></head>"
	     << "<body><h1>Index of " << urlPath << "</h1>\n";

	// Always show parent directory link
	if (urlPath != "/")
	{
		size_t slash = urlPath.rfind('/', urlPath.size() - 2);
		std::string parent = (slash == std::string::npos) ? "/" : urlPath.substr(0, slash + 1);
		html << "<a href=\"" << parent << "\">../</a>\n";
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..") continue;

		bool dir_ = isDir(dirPath + "/" + name);
		std::string href = urlPath;
		if (href[href.size() - 1] != '/') href += "/";
		href += name;
		if (dir_) { name += "/"; href += "/"; }

		html << "<a href=\"" << href << "\">" << name << "</a>\n";
	}
	closedir(dir);
	html << "</body></html>";

	return makeResponse(200, "text/html", html.str());
}

// ── File upload (POST with multipart/form-data) ───────────────────────────────
//
// Parses a multipart body to extract the file content and save it to disk.
// The boundary is taken from the Content-Type header.

static std::string extractBoundary(const HttpRequest& req)
{
	std::map<std::string, std::string>::const_iterator it =
		req.headers.find("content-type");
	if (it == req.headers.end()) return "";

	const std::string& ct = it->second;
	size_t pos = ct.find("boundary=");
	if (pos == std::string::npos) return "";
	return ct.substr(pos + 9);
}

static std::string extractFilename(const std::string& disposition)
{
	size_t pos = disposition.find("filename=\"");
	if (pos == std::string::npos) return "upload";
	pos += 10;
	size_t end = disposition.find('"', pos);
	if (end == std::string::npos) return "upload";
	return disposition.substr(pos, end - pos);
}

std::string buildUploadResponse(const HttpRequest& req, const LocationConfig& loc)
{
	std::string boundary = extractBoundary(req);
	if (boundary.empty() || loc.upload_store.empty())
		return buildErrorResponse(400, ServerConfig());

	std::string delim = "--" + boundary;
	const std::string& body = req.body;

	size_t pos = body.find(delim);
	if (pos == std::string::npos)
		return buildErrorResponse(400, ServerConfig());

	// Walk through each part
	while (true)
	{
		pos += delim.size();
		if (pos + 2 > body.size()) break;

		// "--" at start of boundary means end
		if (body.substr(pos, 2) == "--") break;

		// Skip \r\n after boundary
		pos += 2;

		// Read part headers
		size_t headerEnd = body.find("\r\n\r\n", pos);
		if (headerEnd == std::string::npos) break;

		std::string partHeaders = body.substr(pos, headerEnd - pos);
		std::string filename = "upload";
		if (partHeaders.find("filename") != std::string::npos)
			filename = extractFilename(partHeaders);

		// Part body starts after \r\n\r\n
		size_t bodyStart = headerEnd + 4;
		size_t bodyEnd   = body.find("\r\n" + delim, bodyStart);
		if (bodyEnd == std::string::npos) break;

		std::string fileContent = body.substr(bodyStart, bodyEnd - bodyStart);

		// Write file to upload_store
		std::string filepath = loc.upload_store + "/" + filename;
		std::ofstream out(filepath.c_str(), std::ios::binary);
		if (!out.is_open())
			return buildErrorResponse(500, ServerConfig());
		out.write(fileContent.c_str(), fileContent.size());
		out.close();

		pos = bodyEnd + 2; // skip \r\n before next boundary
	}

	std::string body201 =
		"<html><body><h1>201 Created</h1><p>File uploaded successfully.</p>"
		"<a href='/'>Back</a></body></html>";
	return makeResponse(201, "text/html", body201);
}

// ── DELETE handler ────────────────────────────────────────────────────────────

std::string buildDeleteResponse(const std::string& filepath)
{
	if (!pathExists(filepath))
		return ""; // caller sends 404

	if (remove(filepath.c_str()) != 0)
		return ""; // caller sends 500

	// 204 No Content — success with no body
	return "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n";
}

// ══════════════════════════════════════════════════════════════════════════════
//  Main entry point — routes the request to the right builder
// ══════════════════════════════════════════════════════════════════════════════

// Find the most specific matching LocationConfig (longest prefix match)
static const LocationConfig* findLocation(const ServerConfig& config,
                                           const std::string& url)
{
	const LocationConfig* best    = NULL;
	size_t                bestLen = 0;

	for (size_t i = 0; i < config.locations.size(); ++i)
	{
		const std::string& lp = config.locations[i].path;
		if (url.compare(0, lp.size(), lp) != 0)
			continue;
		// Prevent "/uploads" matching location "/upload"
		if (url.size() > lp.size()
		    && url[lp.size()] != '/'
		    && lp[lp.size() - 1] != '/')
			continue;
		if (lp.size() > bestLen) { best = &config.locations[i]; bestLen = lp.size(); }
	}
	return best;
}

std::string buildResponse(const HttpRequest& req, const ServerConfig& config)
{
	// ── 1. Find matching location ─────────────────────────────────────────────
	const LocationConfig* loc = findLocation(config, req.path);
	if (!loc)
		return buildErrorResponse(404, config);

	// ── 2. Check redirect ─────────────────────────────────────────────────────
	if (loc->redirect_code != 0)
		return buildRedirectResponse(loc->redirect_code, loc->redirect_url);

	// ── 3. Check method is allowed ────────────────────────────────────────────
	if (!isMethodAllowed(*loc, req.method))
		return buildErrorResponse(405, config);

	// ── 4. Check request body size ────────────────────────────────────────────
	if (req.body.size() > config.client_max_body_size)
		return buildErrorResponse(413, config);

	// ── 5. Route by method ────────────────────────────────────────────────────

	// POST → file upload
	if (req.method == "POST")
	{
		if (!loc->upload_store.empty())
			return buildUploadResponse(req, *loc);
		// POST to CGI would be handled here later
		return buildErrorResponse(405, config);
	}

	// DELETE → remove file
	if (req.method == "DELETE")
	{
		std::string filepath = resolveFilepath(*loc, req.path);
		std::string resp = buildDeleteResponse(filepath);
		if (resp.empty())
			return buildErrorResponse(pathExists(filepath) ? 500 : 404, config);
		return resp;
	}

	// GET → serve file or directory
	if (req.method == "GET")
	{
		std::string filepath = resolveFilepath(*loc, req.path);

		// Directory handling
		if (isDir(filepath))
		{
			// Try the index file first
			if (!loc->index.empty())
			{
				std::string indexPath = filepath;
				if (indexPath[indexPath.size() - 1] != '/') indexPath += "/";
				indexPath += loc->index;
				if (pathExists(indexPath))
					return buildStaticResponse(indexPath);
			}

			// No index — autoindex or 403
			if (loc->autoindex)
				return buildAutoindexResponse(filepath, req.path);

			return buildErrorResponse(403, config);
		}

		// Regular file
		std::string resp = buildStaticResponse(filepath);
		if (resp.empty())
			return buildErrorResponse(404, config);
		return resp;
	}

	return buildErrorResponse(400, config);
}
