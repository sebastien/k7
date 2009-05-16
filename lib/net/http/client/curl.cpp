// ----------------------------------------------------------------------------
// Project           : K7 - Standard Library for V8
// -----------------------------------------------------------------------------
// Author            : Sebastien Pierre                   <sebastien@type-z.org>
//                   : Tokuhiro Matsuno                    <tokuhirom@gmail.com>
// -----------------------------------------------------------------------------
// Creation date     : 13-Dec-2008
// Last modification : 08-May-2009
// -----------------------------------------------------------------------------

#ifdef WITH_CURL

#include "k7.h"

#include <curl/curl.h>
#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <iconv.h>
#include <errno.h>

using namespace v8;

#define MODULE_NAME   "net.http.client.curl"
#define MODULE_STATIC  net_http_client_curl

// ----------------------------------------------------------------------------
//
// MACROS & UTILITIES
//
// ----------------------------------------------------------------------------

static char*  response_data = NULL;  /* response data from server. */
static char*  response_mime = NULL;  /* response content-type. ex: "text/html" */
static size_t response_size = 0;    /* response size of data */

static size_t handle_returned_header(void* ptr, size_t size, size_t nmemb, void* stream) {
	char* header = new char[size*nmemb + 1];
	memcpy(header, ptr, size*nmemb);
	header[size*nmemb] = 0;
	if (strncmp(header, "Content-Type: ", 14) == 0) {
		char* stop = header + 14;
		stop = strpbrk(header + 14, "\r\n");
		if (stop) *stop = 0;
		if (response_mime) delete[] response_mime;
		response_mime = new char[strlen(header + 14) + 1];
		strcpy(response_mime, header + 14);
	}
	delete[] header;
	return size*nmemb;
}

static size_t handle_returned_data(char* ptr, size_t size, size_t nmemb, void* stream) {
	if (!response_data) {
		response_data = new char[size*nmemb];
	} else {
		char* response_tmp = new char[response_size+size*nmemb];
		memset(response_tmp, 0, response_size+size*nmemb);
		memcpy(response_tmp, response_data, response_size);
		delete[] response_data;
		response_data = response_tmp;
	}
	if (response_data) {
		memcpy(response_data+response_size, ptr, size*nmemb);
		response_size += size*nmemb;
	}
	return size*nmemb;
}

// NOTE: This should be moved to another module/library or K7 itself
static char* convert_string_alloc(const char* str, const char* from_codeset, const char* to_codeset) {
	char *dest = NULL;
	char *outp;
	size_t inbytes_remaining = strlen (str);
	size_t outbuf_size = inbytes_remaining + 1;
	size_t outbytes_remaining;
	size_t err;
	int have_error = 0;

	outbuf_size *= MB_LEN_MAX;
	outbytes_remaining = outbuf_size - 1;

	if (strcasecmp(to_codeset, from_codeset) == 0) {
		outp = new char[strlen(str) + 1];
		strcpy(outp , str);
		return outp;
	}

	iconv_t cd = iconv_open(to_codeset, from_codeset);
	if (cd == (iconv_t) -1) {
		outp = new char[strlen(str) + 1];
		strcpy(outp , str);
		return outp;
	}

	outp = dest = (char *) malloc (outbuf_size);
	if (dest == NULL)
		goto out;

again:

// Special thanks to the mcalhoun on MacPorts for finding this one.
// http://trac.macports.org/ticket/18258#comment:35
// libiconv is a mess. For some platforms/version the prototype of inbuf is
// "const char **", for others it is "char **". C++ requires the proper cast to
// avoid a compile error, that is were the CASTNEEDED is for
#if ((defined(_LIBICONV_VERSION) && (_LIBICONV_VERSION>=0x0109) && \
      !((defined(_OS_MAC_) || defined(Q_OS_MACX) )&& (_LIBICONV_VERSION==0x010B))) \
    || defined(_OS_SOLARIS_) \
    || defined(_OS_NETBSD_)  \
    )
#define CASTNEEDED(x) (x)
#else
#define CASTNEEDED(x) (char **)(x)
#endif

	//err = iconv(cd, CASTNEEDED(&str), &inbytes_remaining, &outp, &outbytes_remaining);
	err = iconv(cd, (char **)(&str), &inbytes_remaining, &outp, &outbytes_remaining);

	if (err == (size_t) - 1) {
		switch (errno) {
			case EINVAL:
				break;
			case E2BIG:
				{
					size_t used = outp - dest;
					size_t newsize = outbuf_size * 2;
					char *newdest;

					if (newsize <= outbuf_size) {
						errno = ENOMEM;
						have_error = 1;
						goto out;
					}
					newdest = (char *) realloc (dest, newsize);
					if (newdest == NULL) {
						have_error = 1;
						goto out;
					}
					dest = newdest;
					outbuf_size = newsize;

					outp = dest + used;
					outbytes_remaining = outbuf_size - used - 1;        /* -1 for NUL */

					goto again;
				}
				break;
			case EILSEQ:
				have_error = 1;
				break;
			default:
				have_error = 1;
				break;
		}
	}

	*outp = '\0';

out:
	{
		int save_errno = errno;

		if (iconv_close (cd) < 0 && !have_error) {
			/* If we didn't have a real error before, make sure we restore
			   the iconv_close error below. */
			save_errno = errno;
			have_error = 1;
		}

		if (have_error && dest) {
			free (dest);
			dest = (char*)str;
			errno = save_errno;
		}
	}

	return dest;
}


// ----------------------------------------------------------------------------
//
// FUNCTIONS
//
// ----------------------------------------------------------------------------

// TODO: Add an asynchronous variant that returns a future
FUNCTION(fetchURL)
{
	ARG_COUNT(1)
	ARG_str(url,0)

	CURL* curl    = NULL;
	CURLcode ret  = CURLE_OK;
	int stat      = -1;

	// FIXME: Not threadsafe
	response_size = 0;
	response_data = NULL;
	response_mime = NULL;

	curl = curl_easy_init();
	if (!curl) return v8::ThrowException(v8::String::New("Error: unknown"));

	curl_easy_setopt(curl, CURLOPT_URL, *url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_returned_data);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, handle_returned_header);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	ret = curl_easy_perform(curl);
	ret == CURLE_OK ? curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &stat) : ret;
	curl_easy_cleanup(curl);

	v8::Handle<v8::Object> res = v8::Object::New();
	res->Set(v8::String::New("status"), v8::Number::New(stat));

	if (stat != -1 && response_data) {
		char* data = new char[response_size+1];
		memset(data, 0, response_size+1);
		memcpy(data, (char*) response_data, response_size);
		if (response_mime) {
			char* stop;
			char* charset = strstr(response_mime, "charset=");
			if (charset) {
				charset += 8;
				if (*charset == '"') charset++;
				stop = strpbrk(charset, "\";");
				if (stop) *stop = 0;
				if (*charset) {
					char* newbuf = convert_string_alloc(data, charset, "utf-8");
					delete[] data;
					data = newbuf;
				}
				res->Set(v8::String::New("charset"), v8::String::New(charset));
			}
			stop = strpbrk(response_mime, " ;");
			if (stop) *stop = 0;
			res->Set(v8::String::New("mimeType"), v8::String::New(response_mime));
		}
		res->Set(v8::String::New("responseText"), v8::String::New(data));
		delete[] data;
	}

	response_size = 0;
	if (response_data) {
		delete[] response_data;
		response_data = NULL;
	}
	if (response_data) {
		delete[] response_mime;
		response_mime = NULL;
	}
	return res;
}
END

// ----------------------------------------------------------------------------
//
// MODULE
//
// ----------------------------------------------------------------------------

MODULE
{
	BIND("fetchURL",   fetchURL);
	SET_str("VERSION", curl_version());
}
END_MODULE

#endif

// EOF
