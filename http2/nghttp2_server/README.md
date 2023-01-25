* We have nghttp2 as a submodule, for this to work that repo needs to be built:
PWD=`pwd`
LIBNGHTTP2_INSTALL_DIR=$PWD/install
mkdir -p $LIBNGHTTP2_INSTALL_DIR
autoreconf -i
automake
autoconf
./configure --prefix=$LIBNGHTTP2_INSTALL_DIR --enable-lib-only --disable-assert
make
make install

* Written refering to this: https://github.com/nghttp2/nghttp2/blob/master/examples/libevent-server.c

* Test conformance to spec with https://github.com/summerwind/h2spec