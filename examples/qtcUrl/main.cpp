#include <QtCore>
#include <QtCURL>

int main(int argc,char * argv [])
{
  if ( argc < 3 ) return 0                               ;
  QString agent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1309.0 Safari/537.17" ;
  QString url   = QString ( argv[1] )                    ;
  QString ufn   = QString ( argv[2] )                    ;
  QtCURL  curl                                           ;
  QFile   file ( ufn )                                   ;
  QUrl    u    ( url )                                   ;
  printf ( "Download %s into %s\n" , argv[1] , argv[2] ) ;
  curl . Requests [ "user-agent" ] = agent               ;
  curl . download ( u , file )                           ;
  return 1                                               ;
}
