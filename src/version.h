#ifndef VERSION_H
#define VERSION_H

#ifndef TRUE
  #define TRUE (1==1)
#endif

#ifndef FALSE
  #define FALSE (!TRUE)
#endif

#define VER_FILEVERSION             0,6,1
#define VER_FILEVERSION_STR         "0.6.1"

#define VER_PRODUCTVERSION          0,6,1
#define VER_PRODUCTVERSION_STR      "0.6.1"

#define VER_COMPANYNAME_STR         "Kazakuri"
#define VER_FILEDESCRIPTION_STR     "Shinjiru"
#define VER_INTERNALNAME_STR        "Shinjiru"
#define VER_LEGALCOPYRIGHT_STR      "Copyright © 2014-2015 Kazakuri"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "Shinjiru.exe"
#define VER_PRODUCTNAME_STR         "Shinjiru"

#define VER_COMPANYDOMAIN_STR       "app.shinjiru.me"

#define VER_UPDATEFEED_STR          "http://app.shinjiru.me/update.xml"
#define VER_AUTHURL_STR             "http://auth.shinjiru.me/index.php"
#define VER_AUTHPINURL_STR          "http://auth.shinjiru.me/pin.php"

#define VER_USEWEBVIEW_BOOL         FALSE

#endif // VERSION_H
