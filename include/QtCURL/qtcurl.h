/****************************************************************************
 *
 * Copyright (C) 2015 Neutrino International Inc.
 *
 * Author : Brian Lin <lin.foxman@gmail.com>, Skype: wolfram_lin
 *
 * QtCURL acts as an interface between Qt and cURL library.
 * Please keep QtCURL as simple as possible.
 *
 ****************************************************************************/

#ifndef QT_CURL_H
#define QT_CURL_H

#include <QtCore>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#    if defined(QT_BUILD_QTCURL_LIB)
#      define Q_CURL_EXPORT Q_DECL_EXPORT
#    else
#      define Q_CURL_EXPORT Q_DECL_IMPORT
#    endif
#else
#      define Q_CURL_EXPORT
#endif

typedef size_t (*curlio) (void * buffer,size_t size,size_t nmemb,void * userp) ;

class Q_CURL_EXPORT QtCURL
{
  public:

    typedef enum   {
      None     = 0 ,
      Verify   = 1 ,
      List     = 2 ,
      Header   = 3 ,
      Download = 4 ,
      Upload   = 5 }
      Operations   ;

    QIODevice             * File              ;
    QUrl                  * Url               ;
    qint64                * Written           ;
    qint64                * Sent              ;
    Operations              Operation         ;
    QMap<QString,QString>   Requests          ;
    QByteArray              dlHeader          ;
    int                     connectionTimeout ;
    int                     ResponseCode      ;
    QUrl::FormattingOptions urlFormat         ;
    curlio                  writer            ;
    curlio                  reader            ;
    curlio                  lister            ;
    curlio                  tagger            ;
    void                  * accepter          ;

    explicit       QtCURL     (void);
    virtual       ~QtCURL     (void);

    static QString Version    (void) ;
    static void    Initialize (void) ;
    static void    Cleanup    (void) ;
    static QString Explain    (int curlcode) ;

    QStringList    items      (QByteArray & data) ;

    virtual void   starting   (QUrl & Url,Operations Operation) ;
    virtual void   finished   (bool success) ;
    virtual void   failure    (void) ;

    static  size_t cwrite     (void * buffer,size_t size,size_t nmemb,void * userp) ;
    static  size_t cread      (void * buffer,size_t size,size_t nmemb,void * userp) ;
    static  size_t clist      (void * buffer,size_t size,size_t nmemb,void * userp) ;
    static  size_t hwrite     (void * buffer,size_t size,size_t nmemb,void * userp) ;

    virtual size_t write      (void * buffer,size_t size,size_t nmemb) ;
    virtual size_t writeh     (void * buffer,size_t size,size_t nmemb) ;
    virtual size_t list       (void * buffer,size_t size,size_t nmemb) ;
    virtual size_t read       (void * buffer,size_t size,size_t nmemb) ;

    virtual size_t Filesize   (void) ;

    bool           post       (QUrl & url,QIODevice & file,QString values,int mtimeout = 0) ;
    bool           download   (QUrl & url,QIODevice & file,int mtimeout = 0) ;
    bool           upload     (QIODevice & file,QUrl & url) ;

    QStringList    list       (QUrl & url) ;
    QByteArray     list       (QUrl & url,bool listonly) ;

    QByteArray     header     (QUrl & url,int mtimeout = 0) ;

  protected:

  private:

} ;

Q_CURL_EXPORT void RegisterCURL (void) ;

Q_DECLARE_METATYPE(QtCURL)

QT_END_NAMESPACE

#endif
