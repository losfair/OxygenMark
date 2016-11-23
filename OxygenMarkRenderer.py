import cffi

ffi = cffi.FFI()

ffi.cdef('''
void * loadDocument(const char *filename);
void destroyDocument(void *doc);
void setDocumentParam(void *doc, const char *key, const char *value);
char * renderToHtml(void *doc);
void free(void *ptr);
''')

lib = ffi.dlopen("./libOxygenMarkRenderer.so")

def render_template(filename, params):
    tpl = lib.loadDocument(filename)
    for k, v in params.iteritems():
        lib.setDocumentParam(tpl, k, v)

    html_c = lib.renderToHtml(tpl)

    try:
        html = ffi.string(html_c)
        lib.free(html_c)
    except RuntimeError:
        html = ""
    
    lib.destroyDocument(tpl);
    return html
