#【ビルド方法】
# 1.STARBUCKSを使用
#    1.1 libarib25をセットアップする
#    1.2 libstarbucksをセットアップする
#    1.3 recpt1/recpt1ディレクトリで
#        make STARBUCKS=1
#        sudo make install (recpt1を/usr/local/binにコピー)
#
# 2.外付けカードリーダーを使用
#    2.1 recpt1/recpt1ディレクトリで
#        make CARDREADER=1
#        sudo make install
#
# 3.B25解除を行わない場合
#    B25解除を当プログラムで行わず、mirakurun arib-b25-stream-test等で行う場合
#    recpt1/recpt1に移動し
#    make
#    sudo make install
#
# B25処理を変更する為にビルドし直す場合はフルコンパイルする
# 例：外付けカードリーダーからSTARBUCKSに変更する場合
# make CARDREADER=1 clean
# make STARBUCKS=1
#
#
# それぞれのbuildによりリンクするカードリーダーライブラリが異なるので確認する
# ldd recpt1
#
# STARBUCKS  : libarib25.so.0直下にlibstarbucks.so
# CARDREADER : libarib25.so.0直下にlibpcsclite.so.1

ifdef STARBUCKS
HAVE_LIBARIB25 = -DHAVE_LIBARIB25
LIBS    =  -larib25 -lstarbucks
else
ifdef CARDREADER
HAVE_LIBARIB25 = -DHAVE_LIBARIB25
LIBS    =  -larib25 `pkg-config libpcsclite --libs`
endif
endif

DESTDIR = /usr/local/bin

CC = gcc
CXX = g++

TARGET = recpt1
TARGET2 = recpt1ctl
TARGET3 = checksignal
TARGETS = $(TARGET) $(TARGET2) $(TARGET3)

CPPFLAGS = -I../driver -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS   = -O2 -g -pthread $(HAVE_LIBARIB25)

LIBS     += -lpthread -lm
LIBS2    = -lm
LIBS3    = -lpthread -lm
LDFLAGS  =

OBJS  = recpt1.o decoder.o mkpath.o tssplitter_lite.o recpt1core.o
OBJS2 = recpt1ctl.o recpt1core.o
OBJS3 = checksignal.o recpt1core.o
OBJALL = $(OBJS) $(OBJS2) $(OBJS3)
DEPEND = .deps

all: $(TARGETS)

clean:
	rm -f $(OBJALL) $(TARGETS) $(DEPEND)

distclean: clean
	rm -f Makefile config.h config.log config.status

maintainer-clean: distclean
	rm -fr configure config.h.in aclocal.m4 autom4te.cache *~

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
ifdef STARBUCKS
	@echo "\n"B25 DECODE USING STARBUCKS"\n"
else
ifdef CARDREADER
	@echo "\n"B25 DECODE USING CARDREADER"\n"
endif
endif
ifndef HAVE_LIBARIB25
	@echo "\n"B25 DECODE OFF
	@echo "STARBUCKS    -> make STARBUCKS=1"
	@echo "CARDREADER -> make CARDREADER=1""\n"
endif

$(TARGET2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJS2) $(LIBS2)

$(TARGET3): $(OBJS3)
	$(CC) $(LDFLAGS) -o $@ $(OBJS3) $(LIBS3)

$(DEPEND): version.h
	$(CC) -MM $(OBJS:.o=.c) $(OBJS2:.o=.c) $(CPPFLAGS) > $@

install: $(TARGET)
	install -m 755 $(TARGETS) $(DESTDIR)

-include .deps
