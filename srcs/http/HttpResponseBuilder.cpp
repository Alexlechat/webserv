#include "http/Http.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpResponseBuilder.hpp"
#include "utils/Utils.hpp"

#include <vector>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include <sys/stat.h>

HttpResponse	buildResponse(const HttpRequest& request, const Server& server)
{
	return HttpResponseBuilder(request, server).build();
}

HttpResponseBuilder::HttpResponseBuilder(const HttpRequest& request, const Server& server)
	: _request(request)
	, _server(server)
	, _config(_server.getServerConfig())
	, _location(NULL)
{}

HttpResponse	HttpResponseBuilder::build(void)
{
	_location = _matchLocation();
	if (!_location)
		return _buildError(Http::NOT_FOUND);

	if (_config.client_max_body_size > 0 && _request.body.size() > _config.client_max_body_size)
		return _buildError(Http::CONTENT_TOO_LARGE);

	if (_location->redirect.first != 0)
	{
		HttpResponse	resp((Http::StatusCode)_location->redirect.first);
		resp.setHeader("location", _location->redirect.second);
		return resp;
	}

	if (!_isMethodAllowed())
		return _buildError(Http::METHOD_NOT_ALLOWED);

	HttpResponse	resp;
	switch (_request.methodEnum)
	{
		case Http::GET:
		case Http::HEAD:	resp = _handleGet();	break;
		case Http::POST:	resp = _handlePost();	break;
		case Http::DELETE:	resp = _handleDelete();	break;
		default:			return _buildError(Http::NOT_IMPLEMENTED);
	}

	if (_request.methodEnum == Http::HEAD)
		resp.clearBody();

	return resp;
}

const LocationConfig*	HttpResponseBuilder::_matchLocation(void) const
{
	const LocationConfig*	best    = NULL;
	size_t					bestLen = 0;

	for (size_t i = 0; i < _config.locations.size(); ++i)
	{
		const std::string&	locPath = _config.locations[i].path;
		if (_request.path.compare(0, locPath.size(), locPath) == 0 && locPath.size() > bestLen)
		{
			bestLen = locPath.size();
			best    = &_config.locations[i];
		}
	}

	return best;
}

std::string	HttpResponseBuilder::_resolvePath(void) const
{
	std::string	rel = _request.path.substr(_location->path.size());

	std::string	fsPath = _location->root;
	if (!fsPath.empty() && fsPath[fsPath.size() - 1] != '/')
		fsPath += '/';
	if (!rel.empty() && rel[0] == '/')
		rel = rel.substr(1);

	return fsPath + rel;
}

HttpResponse	HttpResponseBuilder::_handleGet(void)
{
	std::string	fsPath = _resolvePath();

	struct stat	st;
	if (stat(fsPath.c_str(), &st) != 0)
		return _buildError(Http::NOT_FOUND);

	if (S_ISDIR(st.st_mode))
		return _serveDirectory(fsPath);

	std::string	interpreter;
	if (_matchCgi(fsPath, interpreter))
		return _executeCgi(fsPath, interpreter);

	return _serveFile(fsPath);
}

HttpResponse	HttpResponseBuilder::_handlePost(void)
{
	std::string	fsPath = _resolvePath();

	struct stat	st;
	if (stat(fsPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
	{
		std::string	interpreter;
		if (_matchCgi(fsPath, interpreter))
			return _executeCgi(fsPath, interpreter);
	}

	if (!_location->upload_store.empty())
		return _handleUpload();

	return _buildError(Http::METHOD_NOT_ALLOWED);
}

HttpResponse	HttpResponseBuilder::_handleDelete(void)
{
	std::string	fsPath = _resolvePath();
	
	struct stat	st;
	if (stat(fsPath.c_str(), &st) != 0)
		return _buildError(Http::NOT_FOUND);
	if (S_ISDIR(st.st_mode))
		return _buildError(Http::FORBIDDEN);
	if (std::remove(fsPath.c_str()) != 0)
		return _buildError(Http::FORBIDDEN);

	return HttpResponse(Http::NO_CONTENT);
}

HttpResponse	HttpResponseBuilder::_serveFile(const std::string& fsPath)
{
	bool	ok;
	std::string	content = _readFile(fsPath, ok);
	if (!ok)
		return _buildError(Http::NOT_FOUND);

	HttpResponse	resp(Http::OK);
	resp.setBody(content, _getContentType(fsPath));
	return resp;
}

HttpResponse	HttpResponseBuilder::_serveDirectory(const std::string& fsPath)
{
	if (_request.path.empty() || _request.path[_request.path.size() - 1] != '/')
	{
		HttpResponse	resp(Http::MOVED_PERMANENTLY);
		resp.setHeader("location", _request.path + "/");
		return resp;
	}

	if (!_location->index.empty())
	{
		std::string	indexPath = fsPath;
		if (indexPath[indexPath.size() - 1] != '/') indexPath += '/';
		indexPath += _location->index;

		struct stat	st;
		if (stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
		{
			std::string	interpreter;
			if (_matchCgi(indexPath, interpreter))
				return _executeCgi(indexPath, interpreter);
			return _serveFile(indexPath);
		}
	}

	if (_location->autoindex)
		return _autoIndex(fsPath);

	return _buildError(Http::FORBIDDEN);
}

HttpResponse	HttpResponseBuilder::_autoIndex(const std::string& fsPath) const
{
	DIR*	dir = opendir(fsPath.c_str());
	if (!dir)
		return _buildError(Http::FORBIDDEN);

	std::vector<std::string>	entries;
	struct dirent*				de;
	while ((de = readdir(dir)) != NULL)
	{
		std::string	name = de->d_name;
		if (name == "." || name == "..") continue;

		struct stat	st;
		if (stat((fsPath + name).c_str(), &st) == 0 && S_ISDIR(st.st_mode))
			name += '/';
		entries.push_back(name);
	}
	closedir(dir);
	std::sort(entries.begin(), entries.end());

	std::ostringstream	html;
	html << "<!DOCTYPE html>\n<html>\n<head><title>Index of "
		 << _request.path << "</title></head\n<body>\n"
		 << "<h1>Index of " << _request.path << "</h1><hr><pre>\n";

	if (_request.path != "/")
		html << "<a href=\"../\">../</a>\n";

	for (size_t i = 0; i < entries.size(); ++i)
		html << "<a href=\"" << entries[i] << "\">" << entries[i] << "</a>\n";

	html << "</pre><hr></body></html>\n";

	HttpResponse	resp(Http::OK);
	resp.setBody(html.str(), "text/html");
	return resp;
}

HttpResponse HttpResponseBuilder::_handleUpload(void)
{
    std::map<std::string, std::string>::const_iterator ct =
        _request.headers.find("content-type");

    if (ct == _request.headers.end())
        return _buildError(Http::BAD_REQUEST);

    if (ct->second.find("multipart/form-data") != std::string::npos)
    {
        size_t bpos = ct->second.find("boundary=");
        if (bpos == std::string::npos)
            return _buildError(Http::BAD_REQUEST);

        std::string boundary = ct->second.substr(bpos + 9);
        // trim any trailing whitespace that some clients append
        size_t end = boundary.find_first_of(" \t\r\n");
        if (end != std::string::npos) boundary = boundary.substr(0, end);

        return _handleMultipartUpload(boundary);
    }

    return _handleRawUpload();
}

// Splits a multipart body into its constituent parts, each with their own
// parsed headers and body. Boundary is the raw value from Content-Type
// (without the leading "--").
std::vector<HttpResponseBuilder::MultipartPart>
HttpResponseBuilder::_parseMultipart(const std::string& boundary) const
{
    std::vector<MultipartPart> parts;
    std::string delim = "--" + boundary;

    size_t pos = _request.body.find(delim);
    if (pos == std::string::npos) return parts;

    while (true)
    {
        pos += delim.size();
        if (pos + 2 > _request.body.size()) break;

        // "--" after the boundary means the final delimiter
        if (_request.body[pos] == '-' && _request.body[pos + 1] == '-') break;

        // skip the CRLF that follows the boundary line
        if (_request.body[pos] == '\r') pos += 2;
        else break;

        size_t headerEnd = _request.body.find("\r\n\r\n", pos);
        if (headerEnd == std::string::npos) break;

        MultipartPart part;

        // parse this part's headers
        size_t hPos = pos;
        while (hPos < headerEnd)
        {
            size_t lineEnd = _request.body.find("\r\n", hPos);
            if (lineEnd == std::string::npos || lineEnd > headerEnd) break;

            size_t colon = _request.body.find(':', hPos);
            if (colon != std::string::npos && colon < lineEnd)
            {
                std::string key = Utils::strToLower(Utils::trim(
                    _request.body.substr(hPos, colon - hPos), Http::SPACE_CHARS));
                std::string val = Utils::trim(
                    _request.body.substr(colon + 1, lineEnd - colon - 1), Http::SPACE_CHARS);
                part.headers[key] = val;
            }
            hPos = lineEnd + 2;
        }

        size_t bodyStart = headerEnd + 4;

        // the part body ends just before the next boundary (always preceded by CRLF)
        size_t nextBoundary = _request.body.find("\r\n" + delim, bodyStart);
        if (nextBoundary == std::string::npos) break;

        part.body = _request.body.substr(bodyStart, nextBoundary - bodyStart);
        parts.push_back(part);

        pos = nextBoundary + 2;   // step to the "--boundary" line
    }

    return parts;
}

HttpResponse HttpResponseBuilder::_handleMultipartUpload(const std::string& boundary)
{
    std::vector<MultipartPart> parts = _parseMultipart(boundary);
    if (parts.empty())
        return _buildError(Http::BAD_REQUEST);

    HttpResponse lastResp = _buildError(Http::BAD_REQUEST);
    bool savedAny = false;

    for (size_t i = 0; i < parts.size(); ++i)
    {
        std::map<std::string, std::string>::const_iterator cd =
            parts[i].headers.find("content-disposition");
        if (cd == parts[i].headers.end()) continue;

        // only process parts that carry a filename (i.e. file fields, not text fields)
        std::string filename = _extractFilename(cd->second);
        if (filename.empty()) continue;

        lastResp  = _saveUploadedFile(filename, parts[i].body);
        savedAny  = true;
    }

    return savedAny ? lastResp : _buildError(Http::BAD_REQUEST);
}

HttpResponse HttpResponseBuilder::_handleRawUpload(void)
{
    std::string filename;

    // For raw uploads Content-Disposition may be a request header
    std::map<std::string, std::string>::const_iterator cd =
        _request.headers.find("content-disposition");
    if (cd != _request.headers.end())
        filename = _extractFilename(cd->second);

    if (filename.empty())
    {
        // derive extension from Content-Type so the file is usable
        std::string ext;
        std::map<std::string, std::string>::const_iterator ct =
            _request.headers.find("content-type");
        if (ct != _request.headers.end())
            ext = _extFromContentType(ct->second);

        std::ostringstream oss;
        oss << "upload_" << time(NULL) << ext;
        filename = oss.str();
    }

    return _saveUploadedFile(filename, _request.body);
}

// Shared between multipart and raw paths
HttpResponse HttpResponseBuilder::_saveUploadedFile(const std::string& filename,
                                                 const std::string& content)
{
    std::string uploadPath = _location->upload_store;
    if (uploadPath[uploadPath.size() - 1] != '/') uploadPath += '/';
    uploadPath += filename;

    std::ofstream file(uploadPath.c_str(), std::ios::binary);
    if (!file) return _buildError(Http::FORBIDDEN);

    file.write(content.c_str(), (std::streamsize)content.size());
    if (!file) return _buildError(Http::INTERNAL_SERVER_ERROR);

    // Bug 3 answer: config-specified redirect takes priority
    if (!_location->uploadReturnUrl.empty())
    {
        HttpResponse resp(Http::SEE_OTHER);
        resp.setHeader("location", _location->uploadReturnUrl);
        return resp;
    }

    // Default: 201 Created with a minimal body so the browser has something to show
    std::ostringstream html;
    html << "<!DOCTYPE html><html><body>"
         << "<p>Uploaded: <a href=\"" << _request.path << filename << "\">"
         << filename << "</a></p>"
         << "</body></html>";

    HttpResponse resp(Http::CREATED);
    resp.setHeader("location", _request.path + filename);
    resp.setBody(html.str(), "text/html");
    return resp;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

std::string HttpResponseBuilder::_extractFilename(const std::string& disposition) const
{
    size_t pos = disposition.find("filename=");
    if (pos == std::string::npos) return "";

    pos += 9;
    bool quoted = pos < disposition.size() && disposition[pos] == '"';
    if (quoted) ++pos;

    size_t end = quoted ? disposition.find('"',  pos)
                        : disposition.find(';', pos);
    if (end == std::string::npos) end = disposition.size();

    return disposition.substr(pos, end - pos);
}

std::string HttpResponseBuilder::_extFromContentType(const std::string& ct) const
{
    if (ct.find("jpeg") != std::string::npos
     || ct.find("jpg")  != std::string::npos) return ".jpg";
    if (ct.find("png")  != std::string::npos) return ".png";
    if (ct.find("gif")  != std::string::npos) return ".gif";
    if (ct.find("webp") != std::string::npos) return ".webp";
    if (ct.find("pdf")  != std::string::npos) return ".pdf";
    if (ct.find("text/plain") != std::string::npos) return ".txt";
    if (ct.find("text/html")  != std::string::npos) return ".html";
    return "";
}

HttpResponse	HttpResponseBuilder::_executeCgi(const std::string& fsPath, const std::string& interpreter)
{
	//TODO CGI
	(void)fsPath;
	(void)interpreter;
	return HttpResponse(Http::NOT_IMPLEMENTED);
}

HttpResponse	HttpResponseBuilder::_parseCgiOutput(const std::string& output)
{
	//TODO CGI
	(void)output;
	return HttpResponse(Http::NOT_IMPLEMENTED);
}

HttpResponse	HttpResponseBuilder::_buildError(Http::StatusCode code) const
{
	HttpResponse	resp(code);

	std::map<unsigned short, std::string>::const_iterator	it = _config.error_pages.find(code);

	if (it != _config.error_pages.end())
	{
		std::string	root = _location ? _location->root : "";
		if (root.empty() && !_config.locations.empty())
			root = _config.locations[0].root;

		bool	ok;
		std::string	content = _readFile(root + it->second, ok);
		if (ok) { resp.setBody(content, "text/html"); return resp; }
	}

	std::ostringstream	html;
	html << "<!DOCTYPE html>\n<html>\n<body>\n<h1>"
		 << (int)code << " " << Http::statusToReason(code)
		 << "</h1>\n</body>\n</html>\n";
	resp.setBody(html.str(), "text.html");
	return resp;
}

bool	HttpResponseBuilder::_isMethodAllowed(void) const
{
	if (_location->methods.empty()) return true;
	return _location->methods.count(_request.method) > 0;
}

bool	HttpResponseBuilder::_matchCgi(const std::string& path, std::string& outInterpreter) const
{
	size_t	dot = path.rfind('.');
	if (dot == std::string::npos) return false;

	std::map<std::string, std::string>::const_iterator	it = _location->cgi_extensions.find(path.substr(dot));
	if (it == _location->cgi_extensions.end()) return false;

	outInterpreter = it->second;
	return true;
}

std::string	HttpResponseBuilder::_getContentType(const std::string& path) const
{
	size_t	dot = path.rfind('.');
	if (dot == std::string::npos) return "application/octet-stream";

	std::string	ext = Utils::strToLower(path.substr(dot + 1));

	if (ext == "html" || ext == "htm")	return "text/html";
	if (ext == "css")					return "text/css";
	if (ext == "js")					return "application/javascript";
	if (ext == "json")					return "application/json";
	if (ext == "txt")					return "text/plain";
	if (ext == "xml")					return "application/xml";
	if (ext == "pdf")					return "application/pdf";
	if (ext == "png")					return "image/png";
	if (ext == "jpg" || ext == "jpeg")	return "image/jpeg";
	if (ext == "gif")					return "image/gif";
	if (ext == "ico")					return "image/x-icon";
	if (ext == "svg")					return "image/svg+xml";
	if (ext == "mp4")					return "video/mp4";
	if (ext == "webm")					return "video/webm";

	return "application.octet-stream";
}

std::string	HttpResponseBuilder::_readFile(const std::string& path, bool& ok) const
{
	std::ifstream	file(path.c_str(), std::ios::binary);
	if (!file) { ok = false; return ""; }

	std::ostringstream	oss;
	oss << file.rdbuf();
	ok = !file.fail();
	return ok ? oss.str() : "";
}
