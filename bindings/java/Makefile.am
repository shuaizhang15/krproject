krenginejardir = $(libdir)
krenginejar_DATA = $(krengine_jarfile)
lib_LTLIBRARIES  = libkrengine_java.la
nodist_libkrengine_java_la_SOURCES    = krengine_wrap.c
libkrengine_java_la_SOURCES    = krengine.i
libkrengine_java_la_LIBADD     = $(top_srcdir)/krengine/libkrengine.la
libkrengine_java_la_CPPFLAGS   = $(JNI_CPPFLAGS) 

krengine_wrap.c: krengine.i 
	@mkdir -p org/krproject
	${SWIG} -I${top_srcdir} -java -outdir org/krproject -package org.krproject -o $@ $<

krengine_jarfile = krengine-$(VERSION).jar

krengine-java.stamp: krengine_wrap.c
	@${JAVAC} org/krproject/*.java
	@date > $@

$(krengine_jarfile): krengine-java.stamp
	@$(JAR) cvf $(JARFLAGS) $(krengine_jarfile) org/krproject/*.class org/krproject/*java

clean-local:
	-rm -fr krengine_wrap.c *.jar *.stamp org

