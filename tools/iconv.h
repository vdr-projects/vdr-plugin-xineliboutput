/*
 * iconv.h: iconv library wrapper
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef _XINELIBOUTPUT_ICONV_H_
#define _XINELIBOUTPUT_ICONV_H_

#if defined(USE_ICONV) && USE_ICONV == 0
#  undef USE_ICONV
#  warning iconv disabled
#endif

#ifdef USE_ICONV
#  include <iconv.h>
#endif

class cIConv
{
#ifdef USE_ICONV
  private:
    iconv_t  m_ic;
#endif

  public:
    cIConv(const char *SrcCharset = NULL, const char * DstCharset = NULL);
    virtual ~cIConv();

    cString Translate(const char *Text) const;
};

cIConv::cIConv(const char *SrcCharset, const char * DstCharset)
{
#ifdef USE_ICONV
  if(!SrcCharset)
    SrcCharset = "UTF-8";
  if(!DstCharset)
    DstCharset = I18nCharSets()[Setup.OSDLanguage];

  m_ic = iconv_open(DstCharset, SrcCharset);

  if(m_ic == (iconv_t)-1) 
    LOGERR("cIConv: iconv_open(\"%s\",\"%s\") failed",
	   SrcCharset, DstCharset);
# if 0
  else
    LOGDBG("cIConv: initialized conversion from \'%s\' to \'%s\'", 
	   SrcCharset, DstCharset);
# endif
#endif
}

cIConv::~cIConv()
{
#ifdef USE_ICONV
  if(m_ic != (iconv_t)-1) 
    iconv_close(m_ic);
#endif
}

cString cIConv::Translate(const char *Text) const
{
#ifdef USE_ICONV
  size_t  inc  = strlen(Text);
  size_t  outc = inc<2048 ? 2048 : inc+1;
#ifdef __APPLE__  
  const char *in = Text;
#else
  char   *in   = (char*)Text;
#endif
  char   *buf  = (char*)malloc(outc+1);
  char   *out  = buf;

  size_t n = iconv(m_ic, &in, &inc, &out, &outc);

  if(n != (size_t)-1) {
    *out = 0;
    return cString(buf, true);
  }

  LOGERR("cIConv: iconv(%s) failed at %d", Text, (int)(in - Text));
  free(buf);
#endif

  return cString(Text);
}


#endif // _XINELIBOUTPUT_ICONV_H_
