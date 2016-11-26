import cffi

ffi = cffi.FFI()

ffi.cdef('''
void * loadDocument(const char *filename);
void * loadDocumentFromSource(const char *data);
void destroyDocument(void *doc);
void setDocumentParam(void *doc, const char *key, const char *value);
char * renderToHtml(void *doc, bool isWholePage);
void free(void *ptr);
''')

lib = ffi.dlopen("./libOxygenMarkRenderer.so")

def render_template(filename, params, _is_whole_page = True):
    try:
        tpl = lib.loadDocument(filename.encode("utf-8"))
        for k in params.keys():
            lib.setDocumentParam(tpl, k.encode("utf-8"), params[k].encode("utf-8"))
    except:
        lib.destroyDocument(tpl)
        raise RuntimeError("Unable to load template")
    
    if _is_whole_page == True:
        is_whole_page = True
    else:
        is_whole_page = False
    
    html_c = lib.renderToHtml(tpl, is_whole_page)

    try:
        html = ffi.string(html_c)
        lib.free(html_c)
    except RuntimeError:
        html = ""
    
    lib.destroyDocument(tpl)
    return html

def render_template_from_source(filename, params, _is_whole_page = True):
    try:
        f = open(filename)
        data = f.read()
        tpl = lib.loadDocumentFromSource(data.encode("utf-8"))
        for k in params.keys():
            lib.setDocumentParam(tpl, k.encode("utf-8"), params[k].encode("utf-8"))
    except:
        lib.destroyDocument(tpl)
        raise RuntimeError("Unable to load template")
    
    if _is_whole_page == True:
        is_whole_page = True
    else:
        is_whole_page = False
    
    html_c = lib.renderToHtml(tpl, is_whole_page)

    try:
        html = ffi.string(html_c)
        lib.free(html_c)
    except RuntimeError:
        html = ""
    
    lib.destroyDocument(tpl)
    return html
