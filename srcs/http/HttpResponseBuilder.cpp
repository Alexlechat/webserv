#include "config/Config.hpp"
#include "config/LocationConfig.hpp"
#include "http/Http.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpResponseBuilder.hpp"
#include "http/Cgi.hpp"
#include "logger/ConsoleLogger.hpp"
#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

#include <vector>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <sys/stat.h>

HttpResponseBuilder::BuildResult	buildResponse(const HttpRequest &request, const ServerConfig &config)
{
	return HttpResponseBuilder(request, config).build();
}

HttpResponseBuilder::HttpResponseBuilder(const HttpRequest& request, const ServerConfig& config)
	: _request(request), _config(config), _location(NULL)
{}

HttpResponseBuilder::BuildResult	HttpResponseBuilder::build(void)
{
	BuildResult	result;

	_location = _matchLocation();
	if (!_location)
	{
		result.response = _buildError(Http::NOT_FOUND);
		return result;
	}

	if (_isBodySizeTooLarge())
	{
		result.response = _buildError(Http::CONTENT_TOO_LARGE);
		return result;
	}

	if (_location->redirect.first != 0)
	{
		HttpResponse	resp((Http::StatusCode)_location->redirect.first);
		resp.setHeader("location", _location->redirect.second);
		result.response = resp;
		return result;
	}

	switch (_request.methodEnum)
	{
		case Http::GET:
		case Http::HEAD:	result = _handleGet();	break;
		case Http::POST:	result = _handlePost();	break;
		case Http::DELETE:	result.response = _handleDelete();	break;
		default:			result.response = _buildError(Http::NOT_IMPLEMENTED); break;
	}

	if (!result.isCgi && _request.methodEnum == Http::HEAD)
		result.response.dropBodyForHead();

	return result;
}

bool			HttpResponseBuilder::_isBodySizeTooLarge(void) const
{
	return _location->client_max_body_size > 0 && _request.body.size() > _location->client_max_body_size;
}

const LocationConfig*	HttpResponseBuilder::_matchLocation(void) const
{
	return _matchLocationForPath(_request.path);
}

const LocationConfig*	HttpResponseBuilder::_matchLocationForPath(const std::string& path) const
{
	const LocationConfig*	best    = NULL;
	size_t					bestLen = 0;

	for (size_t i = 0; i < _config.locations.size(); ++i)
	{
		const std::string&	locPath = _config.locations[i].path;
		if (path.compare(0, locPath.size(), locPath) != 0)
			continue;
		if (locPath.size() < path.size()
			&& locPath[locPath.size() - 1] != '/'
			&& path[locPath.size()] != '/')
			continue;
		if (locPath.size() > bestLen)
		{
			bestLen = locPath.size();
			best    = &_config.locations[i];
		}
	}

	return best;
}

std::string	HttpResponseBuilder::_effectiveRoot(void) const
{
	if (!_location->root->empty())
		return _location->root;
	if (!_location->upload_store->empty())
		return _location->upload_store;
	if (!_config.root->empty())
		return _config.root;
	return "";
}

bool	HttpResponseBuilder::_resolvePath(std::string& outPath) const
{
	std::string	rel = _request.path.substr(_location->path.size());

	std::string	normalized = Utils::normalizePath(rel);
	if (normalized.empty())
		return false;

	std::string	fsPath = _effectiveRoot();
	if (fsPath.empty())
		return false;
	if (fsPath[fsPath.size() - 1] != '/')
		fsPath += '/';

	std::string	relClean = normalized;
	if (!relClean.empty() && relClean[0] == '/')
		relClean = relClean.substr(1);

	outPath = fsPath + relClean;
	return true;
}

HttpResponseBuilder::BuildResult	HttpResponseBuilder::_handleGet(void)
{
	BuildResult	result;

	if (!_isMethodAllowed())
	{
		result.response = _buildError(Http::METHOD_NOT_ALLOWED);
		return result;
	}

	std::string	fsPath;
	if (!_resolvePath(fsPath))
	{
		result.response = _buildError(Http::FORBIDDEN);
		return result;
	}

	struct stat	st;
	if (stat(fsPath.c_str(), &st) != 0)
	{
		if (!_location->cgi_extensions.empty())
			return _tryCgiWithPathInfo();
		result.response = _buildError(Http::NOT_FOUND);
		return result;
	}

	if (S_ISDIR(st.st_mode))
		return _serveDirectory(fsPath);

	std::string	interpreter;
	if (_matchCgi(fsPath, interpreter))
		return _startCgi(fsPath, "", interpreter);

	result.response = _serveFile(fsPath);
	return result;
}

HttpResponseBuilder::BuildResult	HttpResponseBuilder::_tryCgiWithPathInfo(void)
{
	BuildResult		result;
	std::string		scriptRelPath;
	std::string		pathInfo;
	std::string		interpreter;

	std::string		rel = _request.path.substr(_location->path.size());
	if (!_matchCgiWithPathInfo(rel, scriptRelPath, pathInfo, interpreter))
	{
		result.response = _buildError(Http::NOT_FOUND);
		return result;
	}

	std::string	root = _effectiveRoot();
	if (!root.empty() && root[root.size() - 1] != '/')
		root += '/';

	std::string	scriptFs = root + scriptRelPath;
	return _startCgi(scriptFs, pathInfo, interpreter);
}

HttpResponseBuilder::BuildResult	HttpResponseBuilder::_handlePost(void)
{
	BuildResult	result;

	if (!_isMethodAllowed())
	{
		result.response = _buildError(Http::METHOD_NOT_ALLOWED);
		return result;
	}

	std::string	fsPath;
	if (!_resolvePath(fsPath))
	{
		result.response = _buildError(Http::FORBIDDEN);
		return result;
	}

	struct stat	st;
	if (stat(fsPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
	{
		std::string	interpreter;
		if (_matchCgi(fsPath, interpreter))
			return _startCgi(fsPath, "", interpreter);
	}
	else if (!_location->cgi_extensions.empty())
	{
		return _tryCgiWithPathInfo();
	}

	if (!_location->upload_store->empty())
	{
		result.response = _handleUpload();
		return result;
	}

	result.response = _buildError(Http::METHOD_NOT_ALLOWED);
	return result;
}

HttpResponse	HttpResponseBuilder::_handleDelete(void)
{
	if (!_isMethodAllowed())
		return _buildError(Http::METHOD_NOT_ALLOWED);

	std::string	fsPath;
	if (!_resolvePath(fsPath))
		return _buildError(Http::FORBIDDEN);

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

HttpResponseBuilder::BuildResult	HttpResponseBuilder::_serveDirectory(const std::string& fsPath)
{
	BuildResult	result;

	if (_request.path.empty() || _request.path[_request.path.size() - 1] != '/')
	{
		HttpResponse	resp(Http::MOVED_PERMANENTLY);
		resp.setHeader("location", _request.path + "/");
		result.response = resp;
		return result;
	}

	if (!_location->index->empty())
	{
		std::string	indexPath = fsPath;
		if (indexPath[indexPath.size() - 1] != '/') indexPath += '/';
		indexPath += _location->index;

		struct stat	st;
		if (stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
		{
			std::string	interpreter;
			if (_matchCgi(indexPath, interpreter))
				return _startCgi(indexPath, "", interpreter);
			result.response = _serveFile(indexPath);
			return result;
		}
	}

	if (_location->autoindex)
	{
		result.response = _autoIndex(fsPath);
		return result;
	}

	result.response = _buildError(Http::FORBIDDEN);
	return result;
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
		 << _request.path << "</title></head>\n<body>\n"
		 << "<h1>Index of " << _request.path << "</h1><hr><pre>\n";

	if (_request.path != "/")
		html << "<a href=\"../\">../</a>\n";

	for (size_t i = 0; i < entries.size(); ++i)
	{
		std::string	escaped = Utils::htmlEscape(entries[i]);
		html << "<a href=\"" << escaped << "\">" << escaped << "</a>\n";
	}

	html << "</pre><hr></body></html>\n";

	HttpResponse	resp(Http::OK);
	resp.setBody(html.str(), "text/html");
	return resp;
}

bool	HttpResponseBuilder::_parseMultipartUpload(std::string& outFilename, std::string& outContent) const
{
	std::map<std::string, std::string>::const_iterator	ct = _request.headers.find("content-type");
	if (ct == _request.headers.end())
		return false;

	std::string	ctLower = Utils::strToLower(ct->second);
	if (ctLower.find("multipart/form-data") == std::string::npos)
		return false;

	size_t	bPos = ctLower.find("boundary=");
	if (bPos == std::string::npos)
		return false;

	std::string	boundary = ct->second.substr(bPos + 9);
	if (!boundary.empty() && boundary[0] == '"')
	{
		boundary = boundary.substr(1);
		size_t	q = boundary.find('"');
		if (q != std::string::npos)
			boundary = boundary.substr(0, q);
	}

	std::string			delim = "--" + boundary;
	const std::string&		body = _request.body;
	size_t				searchPos = 0;

	while (true)
	{
		size_t	start = body.find(delim, searchPos);
		if (start == std::string::npos)
			break;
		start += delim.size();

		if (body.compare(start, 2, "--") == 0)
			break;
		if (body.compare(start, 2, "\r\n") == 0)
			start += 2;

		size_t	nextBoundary = body.find(delim, start);
		if (nextBoundary == std::string::npos)
			break;

		std::string	part = body.substr(start, nextBoundary - start);
		if (part.size() >= 2 && part.compare(part.size() - 2, 2, "\r\n") == 0)
			part.erase(part.size() - 2);

		size_t	headerEnd = part.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			std::string	partHeaders = part.substr(0, headerEnd);
			std::string	partHeadersLower = Utils::strToLower(partHeaders);
			size_t		cdPos = partHeadersLower.find("content-disposition");
			size_t		fnPos = (cdPos == std::string::npos) ? std::string::npos
								: partHeaders.find("filename=", cdPos);

			if (fnPos != std::string::npos)
			{
				fnPos += 9;
				bool	quoted = (fnPos < partHeaders.size() && partHeaders[fnPos] == '"');
				if (quoted) ++fnPos;
				size_t	end = quoted ? partHeaders.find('"', fnPos)
									 : partHeaders.find_first_of(";\r\n", fnPos);
				if (end == std::string::npos) end = partHeaders.size();

				outFilename = partHeaders.substr(fnPos, end - fnPos);
				outContent = part.substr(headerEnd + 4);
				return true;
			}
		}

		searchPos = nextBoundary;
	}

	return false;
}

HttpResponse	HttpResponseBuilder::_handleUpload(void)
{
	std::string	filename;
	std::string	content;

	if (!_parseMultipartUpload(filename, content))
	{
		content = _request.body;

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
	}

	filename = Utils::sanitizeFilename(filename);

	if (filename.empty())
	{
		static unsigned int	counter = 0;
		std::ostringstream	oss;
		oss << "upload_" << time(NULL) << "_" << counter++;
		filename = oss.str();
	}

	std::string	uploadPath = _location->upload_store;
	if (uploadPath[uploadPath.size() - 1] != '/') uploadPath += '/';
	uploadPath += filename;

	std::ofstream	file(uploadPath.c_str(), std::ios::binary);
	if (!file)
		return _buildError(Http::FORBIDDEN);

	file.write(content.c_str(), (std::streamsize)content.size());
	if (!file)
		return _buildError(Http::INTERNAL_SERVER_ERROR);

	if (_location->upload_return.specified)
	{
		HttpResponse	resp(Http::SEE_OTHER);
		resp.setHeader("location", _location->upload_return);
		return resp;
	}

	HttpResponse	resp(Http::CREATED);
	resp.setHeader("location", _request.path + filename);
	return resp;
}

HttpResponseBuilder::BuildResult	HttpResponseBuilder::_startCgi(const std::string& fsPath, const std::string& pathInfo, const std::string& interpreter)
{
	BuildResult	result;

	struct stat	st;
	if (stat(fsPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
	{
		result.response = _buildError(Http::NOT_FOUND);
		return result;
	}

	Cgi*	cgi = new Cgi(_request, _config, *_location, fsPath, pathInfo, interpreter);

	if (!cgi->start())
	{
		delete cgi;
		result.response = _buildError(Http::INTERNAL_SERVER_ERROR);
		return result;
	}

	result.isCgi = true;
	result.cgi = cgi;
	return result;
}

HttpResponse	HttpResponseBuilder::_buildError(Http::StatusCode code) const
{
	HttpResponse	resp(code);

	const std::map<unsigned short, std::string>*			pageMap = NULL;
	std::map<unsigned short, std::string>::const_iterator	it;

	if (_location)
	{
		it = _location->error_pages.find(code);
		if (it != _location->error_pages.end())
			pageMap = &_location->error_pages;
	}
	if (!pageMap)
	{
		it = _config.error_pages.find(code);
		if (it != _config.error_pages.end())
			pageMap = &_config.error_pages;
	}

	if (pageMap)
	{
		const LocationConfig*	errLoc = _matchLocationForPath(it->second);

		std::string	root;
		if (errLoc && !errLoc->root->empty())
			root = errLoc->root;
		else if (!_config.locations.empty())
			root = _config.locations[0].root;

		if (!root.empty() && root[root.size() - 1] != '/')
			root += '/';

		std::string	relPage = it->second;
		if (!relPage.empty() && relPage[0] == '/')
			relPage = relPage.substr(1);

		bool		ok;
		std::string	content = _readFile(root + relPage, ok);
		if (ok)
		{
			resp.setBody(content, "text/html");
			return resp;
		}

		LOG_WARNING(ConsoleLogger::instance(),
			"Configured error_page for " + Utils::toString((int)code) + " could not be opened: " + root + relPage);
	}

	buildDefaultErrorPageBody(resp, code);
	return resp;
}
bool	HttpResponseBuilder::_isMethodAllowed(void) const
{
	if (_location->methods.empty()) return true;
	if (_location->methods.count(_request.method) > 0) return true;

	if (_request.methodEnum == Http::HEAD && _location->methods.count("GET") > 0) return true;

	return false;
}

bool	HttpResponseBuilder::_matchCgi(const std::string& path, std::string& outInterpreter) const
{
	size_t	dot = path.rfind('.');
	if (dot != std::string::npos)
	{
		std::map<std::string, std::string>::const_iterator	it = _location->cgi_extensions.find(path.substr(dot));
		if (it != _location->cgi_extensions.end())
		{
			outInterpreter = it->second;
			return true;
		}
	}

	return false;
}

bool	HttpResponseBuilder::_matchCgiWithPathInfo(const std::string& requestPath,
		std::string& outScriptPath, std::string& outPathInfo, std::string& outInterpreter) const
{
	for (std::map<std::string, std::string>::const_iterator extIt = _location->cgi_extensions.begin();
		 extIt != _location->cgi_extensions.end(); ++extIt)
	{
		const std::string&	ext = extIt->first;
		size_t				pos = requestPath.find(ext);

		while (pos != std::string::npos)
		{
			size_t	afterExt = pos + ext.size();
			if (afterExt == requestPath.size() || requestPath[afterExt] == '/')
			{
				outScriptPath = requestPath.substr(0, afterExt);
				outPathInfo = (afterExt < requestPath.size()) ? requestPath.substr(afterExt) : "";
				outInterpreter = extIt->second;
				return true;
			}
			pos = requestPath.find(ext, afterExt);
		}
	}

	return false;
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

	return "application/octet-stream";
}

std::string	HttpResponseBuilder::_readFile(const std::string& path, bool& ok) const
{
	struct stat	fileStat;
	if (stat(path.c_str(), &fileStat) != 0 || !S_ISREG(fileStat.st_mode))
	{
		ok = false;
		return "";
	}

	static const off_t	MAX_SERVE_SIZE = 100 * 1024 * 1024;
	if (fileStat.st_size > MAX_SERVE_SIZE)
	{
		ok = false;
		return "";
	}

	std::ifstream	file(path.c_str(), std::ios::binary);
	if (!file) { ok = false; return ""; }

	std::ostringstream	oss;
	oss << file.rdbuf();
	ok = !file.fail();
	return ok ? oss.str() : "";
}

HttpResponse	HttpResponseBuilder::buildDefaultErrorPageBody(HttpResponse& resp, const Http::StatusCode code)
{
	std::ostringstream	html;
	html << "<!DOCTYPE html>\n<html>\n<body>\n<h1>"
		 << (int)code << " " << Http::statusToReason(code)
		 << "</h1>\n</body>\n</html>\n";
	resp.setBody(html.str(), "text/html");

	return resp;
}
