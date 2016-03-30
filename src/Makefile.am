if DEBUG
  AM_CFLAGS = -I$(top_srcdir)/src  -Wall -g -O3 
else
  AM_CFLAGS = -I$(top_srcdir)/src  -Wall 
endif

AM_CFLAGS += $(LIBCURL_CFLAGS)
AM_CFLAGS += $(LIBXML2_CFLAGS)

bin_PROGRAMS = dashcopy
dashcopy_SOURCES = dashcopy.c http.c 
dashcopy_LDADD = $(AM_LDFLAGS)
dashcopy_LDADD += $(LIBCURL_LIBS)
dashcopy_LDADD += $(LIBXML2_LIBS)
