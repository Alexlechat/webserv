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

HttpResponse	buildResponse(const HttpRequest &request, const ServerConfig &config)
{
	return HttpResponseBuilder(request, config).build();
}

HttpResponseBuilder::HttpResponseBuilder(const HttpRequest& request, const ServerConfig& config)
	: _request(request), _config(config), _location(NULL)
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

HttpResponse	HttpResponseBuilder::_handleUpload(void)
{
	std::string	filename;

	std::map<std::string, std::string>::const_iterator	cd = _request.headers.find("content-disposition");
	if (cd != _request.headers.end())
	{
		size_t	pos = cd->second.find("filename=");
		if (pos != std::string::npos)
		{
			pos += 9;
			bool	quoted = (pos < cd->second.size() && cd->second[pos] == '"');
			if (quoted) ++pos;
			size_t	end = quoted ? cd->second.find('"', pos)
								 : cd->second.find(";", pos);
			if (end == std::string::npos) end = cd->second.size();
			filename = cd->second.substr(pos, end - pos);
		}
	}

	if (filename.empty())
	{
		std::ostringstream	oss;
		oss << "upload_" << time(NULL);
		filename = oss.str();
	}

	std::string	uploadPath = _location->upload_store;
	if (uploadPath[uploadPath.size() - 1] != '/') uploadPath += '/';
	uploadPath += filename;
	
	std::ofstream	file(uploadPath.c_str(), std::ios::binary);
	if (!file)
		return _buildError(Http::FORBIDDEN);

	file.write(_request.body.c_str(), (std::streamsize)_request.body.size());
	if (!file)
	{
		// TODO LOG INTO ERROR.LOG
		return _buildError(Http::INTERNAL_SERVER_ERROR);
	}

	HttpResponse	resp(Http::CREATED);
	resp.setHeader("location", _request.path + filename);
	return resp;
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
