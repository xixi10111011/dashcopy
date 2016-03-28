if DEBUG
  AM_CFLAGS = -I$(top_srcdir)/src  -Wall -g -o3 ${libcurl_CFLAGS} ${libxml2_CFLAGS}
else
  AM_CFLAGS = -I$(top_srcdir)/src  -Wall ${libcurl_CFLAGS} ${libxml2_CFLAGS}
endif

bin_PROGRAMS = dashcopy
dashcopy_SOURCES = main.c
dashcopy_LDADD = ${libcurl_LIBS} ${libxml2_LIBS}
