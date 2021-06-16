/****************************************************************************
 *
 * Copyright (C) 2015 Neutrino International Inc.
 *
 ****************************************************************************/

#include <qtcurl.h>

#ifdef QT_STATIC
#define CURL_STATICLIB
#endif

#include <libssh2.h>
#include <curl.h>

QT_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////////

void RegisterCURL(void)
{
  qRegisterMetaType < QtCURL > ( "QtCURL" ) ;
}

//////////////////////////////////////////////////////////////////////////////

static bool cUrlInitialized = false ;

QtCURL:: QtCURL            ( void                )
       : Operation         ( None                )
       , File              ( NULL                )
       , Url               ( NULL                )
       , Written           ( NULL                )
       , Sent              ( NULL                )
       , connectionTimeout ( 0                   )
       , ResponseCode      ( CURLE_OK            )
       , urlFormat         ( QUrl::EncodeUnicode )
//       , urlFormat         ( QUrl::FullyEncoded )
{
  writer   = this -> cwrite ;
  reader   = this -> cread  ;
  lister   = this -> clist  ;
  tagger   = this -> hwrite ;
  accepter = this           ;
}

QtCURL::~QtCURL (void)
{
}

QString QtCURL::Version(void)
{
  return QString ( ::curl_version() ) ;
}

void QtCURL::Initialize(void)
{
  if ( cUrlInitialized ) return          ;
  ::curl_global_init ( CURL_GLOBAL_ALL ) ;
  cUrlInitialized = true                 ;
}

void QtCURL::Cleanup(void)
{
  if ( ! cUrlInitialized ) return ;
  ::curl_global_cleanup ( )       ;
  cUrlInitialized = false         ;
}

QString QtCURL::Explain(int curlcode)
{
  const char * errstr                                     ;
  errstr = ::curl_easy_strerror ( ( CURLcode ) curlcode ) ;
  if ( NULL == errstr ) return QString ( )                ;
  return QString ( errstr )                               ;
}

QStringList QtCURL::items(QByteArray & data)
{
  QString     S = QString::fromUtf8(data) ;
  QStringList s = S.split('\n')           ;
  QStringList R                           ;
  foreach (S,s)                           {
    QString i = S                         ;
    i  = i.replace("\r","")               ;
    i  = i.replace("\n","")               ;
    R << i                                ;
  }                                       ;
  return R                                ;
}

void QtCURL::starting(QUrl & url,Operations Operation)
{ Q_UNUSED ( url )                                                ;
  if ( NULL == File ) return                                      ;
  switch ( Operation )                                            {
    case None     :                                         break ;
    case Verify   :                                         break ;
    case List     : File -> open ( QIODevice::WriteOnly ) ; break ;
    case Header   : File -> open ( QIODevice::WriteOnly ) ; break ;
    case Download : File -> open ( QIODevice::WriteOnly ) ; break ;
    case Upload   : File -> open ( QIODevice::ReadOnly  ) ; break ;
  }                                                               ;
}

void QtCURL::finished(bool success)
{ Q_UNUSED ( success )       ;
  if ( NULL == File ) return ;
  File->close()              ;
}

void QtCURL::failure(void)
{
}

size_t QtCURL::cwrite(void * buffer,size_t size,size_t nmemb,void * userp)
{
  QtCURL * ftp = (QtCURL *) userp               ;
  return ftp -> write ( buffer , size , nmemb ) ;
}

size_t QtCURL::cread(void * buffer,size_t size,size_t nmemb,void * userp)
{
  QtCURL * ftp = (QtCURL *) userp              ;
  return ftp -> read ( buffer , size , nmemb ) ;
}

size_t QtCURL::clist(void * buffer,size_t size,size_t nmemb,void * userp)
{
  QtCURL * ftp = (QtCURL *) userp              ;
  return ftp -> list ( buffer , size , nmemb ) ;
}

size_t QtCURL::hwrite(void * buffer,size_t size,size_t nmemb,void * userp)
{
  QtCURL * ftp = (QtCURL *) userp                ;
  return ftp -> writeh ( buffer , size , nmemb ) ;
}

size_t QtCURL::write(void * buffer,size_t size,size_t nmemb)
{
  qint64 n                                        ;
  n  = size                                       ;
  n *= nmemb                                      ;
  if ( NULL == File    ) return n                 ;
  n = File -> write ( (const char *) buffer , n ) ;
  if ( NULL != Written ) (*Written) +=  n         ;
  return n                                        ;
}

size_t QtCURL::writeh(void * buffer,size_t size,size_t nmemb)
{
  qint64 n                                        ;
  n  = size                                       ;
  n *= nmemb                                      ;
  if ( n <= 0 ) return 0                          ;
  dlHeader . append ( (const char *) buffer , n ) ;
  return n                                        ;
}

size_t QtCURL::list(void * buffer,size_t size,size_t nmemb)
{
  qint64 n                                        ;
  n  = size                                       ;
  n *= nmemb                                      ;
  if ( NULL == File ) return n                    ;
  n = File -> write ( (const char *) buffer , n ) ;
  return n                                        ;
}

size_t QtCURL::read(void * buffer,size_t size,size_t nmemb)
{
  qint64 n = ( size * nmemb )              ;
  if ( NULL == File ) return n             ;
  n = File -> read ( (char *) buffer , n ) ;
  if ( NULL != Sent ) (*Sent) +=  n        ;
  return n                                 ;
}

size_t QtCURL::Filesize(void)
{
  if ( NULL == File ) return 0 ;
  return File -> size ( )      ;
}

bool QtCURL::post(QUrl & url,QIODevice & file,QString values,int mtimeout)
{
  Initialize       ( )                                                       ;
  dlHeader . clear ( )                                                       ;
  ////////////////////////////////////////////////////////////////////////////
  bool     success = false                                                   ;
  CURL   * curl    = NULL                                                    ;
  CURLcode result                                                            ;
  ////////////////////////////////////////////////////////////////////////////
  Operation    =  Download                                                   ;
  File         = &file                                                       ;
  Url          = &url                                                        ;
  ResponseCode = CURLE_OK                                                    ;
  ////////////////////////////////////////////////////////////////////////////
  if ( NULL != Written ) (*Written) = 0                                      ;
  if ( NULL != Sent    ) (*Sent   ) = 0                                      ;
  ////////////////////////////////////////////////////////////////////////////
  curl = ::curl_easy_init (                                                ) ;
  if ( NULL != curl )                                                        {
    QByteArray ADDR = url.toEncoded(urlFormat)                               ;
    QByteArray BBBB = values . toUtf8 ( ) . data ( )                         ;
    starting             (  url , Download                                 ) ;
    //////////////////////////////////////////////////////////////////////////
    ::curl_easy_setopt   ( curl , CURLOPT_URL           , ADDR.constData() ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_POST          , 1                ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_POSTFIELDS    , BBBB.constData() ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_WRITEFUNCTION , writer           ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_READFUNCTION  , reader           ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_WRITEDATA     , accepter         ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_READDATA      , accepter         ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_VERBOSE       , 1L               ) ;
    //////////////////////////////////////////////////////////////////////////
    if ( connectionTimeout > 0 )                                             {
      ::curl_easy_setopt ( curl                                              ,
                           CURLOPT_CONNECTTIMEOUT_MS                         ,
                           connectionTimeout                               ) ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    if ( mtimeout > 0 )                                                      {
      ::curl_easy_setopt ( curl , CURLOPT_TIMEOUT_MS , mtimeout            ) ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    QStringList         keys = Requests . keys ( )                           ;
    QString             K                                                    ;
    struct curl_slist * list = NULL                                          ;
    foreach ( K , keys )                                                     {
      if (K.toLower()=="user-agent")                                         {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_USERAGENT , B.constData()      ) ;
      } else
      if (K.toLower()=="proxy")                                              {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_PROXYTYPE,CURLPROXY_HTTP       ) ;
        ::curl_easy_setopt ( curl , CURLOPT_PROXY , B.constData()          ) ;
      } else
      if (K.toLower()=="cookie")                                             {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIESESSION , 1              ) ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIEJAR  , B.constData()     ) ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIEFILE , B.constData()     ) ;
      } else                                                                 {
        QString U = K + ": " + Requests[K]                                   ;
        list = ::curl_slist_append ( list , U . toUtf8() . constData()     ) ;
      }                                                                      ;
    }                                                                        ;
    if ( ( keys . count ( ) > 0 ) && ( NULL != list ) )                      {
      ::curl_easy_setopt ( curl , CURLOPT_HTTPHEADER , list )                ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    result       = ::curl_easy_perform ( curl                              ) ;
    ResponseCode = result                                                    ;
    ::curl_easy_cleanup                ( curl                              ) ;
    //////////////////////////////////////////////////////////////////////////
    if ( NULL != list ) ::curl_slist_free_all ( list )                       ;
    list = NULL                                                              ;
    success = ( CURLE_OK == result )                                         ;
    finished  ( success            )                                         ;
  } else failure ( )                                                         ;
  ////////////////////////////////////////////////////////////////////////////
  File = NULL                                                                ;
  Url  = NULL                                                                ;
  ////////////////////////////////////////////////////////////////////////////
  return success                                                             ;
}

bool QtCURL::download(QUrl & url,QIODevice & file,int mtimeout)
{
  Initialize       ( )                                                       ;
  dlHeader . clear ( )                                                       ;
  ////////////////////////////////////////////////////////////////////////////
  bool     success = false                                                   ;
  CURL   * curl    = NULL                                                    ;
  CURLcode result                                                            ;
  ////////////////////////////////////////////////////////////////////////////
  Operation    =  Download                                                   ;
  File         = &file                                                       ;
  Url          = &url                                                        ;
  ResponseCode = CURLE_OK                                                    ;
  ////////////////////////////////////////////////////////////////////////////
  if ( NULL != Written ) (*Written) = 0                                      ;
  if ( NULL != Sent    ) (*Sent   ) = 0                                      ;
  ////////////////////////////////////////////////////////////////////////////
  curl = ::curl_easy_init (                                                ) ;
  if ( NULL != curl )                                                        {
    QByteArray ADDR = url.toEncoded(urlFormat)                               ;
    starting           (  url , Download                                   ) ;
    ::curl_easy_setopt ( curl , CURLOPT_URL            , ADDR . data ( )   ) ;
    ::curl_easy_setopt ( curl , CURLOPT_HEADERFUNCTION , tagger            ) ;
    ::curl_easy_setopt ( curl , CURLOPT_WRITEFUNCTION  , writer            ) ;
    ::curl_easy_setopt ( curl , CURLOPT_WRITEHEADER    , accepter          ) ;
    ::curl_easy_setopt ( curl , CURLOPT_WRITEDATA      , accepter          ) ;
    ::curl_easy_setopt ( curl , CURLOPT_VERBOSE        , 0L                ) ;
    if ( connectionTimeout > 0 )                                             {
      ::curl_easy_setopt ( curl                                              ,
                           CURLOPT_CONNECTTIMEOUT_MS                         ,
                           connectionTimeout                               ) ;
    }                                                                        ;
    if ( mtimeout > 0 )                                                      {
      ::curl_easy_setopt ( curl , CURLOPT_TIMEOUT_MS , mtimeout )            ;
    }                                                                        ;
    QStringList keys = Requests.keys()                                       ;
    QString     K                                                            ;
    struct curl_slist * list = NULL                                          ;
    foreach ( K , keys )                                                     {
      if (K.toLower()=="user-agent")                                         {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_USERAGENT , B.constData()      ) ;
      } else
      if (K.toLower()=="proxy")                                              {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_PROXYTYPE , CURLPROXY_HTTP     ) ;
        ::curl_easy_setopt ( curl , CURLOPT_PROXY     , B.constData()      ) ;
      } else
      if (K.toLower()=="referer")                                            {
        QUrl UR(Requests[K])                                                 ;
        QByteArray RDDR = UR.toEncoded(urlFormat)                            ;
        ::curl_easy_setopt ( curl , CURLOPT_REFERER , RDDR.data()          ) ;
      } else
      if (K.toLower()=="cookie")                                             {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIESESSION , 1              ) ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIEJAR     , B.constData()  ) ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIEFILE    , B.constData()  ) ;
      } else                                                                 {
        QString U = K + ": " + Requests[K]                                   ;
        list = ::curl_slist_append ( list , U.toUtf8().constData() )         ;
      }                                                                      ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    if ( ( keys.count( ) > 0 ) && ( NULL != list ) )                         {
      ::curl_easy_setopt ( curl , CURLOPT_HTTPHEADER , list )                ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    result       = ::curl_easy_perform ( curl                              ) ;
    ResponseCode = result                                                    ;
    ::curl_easy_cleanup                ( curl                              ) ;
    //////////////////////////////////////////////////////////////////////////
    if ( NULL != list ) ::curl_slist_free_all ( list )                       ;
    list       = NULL                                                        ;
    success    = ( CURLE_OK == result                                      ) ;
    finished     ( success                                                 ) ;
  } else failure ( )                                                         ;
  ////////////////////////////////////////////////////////////////////////////
  File = NULL                                                                ;
  Url  = NULL                                                                ;
  ////////////////////////////////////////////////////////////////////////////
  return success                                                             ;
}

bool QtCURL::upload(QIODevice & file,QUrl & url)
{
  Initialize       ( )                                                       ;
  dlHeader . clear ( )                                                       ;
  ////////////////////////////////////////////////////////////////////////////
  bool     success = false                                                   ;
  CURL   * curl    = NULL                                                    ;
  CURLcode result                                                            ;
  ////////////////////////////////////////////////////////////////////////////
  Operation    =  Upload                                                     ;
  File         = &file                                                       ;
  Url          = &url                                                        ;
  ResponseCode = CURLE_OK                                                    ;
  ////////////////////////////////////////////////////////////////////////////
  if ( NULL != Written ) (*Written) = 0                                      ;
  if ( NULL != Sent    ) (*Sent   ) = 0                                      ;
  ////////////////////////////////////////////////////////////////////////////
  curl = ::curl_easy_init (                                                ) ;
  if ( NULL != curl )                                                        {
    QByteArray ADDR = url.toEncoded(urlFormat)                               ;
    starting           (  url , Upload                                     ) ;
    ::curl_easy_setopt ( curl , CURLOPT_URL          , ADDR.data()         ) ;
    ::curl_easy_setopt ( curl , CURLOPT_UPLOAD       , 1L                  ) ;
    ::curl_easy_setopt ( curl , CURLOPT_READFUNCTION , reader              ) ;
    ::curl_easy_setopt ( curl , CURLOPT_READDATA     , accepter            ) ;
    ::curl_easy_setopt ( curl                                                ,
                         CURLOPT_INFILESIZE_LARGE                            ,
                         (curl_off_t) Filesize ( )                         ) ;
    //////////////////////////////////////////////////////////////////////////
    result       = ::curl_easy_perform ( curl                              ) ;
    ResponseCode = result                                                    ;
    ::curl_easy_cleanup                ( curl                              ) ;
    //////////////////////////////////////////////////////////////////////////
    success =    ( CURLE_OK == result                                      ) ;
    finished     ( success                                                 ) ;
  } else failure (                                                         ) ;
  ////////////////////////////////////////////////////////////////////////////
  File = NULL                                                                ;
  Url  = NULL                                                                ;
  ////////////////////////////////////////////////////////////////////////////
  return success                                                             ;
}

QStringList QtCURL::list(QUrl & url)
{
  Initialize       ( )                                                       ;
  dlHeader . clear ( )                                                       ;
  ////////////////////////////////////////////////////////////////////////////
  QByteArray Data                                                            ;
  QBuffer    Buffer ( &Data )                                                ;
  bool       success = false                                                 ;
  CURL     * curl    = NULL                                                  ;
  CURLcode   result                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  Operation    =  List                                                       ;
  File         = &Buffer                                                     ;
  Url          = &url                                                        ;
  ResponseCode = CURLE_OK                                                    ;
  ////////////////////////////////////////////////////////////////////////////
  if ( NULL != Written ) (*Written) = 0                                      ;
  if ( NULL != Sent    ) (*Sent   ) = 0                                      ;
  ////////////////////////////////////////////////////////////////////////////
  curl = ::curl_easy_init (                                                ) ;
  if ( NULL != curl )                                                        {
    QByteArray ADDR = url.toEncoded(urlFormat)                               ;
    starting           (  url , List                                       ) ;
    ::curl_easy_setopt ( curl , CURLOPT_URL           , ADDR . data ( )    ) ;
    ::curl_easy_setopt ( curl , CURLOPT_DIRLISTONLY   , 1                  ) ;
    ::curl_easy_setopt ( curl , CURLOPT_WRITEFUNCTION , lister             ) ;
    ::curl_easy_setopt ( curl , CURLOPT_WRITEDATA     , accepter           ) ;
    ::curl_easy_setopt ( curl , CURLOPT_VERBOSE       , 1L                 ) ;
    //////////////////////////////////////////////////////////////////////////
    result       = ::curl_easy_perform ( curl                              ) ;
    ResponseCode = result                                                    ;
    ::curl_easy_cleanup                ( curl                              ) ;
    //////////////////////////////////////////////////////////////////////////
    success =    ( CURLE_OK == result                                      ) ;
    finished     ( success                                                 ) ;
  } else failure (                                                         ) ;
  ////////////////////////////////////////////////////////////////////////////
  File = NULL                                                                ;
  Url  = NULL                                                                ;
  ////////////////////////////////////////////////////////////////////////////
  return items ( Data )                                                      ;
}

QByteArray QtCURL::list(QUrl & url,bool listonly)
{
  Initialize       ( )                                                       ;
  dlHeader . clear ( )                                                       ;
  ////////////////////////////////////////////////////////////////////////////
  QByteArray Data                                                            ;
  QBuffer    Buffer(&Data)                                                   ;
  bool       success = false                                                 ;
  CURL     * curl    = NULL                                                  ;
  CURLcode   result                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  Operation    =  List                                                       ;
  File         = &Buffer                                                     ;
  Url          = &url                                                        ;
  ResponseCode = CURLE_OK                                                    ;
  ////////////////////////////////////////////////////////////////////////////
  if ( NULL != Written ) (*Written) = 0                                      ;
  if ( NULL != Sent    ) (*Sent   ) = 0                                      ;
  ////////////////////////////////////////////////////////////////////////////
  curl = ::curl_easy_init (                                                ) ;
  if ( NULL != curl )                                                        {
    QByteArray ADDR = url.toEncoded(urlFormat)                               ;
    starting           (  url , List                                       ) ;
    ::curl_easy_setopt ( curl , CURLOPT_URL           , ADDR . data ( )    ) ;
    ::curl_easy_setopt ( curl , CURLOPT_WRITEFUNCTION , lister             ) ;
    ::curl_easy_setopt ( curl , CURLOPT_WRITEDATA     , accepter           ) ;
    ::curl_easy_setopt ( curl , CURLOPT_VERBOSE       , 1L                 ) ;
    if ( listonly )                                                          {
      ::curl_easy_setopt ( curl , CURLOPT_DIRLISTONLY , 1                  ) ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    result       = ::curl_easy_perform ( curl                              ) ;
    ResponseCode = result                                                    ;
    ::curl_easy_cleanup                ( curl                              ) ;
    //////////////////////////////////////////////////////////////////////////
    success =    ( CURLE_OK == result                                      ) ;
    finished     ( success                                                 ) ;
  } else failure (                                                         ) ;
  ////////////////////////////////////////////////////////////////////////////
  File = NULL                                                                ;
  Url  = NULL                                                                ;
  ////////////////////////////////////////////////////////////////////////////
  return Data                                                                ;
}

QByteArray QtCURL::header(QUrl & url,int mtimeout)
{
  Initialize       ( )                                                       ;
  dlHeader . clear ( )                                                       ;
  ////////////////////////////////////////////////////////////////////////////
  QBuffer    Buffer(&dlHeader)                                               ;
  bool       success = false                                                 ;
  CURL     * curl    = NULL                                                  ;
  CURLcode   result                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  Operation    =  Header                                                     ;
  File         = &Buffer                                                     ;
  Url          = &url                                                        ;
  ResponseCode = CURLE_OK                                                    ;
  ////////////////////////////////////////////////////////////////////////////
  if ( NULL != Written ) (*Written) = 0                                      ;
  if ( NULL != Sent    ) (*Sent   ) = 0                                      ;
  ////////////////////////////////////////////////////////////////////////////
  curl = ::curl_easy_init (                                                ) ;
  if ( NULL != curl )                                                        {
    QByteArray ADDR = url.toEncoded(urlFormat)                               ;
    starting             (  url , Header                                   ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_URL            , ADDR . data ( ) ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_NOBODY         , 1               ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_NOPROGRESS     , 1L              ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_HEADERFUNCTION , writer          ) ;
    ::curl_easy_setopt   ( curl , CURLOPT_WRITEHEADER    , this            ) ;
    //////////////////////////////////////////////////////////////////////////
    if ( connectionTimeout > 0 )                                             {
      ::curl_easy_setopt ( curl                                              ,
                           CURLOPT_CONNECTTIMEOUT_MS                         ,
                           connectionTimeout                               ) ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    if ( mtimeout > 0 )                                                      {
      ::curl_easy_setopt ( curl , CURLOPT_TIMEOUT_MS , mtimeout            ) ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    QStringList         keys = Requests . keys ( )                           ;
    QString             K                                                    ;
    struct curl_slist * list = NULL                                          ;
    //////////////////////////////////////////////////////////////////////////
    foreach ( K , keys )                                                     {
      if (K.toLower()=="user-agent")                                         {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_USERAGENT , B.constData()      ) ;
      } else
      if (K.toLower()=="proxy")                                              {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_PROXYTYPE , CURLPROXY_HTTP     ) ;
        ::curl_easy_setopt ( curl , CURLOPT_PROXY     , B.constData()      ) ;
      } else
      if (K.toLower()=="referer")                                            {
        QUrl UR(Requests[K])                                                 ;
        QByteArray B = UR.toEncoded(urlFormat)                               ;
        ::curl_easy_setopt ( curl , CURLOPT_REFERER    , B.constData()     ) ;
      } else
      if (K.toLower()=="cookie")                                             {
        QByteArray B = Requests[K].toUtf8()                                  ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIESESSION , 1              ) ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIEJAR  , B.constData()     ) ;
        ::curl_easy_setopt ( curl , CURLOPT_COOKIEFILE , B.constData()     ) ;
      } else                                                                 {
        QString U = K + ": " + Requests[K]                                   ;
        list = ::curl_slist_append ( list , U . toUtf8() . constData()     ) ;
      }                                                                      ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    if ( ( keys.count()>0 ) && ( NULL != list ) )                            {
      ::curl_easy_setopt ( curl , CURLOPT_HTTPHEADER , list )                ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    result       = ::curl_easy_perform ( curl                              ) ;
    ResponseCode = result                                                    ;
    ::curl_easy_cleanup                ( curl                              ) ;
    //////////////////////////////////////////////////////////////////////////
    if ( NULL != list ) ::curl_slist_free_all ( list )                       ;
    list    = NULL                                                           ;
    success =    ( CURLE_OK == result                                      ) ;
    finished     ( success                                                 ) ;
  } else failure (                                                         ) ;
  ////////////////////////////////////////////////////////////////////////////
  File = NULL                                                                ;
  Url  = NULL                                                                ;
  ////////////////////////////////////////////////////////////////////////////
  return dlHeader                                                            ;
}

QT_END_NAMESPACE
