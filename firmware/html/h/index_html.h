#ifndef __INDEX_HTML_H__
#define __INDEX_HTML_H__
const char index_html[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"Content-Type\" content=\"text/html\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" /><title>WiFi Display</title></head>on<body><div class=\"middlediv\"><h1>WiFI Display</h1><p><form class=\"navbuton\" action=\"live\" method=\"get\"><button>Live view</button></form></p><p><form class=\"navbuton\" action=\"cfg\" method=\"get\"><button>Settings</button></form></p><div class=\"footer\"><hr><a href=\"https://github.com/ofreddy\" target=\"_blank\">                Version {VERSION} Git #{GIT}            </a></div></div></body></html>";
const uint32_t index_html_len = 666;
#endif /*__INDEX_HTML_H__*/
